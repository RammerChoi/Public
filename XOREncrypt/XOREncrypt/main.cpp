#include <iostream>
#include <vector>
#include <string>
#include <random>

using namespace std;

vector<std::uint8_t> GetFilter(const int _len)
{
	vector<uint8_t> ret;
	ret.resize(_len);

	std::random_device rd;
	std::mt19937 mt(rd());

	uint32_t* ptr = nullptr;
	for (int i = 0; i < _len / 4; ++i)
	{
		ptr = (uint32_t*)(ret.data()) + i;
		*ptr = mt();
	}

	uint32_t lastRandom = mt();
	uint8_t* lastRandomPtr = (uint8_t*)&lastRandom;
	uint8_t* ptrLastDword = (uint8_t*)(ptr + 1);
	for (int i = 0; i < _len % 4; ++i)
	{
		*(ptrLastDword + i) = *(lastRandomPtr + i);
	}

	return ret;
}

template<class T>
void PrintByte(T&& _cont)
{
	for (auto& e : _cont)
	{
		printf("%4d", (int)e);
	}
	cout << endl;
}

template<class T>
void Print(T&& _cont)
{
	for (auto& e : _cont)
	{
		cout << e;
	}
	cout << endl;
}

template<class T1, class T2>
vector<uint8_t> GetXOR(const T1 _target, const T2 _filter)
{
	vector<uint8_t> ret;
	ret.resize(_target.size());

	for (int i = 0; i < _target.size(); ++i)
	{
		ret[i] = _target[i] ^ _filter[i];
	}
	
	return ret;
}

int main()
{
	string target = "Hello World";
	vector<uint8_t> filter = GetFilter(target.size());

	PrintByte(target);
	PrintByte(filter);

	auto encrypted = GetXOR(target, filter);
	PrintByte(encrypted);

	auto decrypted = GetXOR(encrypted, filter);
	Print(decrypted);

}