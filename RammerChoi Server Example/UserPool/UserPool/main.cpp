#include<iostream>
#include"UserPool.h"
#include<chrono>
#include<map>
#include<thread>
#include"LockUserPool.h"
#include<conio.h>
#include<vector>

#define TEST_SIZE MAX_USER

using namespace std;
using namespace std::chrono;

template<class Pool>
void producerThreadFunc(int threadID, int threadNum)
{
	auto pool = Pool::getInstance();
	int beg = TEST_SIZE * threadID / threadNum;
	int end = beg + TEST_SIZE / threadNum;
	for (int i = beg; i < end; ++i)
	{
		pool->acceptPlayer(i, rand());
	}
}

template<class Pool>
void consumerThreadFunc(int threadID, int threadNum)
{
	auto pool = Pool::getInstance();
	int beg = TEST_SIZE * threadID / threadNum;
	int end = beg + TEST_SIZE / threadNum;
	for (int i = beg; i < end; ++i)
	{
		pool->kickPlayer(i);
	}
}


template<class Pool>
void UserPoolTest(int coreNum)
{
	vector<thread*> producer;
	vector<thread*> consumers;

	cout << endl << "--" << typeid(Pool).name() <<" with TBB Test Start--" << endl;

	auto start = high_resolution_clock::now();

	//삽입 먼저
	for (int i = 0; i < coreNum; ++i){
		producer.push_back(new thread{ producerThreadFunc<Pool>, i, coreNum });
	}
	for (auto& th : producer)
		th->join();

	cout << TEST_SIZE << " insert result : " ;
	if (TEST_SIZE == Pool::getInstance()->getNumOfPlayer() &&
		TEST_SIZE == Pool::getInstance()->getNumOfPlayer2())
		cout << "All Insert Success" << endl;
	cout << "---" << endl;

	//삭제 나중
	for (int i = 0; i < coreNum; ++i){
		consumers.push_back(new thread{ consumerThreadFunc<Pool>, i, coreNum });
	}
	for (auto& th : consumers)
		th->join();

	cout << TEST_SIZE << " erase result : " ;
	if (0 == Pool::getInstance()->getNumOfPlayer() &&
		0 == Pool::getInstance()->getNumOfPlayer2())
		cout << "All erase Success" << endl;
	cout << "---" << endl;

	auto durCount = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
	cout << "duration count : " << durCount << endl;

	cout << "------------------------------------" << endl;
}

int main()
{
	//최초 초기화는 싱글스레드에서.
	auto pool = UserPool::getInstance();
	auto lockPool = LockUserPool::getInstance();


	//---시스템 정보 확인---
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	//프로세서 개수 확인
	int coreNum = static_cast<int>(si.dwNumberOfProcessors);
	cout << "Number of Core : " << coreNum << endl << endl;

	//유저 천만 명 접속 -> 천만 명 모두 종료 과정에서의 시간 측정
	cout << "유저 천만 명 접속 -> 천만 명 모두 종료 과정에서의 시간 측정" << endl;

	//---성능 테스트 시작---
	UserPoolTest<UserPool>(coreNum);
	UserPoolTest<LockUserPool>(coreNum);	

	cout << endl<< endl<< "Test End";
	_getch();
}