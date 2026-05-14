#pragma once
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
class CCreateDump
{
public:
	CCreateDump();
	~CCreateDump(void);
	static CCreateDump* Instance();
	static long WINAPI UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo);

	void DeclarDumpFile(std::string dmpFileName = "");
private:
	static std::string    strDumpFile;
	static CCreateDump* __instance;
};
