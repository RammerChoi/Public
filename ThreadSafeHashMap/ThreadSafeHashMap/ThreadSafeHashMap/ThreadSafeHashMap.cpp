
#include "stdafx.h"
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <ppl.h>
#include <concurrent_queue.h>
#include <chrono>
#include <Windows.h>

using namespace std;
using namespace std::chrono;

struct MyMutex
{
	CRITICAL_SECTION cs;

	MyMutex()
	{
		InitializeCriticalSection(&cs);
	}

	~MyMutex()
	{
		DeleteCriticalSection(&cs);
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

	MyMutex(const MyMutex&) = delete;
	MyMutex& operator =(const MyMutex&) = delete;

};

struct Test
{
	int id;
	int data;
	MyMutex dataLock;
};


int Rand()
{
	//return rand() * rand();
	return rand() % 10;	// 충돌확률 높임
}

//concurrency::concurrent_queue<int> queue;

unordered_map<int, Test*> m;
MyMutex containerLock;

bool Insert(const int id, int data)
{
	Test* pTest = new Test{ id, data };

	lock_guard<MyMutex> lock(containerLock);
	if (!m.emplace(id, pTest).second)
	{
		delete pTest;
		return false;
	}

	cout << "삽입" << endl;
	return true;
}

Test* Lock(int id)
{
	lock_guard<MyMutex> lock(containerLock);
	auto it = m.find(id);
	if (it == m.end())
		return nullptr;

	it->second->dataLock.lock();
	cout << "사용" << endl;
	return it->second;
}

void Unlock(Test* pTest)
{
	if (pTest == nullptr)
		return;

	pTest->dataLock.unlock();
	cout << "해제" << endl;
}

void Erase(int id);
void Erase(Test* pTest)
{
	if (pTest == nullptr)
		return;

	int id = pTest->id;
	Unlock(pTest);
	Erase(id);
}

void Erase(int id)
{
	lock_guard<MyMutex> lock(containerLock);
	auto it = m.find(id);
	if (it == m.end())
		return;

	// lock을 얻으면, "TestWrapper가 다른곳에서 사용중이지 않음"이 보장됨.
	it->second->dataLock.lock();		// 위에서 잡은 containerLock을 놓기 전까지는 
	it->second->dataLock.unlock();		// 다른 스레드가 이 TestWrapper를 사용하지 못한다.
	delete it->second;					// 그러므로 지워도 됨.

	m.erase(id);
	cout << "삭제" << endl;
}

void consumerThread()
{
	while (1)
	{
		int id = 0;
		//if (!queue.try_pop(id))
		//	continue;

		id = Rand();


		if (Test* pTest = Lock(id))
		{
			pTest->data = 20;
			Unlock(pTest);
		}

		// Lock을 얻었을 때에는 다음과 같이 포인터로 해제
		//if (Test* pTest = Lock(id))
		//{
		//	Erase(pTest);
		//}

		// Lock 이 없을 때에는 ID로 해제
		Erase(id);
	}
}

void providerThread()
{
	while (1)
	{
		int id = Rand();

		if (Insert(id, 10))
		{
			//queue.push(id);
			this_thread::sleep_for(10ms);
		}
	}
}

int main()
{
	srand((unsigned)time(NULL));
	vector<thread> threads;
	threads.reserve(4);
	threads.emplace_back(providerThread);

	for (int i = 0; i < 3; ++i)
	{
		threads.emplace_back(consumerThread);
	}

	for (auto& e : threads)
	{
		e.join();
	}

	return 0;
}

