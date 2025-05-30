// Library: Fortrend
// Package: CommandImp/Rnd/Aligner
//
// status command for rnd subsystem
//
// author xielonghua
//


#include "EFEM/efem_aligner_status_command.h"
#include "EFEM/efem_aligner_subsystem.h"
#include "Kernel/Fortrend/fortrend_cassette_manager.h"
#include "Kernel/kernel.h"
#include "Kernel/kernel_command_reject_exception.h"
#include <thread>

#include "Poco/Format.h"

KERNEL_NS_BEGIN

#if _MSC_VER >1600
#pragma execution_character_set("utf-8")
#endif

class EFEMAlignerStatusCommandPrivate{
public: 
	
};


EFEMAlignerStatusCommand::EFEMAlignerStatusCommand(TcpEfemSubSystemHelper* hexHelper)
	:TcpEfemCommandExecuter(hexHelper), d(new EFEMAlignerStatusCommandPrivate)
{
 
}
 

/**
* return true if success else false.
*/
IKernelCommand::RunResult EFEMAlignerStatusCommand::onRun() throw(KernelException){
	std::string res;
	auto aligner = dynamic_cast<EFEMAlignerSubsystem*>(getSubsystem());
	if (!aligner){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
			"aligner status command must belong to Aligner.",this);
	}
	auto cassManager = aligner->getKernel()->getKernelModule<FortrendCassetteManager>();
	//get cass
	Cassette::Ptr cass = cassManager->getCassette(aligner);
	if (!cass){
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_SYSTEM_LOGIC_ERROR, 
			Poco::format("Aliger %s must has logic cassette.", aligner->getName()),this);
	}
	std::shared_ptr<KernelConfiguration> command_config = aligner->getConfigure()->createView(getName());

	//fill params
	int timeout = command_config->getInt("timeout", -1);//20000ms
	if (timeout < 10) {
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_DATA_OUTOF_RANGE,
			Poco::format("³¬Ê±: »ñÈ¡×´Ì¬³¬Ê±²ÎÊýÉèÖÃÊ§°Ü", aligner->getName()), this);
	}

	//GET:MAPDT/ALIGNER;
	std::string stationName = aligner->getName(); //EALIGNER
	std::string command = Poco::format("GET:MAPDT/%s", stationName);
	command.push_back(';');
	if (!sendRequest(command))
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT, 
			Poco::format("»ñÈ¡Ñ°±ßÆ÷×´Ì¬Ö¸Áî·¢ËÍÊ§°Ü:", command).c_str(), this);
	}
	auto startTime = std::chrono::high_resolution_clock::now();
	std::string strifInf = "GET";
	while (true)
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

		strifInf = recvResponse(timeout);

		if (strifInf.find("INF:MAPDT") != std::string::npos)
		{
			break;
		}
		if (elapsed.count() >= timeout / 1000)
		{
			throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
				"»ñÈ¡Ñ°±ßÆ÷×´Ì¬ÃüÁîÖ´ÐÐ³¬Ê±", this);
		}
	}
	//INF:MAPDT/EALIGNER/E;
	//INF:MAPDT/EALIGNER/P;
	char mapdt;
	size_t lastSlash = strifInf.rfind('/');
	size_t semicolon = strifInf.rfind(';');

	if (lastSlash != std::string::npos &&
		semicolon != std::string::npos &&
		lastSlash < semicolon - 1)
	{
		mapdt = strifInf[lastSlash + 1];
	}
	else
	{
		throw KernelCommandRejectException(__FILE__, KernelSysException::KR_COMMON_COMMAND_NO_SUPPORT,
			"´¦Àí×Ö·û´®´íÎó", this);
	}
	Cassette::Mapping map = getMappingChar(mapdt);
	cass->setMapping(1, map);
	logInform(aligner->getName().c_str(), Poco::format("»ñÈ¡Ñ°±ßÆ÷¾§Ô²×´Ì¬ %s ÃüÁîÖ´ÐÐ½áÊø", aligner).c_str());

	return RunResult::RUN_OK;
}

KERNEL_NS_END
