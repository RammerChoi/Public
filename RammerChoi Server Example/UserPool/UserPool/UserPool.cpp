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
	//�̹� �Ҵ�� ������ return false;
	if (players[index].used == true) return false;

	//���� ����(�������� ����) �ʱ�ȭ
	players[index].hp = 10;
	players[index].dp = 10;
	players[index].pos = { 10, 20 };
	players[index].sock = sock;
	players[index].id = id;

	//�� ��������
	players[index].used = true;
	
	return true;
}

int UserPool::getUsableIndex()
{
	int index = -2;	// freeQueue ��Ȱ�밪 ������ �����ϱ� ����
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
{//���� �ȵ� id��� �ƹ� �ϵ� ���� �ʴ´�.
	int index = -1;

	Player* player = getPlayer(id, &index);
	if (nullptr == player) return;
	
	player->used = false;
	closesocket(player->sock);

	//�ٽ� freeQueue�� index ���, �ε����� ��Ȱ���Ѵ�.
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