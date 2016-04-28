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

	//찾으면 배열의 index 리턴, 못찾으면 -1 리턴
	int getUsableIndex();

	//유저 정보 초기화
	bool initPlayer(int index, UserID id, SOCKET sock);

public:
	//싱글 스레드에서 생성되어야 한다.
	static UserPool* getInstance()
	{
		static UserPool *instance = nullptr;
		if (instance == nullptr){
			instance = new UserPool;
		}

		return instance;
	}
	~UserPool();

	Player* getPlayer(UserID id, int* output_index = nullptr);	//내부 배열의 포인터이므로, 언제나 해제되지 않는다.
	void kickPlayer(UserID id);
	bool acceptPlayer(UserID id, SOCKET sock);
	size_t getNumOfPlayer();
	size_t getNumOfPlayer2();

	
};

