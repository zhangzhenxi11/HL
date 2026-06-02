#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <Windows.h>

using namespace std;
class CCreateDump
{
public:
	CCreateDump();
	~CCreateDump(void);
	static CCreateDump* Instance();
	static long WINAPI UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo);
	static void HandleTerminate();
	static void HandlePureCall();
	static void HandleInvalidParameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved);
	static void HandleSignal(int signalNumber);

	void DeclarDumpFile(std::string dmpFileName = "");
	void InstallHandlers(std::string dmpFileName = "");
private:
	static bool WriteDump(_EXCEPTION_POINTERS* ExceptionInfo, const wchar_t* reason);
	static std::wstring BuildDumpFilePath(const wchar_t* reason);
	static std::wstring GetDumpDirectory();
	static std::string    strDumpFile;
	static CCreateDump* __instance;
};
