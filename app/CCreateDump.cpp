#include <Windows.h>
#include "CCreateDump.h"
#include <DbgHelp.h>
#include <codecvt>
#include <eh.h>
#include <signal.h>
#include <sstream>

#pragma comment(lib,  "dbghelp.lib")

CCreateDump* CCreateDump::__instance = NULL;
std::string CCreateDump::strDumpFile = "";

CCreateDump::CCreateDump()
{
}

CCreateDump::~CCreateDump(void)
{

}

long WINAPI CCreateDump::UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
	return WriteDump(ExceptionInfo, L"seh") ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

void CCreateDump::DeclarDumpFile(std::string dmpFileName)
{
	strDumpFile = dmpFileName.empty() ? "app_dump" : dmpFileName;
}

CCreateDump* CCreateDump::Instance()
{
	if (__instance == NULL)
	{
		__instance = new CCreateDump;
	}
	return __instance;
}

void CCreateDump::InstallHandlers(std::string dmpFileName)
{
	DeclarDumpFile(dmpFileName);
	SetUnhandledExceptionFilter(UnhandleExceptionFilter);
	std::set_terminate(&CCreateDump::HandleTerminate);
	_set_purecall_handler(&CCreateDump::HandlePureCall);
	_set_invalid_parameter_handler(&CCreateDump::HandleInvalidParameter);
	signal(SIGABRT, &CCreateDump::HandleSignal);
	signal(SIGSEGV, &CCreateDump::HandleSignal);
	signal(SIGFPE, &CCreateDump::HandleSignal);
	signal(SIGILL, &CCreateDump::HandleSignal);
}

void CCreateDump::HandleTerminate()
{
	WriteDump(nullptr, L"terminate");
	TerminateProcess(GetCurrentProcess(), 1);
}

void CCreateDump::HandlePureCall()
{
	WriteDump(nullptr, L"purecall");
	TerminateProcess(GetCurrentProcess(), 1);
}

void CCreateDump::HandleInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
{
	WriteDump(nullptr, L"invalid_parameter");
	TerminateProcess(GetCurrentProcess(), 1);
}

void CCreateDump::HandleSignal(int signalNumber)
{
	const wchar_t* reason = L"signal";
	switch (signalNumber)
	{
	case SIGABRT:
		reason = L"sigabrt";
		break;
	case SIGSEGV:
		reason = L"sigsegv";
		break;
	case SIGFPE:
		reason = L"sigfpe";
		break;
	case SIGILL:
		reason = L"sigill";
		break;
	default:
		break;
	}

	WriteDump(nullptr, reason);
	TerminateProcess(GetCurrentProcess(), static_cast<UINT>(signalNumber));
}

bool CCreateDump::WriteDump(_EXCEPTION_POINTERS* ExceptionInfo, const wchar_t* reason)
{
	const std::wstring dumpFile = BuildDumpFilePath(reason);
	HANDLE hFile = CreateFileW(dumpFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::wstringstream ss;
		ss << L"Create dump file failed: " << dumpFile << L", error=" << GetLastError() << L"\n";
		OutputDebugStringW(ss.str().c_str());
		return false;
	}

	MINIDUMP_EXCEPTION_INFORMATION exInfo;
	MINIDUMP_EXCEPTION_INFORMATION* exInfoPtr = nullptr;
	if (ExceptionInfo != nullptr)
	{
		exInfo.ThreadId = ::GetCurrentThreadId();
		exInfo.ExceptionPointers = ExceptionInfo;
		exInfo.ClientPointers = FALSE;
		exInfoPtr = &exInfo;
	}

	const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
		MiniDumpWithDataSegs |
		MiniDumpWithHandleData |
		MiniDumpWithThreadInfo |
		MiniDumpWithIndirectlyReferencedMemory |
		MiniDumpScanMemory);
	BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, exInfoPtr, NULL, NULL);
	CloseHandle(hFile);
	if (!ok)
	{
		std::wstringstream ss;
		ss << L"MiniDumpWriteDump failed: " << dumpFile << L", error=" << GetLastError() << L"\n";
		OutputDebugStringW(ss.str().c_str());
		return false;
	}

	std::wstringstream ss;
	ss << L"Dump file created: " << dumpFile << L"\n";
	OutputDebugStringW(ss.str().c_str());
	return true;
}

std::wstring CCreateDump::GetDumpDirectory()
{
	wchar_t modulePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
	std::wstring dir(modulePath);
	const size_t pos = dir.find_last_of(L"\\/");
	if (pos != std::wstring::npos)
	{
		dir = dir.substr(0, pos);
	}
	dir += L"\\dumps";
	CreateDirectoryW(dir.c_str(), NULL);
	return dir;
}

std::wstring CCreateDump::BuildDumpFilePath(const wchar_t* reason)
{
	SYSTEMTIME syt;
	GetLocalTime(&syt);

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	std::wstring prefix = converterX.from_bytes(strDumpFile.empty() ? "app_dump" : strDumpFile);
	std::wstringstream ss;
	ss << GetDumpDirectory()
		<< L"\\"
		<< syt.wYear
		<< L"-";
	ss.width(2); ss.fill(L'0'); ss << syt.wMonth;
	ss << L"-";
	ss.width(2); ss.fill(L'0'); ss << syt.wDay;
	ss << L"_";
	ss.width(2); ss.fill(L'0'); ss << syt.wHour;
	ss << L"-";
	ss.width(2); ss.fill(L'0'); ss << syt.wMinute;
	ss << L"-";
	ss.width(2); ss.fill(L'0'); ss << syt.wSecond;
	ss << L"-";
	ss.width(3); ss.fill(L'0'); ss << syt.wMilliseconds;
	ss << L"_" << prefix;
	if (reason != nullptr && reason[0] != L'\0')
	{
		ss << L"_" << reason;
	}
	ss << L".dmp";
	return ss.str();
}
