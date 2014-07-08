#pragma once
#include<WinSock2.h>
#include<iostream>
#include<concurrent_queue.h>
#include<concurrent_hash_map.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_USER 10000000

using namespace std;

typedef unsigned int UserID;

struct Player
{
	int hp;
	int dp;
	int sp;
	UserID id;
	POINT pos;
	SOCKET sock;
	bool used;
};

class UserPool
{
	Player players[MAX_USER];
	UserPool();

	typedef tbb::concurrent_hash_map<UserID, int> IndexHashMap;

	IndexHashMap indexMap;
	tbb::concurrent_queue<int> freeQueue;

	//ã���� �迭�� index ����, ��ã���� -1 ����
	int getUsableIndex();

	//���� ���� �ʱ�ȭ
	bool initPlayer(int index, UserID id, SOCKET sock);

public:
	//�̱� �����忡�� �����Ǿ�� �Ѵ�.
	static UserPool* getInstance()
	{
		static UserPool *instance = nullptr;
		if (instance == nullptr){
			instance = new UserPool;
		}

		return instance;
	}
	~UserPool();

	Player* getPlayer(UserID id, int* output_index = nullptr);	//���� �迭�� �������̹Ƿ�, ������ �������� �ʴ´�.
	void kickPlayer(UserID id);
	bool acceptPlayer(UserID id, SOCKET sock);
	size_t getNumOfPlayer();
	size_t getNumOfPlayer2();

	
};

