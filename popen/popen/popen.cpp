#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <windows.h>

//https://stackoverflow.com/questions/44610978/popen-writes-output-of-command-executed-to-cout
int main()
{
	wchar_t dir[128];
	GetCurrentDirectory(128, dir);
	std::wcout << dir << std::endl;

	// modify current directory with SetCurrentDirectory()

	std::string command("dummy.exe");

	std::array<char, 128> buffer;
	std::string result;

	std::cout << "Opening reading pipe" << std::endl;
	FILE* pipe = _popen(command.c_str(), "r");
	if (!pipe)
	{
		std::cerr << "Couldn't start command." << std::endl;
		return 0;
	}
	while (fgets(buffer.data(), 128, pipe) != NULL) {
		std::cout << "Reading... : " << buffer.data() << std::endl;
		result += buffer.data();
	}
	auto returnCode = _pclose(pipe);

	std::cout << result << std::endl;
	std::cout << "returnCode : " << returnCode << std::endl;

	return 0;
}