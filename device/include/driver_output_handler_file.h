/**
* @file            driver_output_handler_file.h
* @brief           driver output handler file
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Main

#ifndef _DRIVER_OUTPUT_HANDLER_FILE_INCLUDE_
#define _DRIVER_OUTPUT_HANDLER_FILE_INCLUDE_

#include <memory>
#include <QObject>
#include "Core/modulebase.hpp"
#include "Kernel/kernel_log.h"

namespace FC{


	/**
	*  driver output handler file
	*/
	class  DriverOutputHandlerFile :public QObject, public KernelLog::OutputHandler
	{
		Q_OBJECT
	public:
		DriverOutputHandlerFile();
		~DriverOutputHandlerFile();

	public:
		virtual void log(const std::string &category, const std::string &text, KernelLog::VisibleLevel visible, KernelLog::LogLevel level, const char *filename, int line) override;

	private:
		std::shared_ptr<KernelLog::OutputHandlerSTD> stdOut;
	};

}
#endif
