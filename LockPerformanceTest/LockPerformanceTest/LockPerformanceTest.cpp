// LockPerformanceTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <ppl.h>
#include <mutex>
#include <windows.h>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace concurrency;

constexpr int THREAD_NUM = 8;
constexpr int TEST_SIZE = 1000000;

int target = 0;

// exclusive lock 으로만 성능비교

mutex m;
void StdMutexTest()
{
	m.lock();
	++target;
	m.unlock();
}

reader_writer_lock rw;
void PplRWLockTest()
{
	rw.lock();
	++target;
	rw.unlock();
}

CRITICAL_SECTION cs;
void SpinCSTest()
{
	EnterCriticalSection(&cs);
	++target;
	LeaveCriticalSection(&cs);
}

SRWLOCK srw;
void SRWTest()
{
	AcquireSRWLockExclusive(&srw);
	++target;
	ReleaseSRWLockExclusive(&srw);
}

void ThreadEntry(void (*f)())
{
	for (int i = 0; i < TEST_SIZE / THREAD_NUM; ++i)
	{
		f();
	}
}

template<class Func>
void Test(Func f, std::string text)
{
	vector<thread> v;
	v.reserve(THREAD_NUM);

	auto beg = system_clock::now();
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		v.emplace_back(ThreadEntry, f);
	}

	for (auto& e : v)
	{
		e.join();
	}
	auto end = system_clock::now();
	cout << text << duration_cast<milliseconds>(end - beg).count() << endl;
}

int main()
{
	InitializeCriticalSectionAndSpinCount(&cs, 1024);
	InitializeSRWLock(&srw);

	Test(StdMutexTest, "std::mutex : ");
	Test(PplRWLockTest, "concurrency::reader_writer_lock : ");
	Test(SpinCSTest, "spin CRITICAL_SECTION : ");
	Test(SRWTest, "SRWLock : ");

	DeleteCriticalSection(&cs);
}