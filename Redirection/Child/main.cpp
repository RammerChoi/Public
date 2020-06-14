#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

#define BUFSIZE 4096 


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	std::string str;
	str.resize(10000);
	
	while (1)
	{
		str.clear();
		std::getline(std::cin, str);
		MessageBoxA(0, str.c_str(), str.c_str(), MB_OK);
		cout << str << endl;
	}

	return 0;
}