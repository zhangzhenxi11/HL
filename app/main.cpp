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

int main(int argc, char *argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	//高分辨率自动缩放
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	 FC::CoreRunner app(argc, argv); 
	 CCreateDump::Instance()->DeclarDumpFile("app_dump");
	 qDebug() << "The application Dump file has been set";
	if (app.isRunning()){
		QMessageBox::warning(0, "Warn", "The gui is already running!!!");
		return 0;
	}
	if (app.init()){
		return  FC::CoreRunner::exec();
	}
}
