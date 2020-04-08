#include <iostream>
#include <windows.h>

using namespace std;

#define BUFSIZE 4096 


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	for (int i = 0; i < 3; ++i)
	{
		cout << "hello " << i << endl;
		Sleep(1000);
	}
	
	return 0;

}