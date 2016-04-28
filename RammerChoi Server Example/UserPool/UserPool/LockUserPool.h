#pragma once
#include"UserPool.h"
#include<unordered_map>
#include<queue>

//queue와 hashMap을 Lock으로 동기화한 클래스
//hashMap은 버킷에만 Lock을 걸 수 없어서 성긴 동기화.
class LockUserPool
{
	Player players[MAX_USER];
	LockUserPool();
	typedef unordered_map<UserID, int> IndexHashMap;

	IndexHashMap indexMap;
	CRITICAL_SECTION mapCS;

	queue<int> freeQueue;
	CRITICAL_SECTION queueCS;

	//찾으면 배열의 index 리턴, 못찾으면 -1 리턴
	int getUsableIndex();

	//유저 정보 초기화
	bool initPlayer(int index, UserID id, SOCKET sock);
public:
	
	~LockUserPool();

	static LockUserPool* getInstance()
	{
		static LockUserPool *instance = nullptr;
		if (instance == nullptr){
			instance = new LockUserPool;
		}

		return instance;
	}

	Player* getPlayer(UserID id, int* output_index = nullptr);	//내부 배열의 포인터이므로, 언제나 해제되지 않는다.
	void kickPlayer(UserID id);
	bool acceptPlayer(UserID id, SOCKET sock);

	//싱글스레드에서만 사용
	size_t getNumOfPlayer();
	size_t getNumOfPlayer2();
};

