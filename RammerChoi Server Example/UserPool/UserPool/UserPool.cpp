#include "UserPool.h"

UserPool::UserPool()
{
	for (int i = 0; i < MAX_USER; ++i)
		freeQueue.push(i);
	if (freeQueue.unsafe_size() == MAX_USER)
		cout << "UserPool init Success" << endl;
}

UserPool::~UserPool()
{
}

Player* UserPool::getPlayer(UserID id, int* output_index)
{
	int index = -1;
	IndexHashMap::const_accessor a;
	bool success = indexMap.find(a, id);
	if (success)
		index = a->second;

	a.release();

	if (index < 0)
		return nullptr;

	if (output_index)
		*output_index = index;

	return &players[index];
}

bool UserPool::initPlayer(int index, UserID id, SOCKET sock)
{
	//이미 할당된 유저면 return false;
	if (players[index].used == true) return false;

	//유저 정보(게임정보 제외) 초기화
	players[index].hp = 10;
	players[index].dp = 10;
	players[index].pos = { 10, 20 };
	players[index].sock = sock;
	players[index].id = id;

	//맨 마지막에
	players[index].used = true;
	
	return true;
}

int UserPool::getUsableIndex()
{
	int index = -2;	// freeQueue 재활용값 에러를 구분하기 위함
	bool success = freeQueue.try_pop(index);

	if (success)
		return index;
	else
		return -1;
}

bool UserPool::acceptPlayer(UserID id, SOCKET sock)
{
	int index = getUsableIndex();
	if (index < 0) return false;

	bool success = indexMap.insert(std::make_pair(id, index));

	if (success)
 		return initPlayer(index, id, sock);
	else
		return false;
}

void UserPool::kickPlayer(UserID id)
{//접속 안된 id라면 아무 일도 하지 않는다.
	int index = -1;

	Player* player = getPlayer(id, &index);
	if (nullptr == player) return;
	
	player->used = false;
	closesocket(player->sock);

	//다시 freeQueue에 index 등록, 인덱스는 재활용한다.
	freeQueue.push(index);
	indexMap.erase(id);
}

size_t UserPool::getNumOfPlayer()
{
	return indexMap.size();
}

size_t UserPool::getNumOfPlayer2()
{
	return MAX_USER - freeQueue.unsafe_size();
}