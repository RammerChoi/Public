
#include "stdafx.h"
#include <thread>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <chrono>
#include <memory>
#include <Windows.h>

using namespace std;
using namespace std::chrono;

class RWLock
{
public:
	RWLock()
	{
		InitializeSRWLock(&lock);
	}

	~RWLock()
	{
		// do nothing
	}

	void lockExclusive()
	{
		AcquireSRWLockExclusive(&lock);
	}
	void unlockExclusive()
	{
		ReleaseSRWLockExclusive(&lock);
	}

	void lockShared()
	{
		AcquireSRWLockShared(&lock);
	}

	void unlockShared()
	{
		ReleaseSRWLockShared(&lock);
	}

	// scoped lock
public:
	class ExclusiveGuard
	{
	public:
		explicit ExclusiveGuard(RWLock& _lock)
			: lock_(_lock)
		{
			lock_.lockExclusive();
		}
		~ExclusiveGuard()
		{
			lock_.unlockExclusive();
		}
	
	private:
		RWLock& lock_;
	};

	class SharedGuard
	{
	public:
		explicit SharedGuard(RWLock& _lock)
			: lock_(_lock)
		{
			lock_.lockShared();
		}

		~SharedGuard()
		{
			lock_.unlockShared();
		}

	private:
		RWLock& lock_;
	};

private:
	RWLock(const RWLock&) = delete;
	RWLock& operator=(const RWLock&) = delete;

	SRWLOCK lock;
};

struct Test
{
	int id;
	int data;
};


int Rand()
{
	//return rand() * rand();
	return rand() % 100;	// 충돌확률 높임
}

concurrency::concurrent_unordered_map<int, shared_ptr<Test>> m;   // insert, find - thread safe
RWLock containerLock;											  // erase - unsafe

bool Insert(const int id, int data)
{
	auto pTest = make_shared<Test>();
	pTest->id = id;
	pTest->data = data;

	RWLock::SharedGuard scoped(containerLock);
	if (!m.insert({ id,  pTest }).second)
	{
		return false;
	}

	cout << "삽입" << endl;
	return true;
}

shared_ptr<Test> Lock(int id)
{
	RWLock::SharedGuard scoped(containerLock);
	auto it = m.find(id);
	if (it == m.end())
		return nullptr;

	cout << "사용" << endl;
	return it->second;
}

void Unlock(const shared_ptr<Test>& pTest)
{
	if (pTest == nullptr)
		return;

	cout << "해제" << endl;
}

void Erase(int id);
void Erase(shared_ptr<Test> pTest)
{
	if (pTest == nullptr)
		return;

	int id = pTest->id;
	Unlock(pTest);
	Erase(id);
}

void Erase(int id)
{
	RWLock::ExclusiveGuard scoped(containerLock);
	auto it = m.find(id);
	if (it == m.end())
		return;

	m.unsafe_erase(id);
	cout << "삭제" << endl;
}

void consumerThread()
{
	while (1)
	{
		int id = Rand();

		if (shared_ptr<Test> pTest = Lock(id))
		{
			pTest->data = 20;
			Unlock(pTest);
		}

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
			// 생성 속도 조절
			//this_thread::sleep_for(10ms);
		}
	}
}

void homogeneousThread()
{
	while (1)
	{
		int id = Rand();
		int action = rand() % 3;
		switch (action)
		{
		case 0:
			Insert(id, 10);
			break;
		case 1:
			if (shared_ptr<Test> pTest = Lock(id))
			{
				// expensive work test
				//for (int i = 0; i < 1024; ++i)
				//{
				//	++pTest->data;
				//}

				++pTest->data;
				Unlock(pTest);
			}
			break;
		case 2:
			Erase(id);
			break;
		}
	}
}

int main()
{
	srand((unsigned)time(NULL));
	vector<thread> threads;
	threads.reserve(8);

	// provider - consumer test
	//for (int i = 0; i < 4; ++i)
	//{
	//	threads.emplace_back(providerThread);
	//}

	//for (int i = 0; i < 4; ++i)
	//{
	//	threads.emplace_back(consumerThread);
	//}

	// homogeneous test
	for (int i = 0; i < 8; ++i)
	{
		threads.emplace_back(homogeneousThread);
	}

	for (auto& e : threads)
	{
		e.join();
	}

	return 0;
}

