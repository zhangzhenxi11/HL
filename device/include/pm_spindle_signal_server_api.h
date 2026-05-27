/**
* @file            pm_spindle_signal_server_api.h
* @brief           PM spindle signal server api for notifying axis position changes
* @author          zhangzhenxi
*/

// Library: Fortrend
// Package: Api

#ifndef _FORTREND_PM_SPINDLE_SIGNAL_SERVER_API_INCLUDE_
#define _FORTREND_PM_SPINDLE_SIGNAL_SERVER_API_INCLUDE_

#include "Kernel/Fortrend/tcp_server_api.h"
#include "Kernel/kernel_exception.h"
#include <memory>
#include <string>
#include <chrono>

KERNEL_NS_BEGIN

class IKernel;

class PMSpindleSignalServerApi : public TcpServerApi
{
public:
    DECLARE_PTR(PMSpindleSignalServerApi)
    PMSpindleSignalServerApi(IKernel* kernel);
    virtual std::string getName() const override { return "PMHOST"; }

    bool notifyAxisArrivedProcess(int pmIndex, const std::string& position);  //到达
    bool notifyAxisDepartedProcess(int pmIndex, const std::string& position); //离开
	bool notifyStartProcess(int pmIndex, const std::string& position); //开始工艺
	bool notifyEndProcess(int pmIndex, const std::string& position);   //结束工艺

protected:
    virtual void initialize() throw(KernelException) override;
    virtual void unInitialize() override;
    virtual void process() override;
    virtual void configure(const std::shared_ptr<KernelConfiguration>& config) throw(KernelException) override;

private:
    bool sendSignalToClients(const std::string& event, int pmIndex, const std::string& position);
    std::chrono::system_clock::time_point last_send_ready_time;
};

KERNEL_NS_END

#endif