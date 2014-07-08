#pragma once
#include"UserPool.h"
#include<unordered_map>
#include<queue>

//queue�� hashMap�� Lock���� ����ȭ�� Ŭ����
//hashMap�� ��Ŷ���� Lock�� �� �� ��� ���� ����ȭ.
class LockUserPool
{
	Player players[MAX_USER];
	LockUserPool();
	typedef unordered_map<UserID, int> IndexHashMap;

	IndexHashMap indexMap;
	CRITICAL_SECTION mapCS;

	queue<int> freeQueue;
	CRITICAL_SECTION queueCS;

	//ã���� �迭�� index ����, ��ã���� -1 ����
	int getUsableIndex();

	//���� ���� �ʱ�ȭ
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

	Player* getPlayer(UserID id, int* output_index = nullptr);	//���� �迭�� �������̹Ƿ�, ������ �������� �ʴ´�.
	void kickPlayer(UserID id);
	bool acceptPlayer(UserID id, SOCKET sock);

	//�̱۽����忡���� ���
	size_t getNumOfPlayer();
	size_t getNumOfPlayer2();
};

