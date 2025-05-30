/**
* @file            driver_output_handler_file.h
* @brief           driver output handler file for fortrend
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "driver_output_handler_file.h"
#include "CorePlugin/messagemanager.hpp"
using namespace FC;

namespace FC{

	//driver log
	DriverOutputHandlerFile::DriverOutputHandlerFile()
	{
		stdOut.reset(new KernelLog::OutputHandlerSTD);

	}

	DriverOutputHandlerFile::~DriverOutputHandlerFile()
	{

	}

	void DriverOutputHandlerFile::log(const std::string &category, const std::string &text, KernelLog::VisibleLevel visible, KernelLog::LogLevel level, const char *filename, int line)
	{
		stdOut->log(category, text, visible, level, filename, line);
		//save to framework
		if (level > KernelLog::LogLevel::LOG_DEBUG){ //just push >DEBUG message
			QString cat = QString::fromStdString(category);
			MessageManager::instance()->push(cat + ":" + QString::fromStdString(text), "kernel_" + cat, (MsgType)level, true, false, visible == KernelLog::LOG_VISIBLE_LEVEL1);
			//MessageManager::instance()->push(cat + ":" + QString::fromLocal8Bit(text.c_str()), "kernel_" + cat, (MsgType)level, true, false, visible == KernelLog::LOG_VISIBLE_LEVEL1);

		}
	}
}



