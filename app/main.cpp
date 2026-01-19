#include <QApplication>
#include "CoreRunner/core_runner.hpp"
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <cstdlib>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngine/QtWebEngine>
#ifdef _WIN32
#include <Windows.h>
#include <Dbghelp.h>
#include <iostream>
#include <QDebug.h>
#include "CCreateDump.h"
#endif

int main(int argc, char *argv[])
{
	//首先设置 OpenGL 上下文共享(必须在第一行)
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif
	//高分辨率自动缩放
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	
	// 创建应用程序对象
	 FC::CoreRunner app(argc, argv);
	
	// 初始化QtWebEngine - 必须在QApplication创建之后调用
	QtWebEngine::initialize();
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
