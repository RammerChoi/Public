#include "LockUserPool.h"


LockUserPool::LockUserPool()
{
	InitializeCriticalSection(&mapCS);
	InitializeCriticalSection(&queueCS);

	for (int i = 0; i < MAX_USER; ++i)
		freeQueue.push(i);

	if (freeQueue.size() == MAX_USER)
		cout << "LockUserPool init Success" << endl;
}


LockUserPool::~LockUserPool()
{
	DeleteCriticalSection(&mapCS);
	DeleteCriticalSection(&queueCS);
}


Player* LockUserPool::getPlayer(UserID id, int* output_index)
{
	int index = -1;

	EnterCriticalSection(&mapCS);
	auto iter = indexMap.find(id);
	if (iter != indexMap.end())
		index = iter->second;
	LeaveCriticalSection(&mapCS);


	if (index < 0)
		return nullptr;

	if (output_index)
		*output_index = index;

	return &players[index];
}

bool LockUserPool::initPlayer(int index, UserID id, SOCKET sock)
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

int LockUserPool::getUsableIndex()
{
	int index = -2;	// freeQueue 재활용값 에러를 구분하기 위함

	EnterCriticalSection(&queueCS);

	if (freeQueue.empty()) return -1;

	index = freeQueue.front();
	freeQueue.pop();

	LeaveCriticalSection(&queueCS);

	return index;
}

bool LockUserPool::acceptPlayer(UserID id, SOCKET sock)
{
	int index = getUsableIndex();
	if (index < 0) return false;

	EnterCriticalSection(&mapCS);
	auto result = indexMap.insert(std::make_pair(id, index));
	LeaveCriticalSection(&mapCS);

	if (true == result.second)
		return initPlayer(index, id, sock);
	else
		return false;
}

void LockUserPool::kickPlayer(UserID id)
{//접속 안된 id라면 아무 일도 하지 않는다.
	int index = -1;

	Player* player = getPlayer(id, &index);
	if (nullptr == player) return;

	player->used = false;
	closesocket(player->sock);

	//다시 freeQueue에 index 등록, 인덱스는 재활용한다.
	EnterCriticalSection(&queueCS);
	freeQueue.push(index);
	LeaveCriticalSection(&queueCS);

	EnterCriticalSection(&mapCS);
	indexMap.erase(id);
	LeaveCriticalSection(&mapCS);
}

size_t LockUserPool::getNumOfPlayer()
{
	return indexMap.size();
}

size_t LockUserPool::getNumOfPlayer2()
{
	return MAX_USER - freeQueue.size();
}
