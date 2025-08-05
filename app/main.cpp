#include <QApplication>
#include "CoreRunner/core_runner.hpp"
#include <QMessageBox>
#ifdef _WIN32
#include <Windows.h>
#include <Dbghelp.h>
#include <iostream>

#endif



int main(int argc, char *argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	
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