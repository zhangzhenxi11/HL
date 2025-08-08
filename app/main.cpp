#include <QApplication>
#include "CoreRunner/core_runner.hpp"
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#include <Dbghelp.h>
#include <iostream>
#include <QDebug.h>

#include "CCreateDump.h"
#endif

void  TestDumpGeneration()
{
	std::ofstream ofs("test.txt", std::ios::app);
	ofs << "begin!!!" << std::endl;
	ofs.close();

	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;

	CCreateDump::Instance()->DeclarDumpFile("dumpfile");

	int* p = NULL;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	*p = 5;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;
	std::cout << "11111111111111111" << std::endl;

	std::ofstream ofs1("test.txt", std::ios::app);
	ofs1 << "end!!!" << std::endl;
	ofs1.close();
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	//TestDumpGeneration();
	//高分辨率自动缩放
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	 FC::CoreRunner app(argc, argv); 
	if (app.isRunning()){
		QMessageBox::warning(0, "Warn", "The gui is already running!!!");
		return 0;
	}

	if (app.init()){
		return  FC::CoreRunner::exec();
	}
}