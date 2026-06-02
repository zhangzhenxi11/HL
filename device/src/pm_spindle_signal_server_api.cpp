/**
* @file            pm_spindle_signal_server_api.cpp
* @brief           PM spindle signal server api implementation
* @author          zhangzhenxi
*/

#include "pm_spindle_signal_server_api.h"
#include "Kernel/kernel_log.h"
#include "Kernel/kernel_exception.h"
#include "Poco/Format.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>

KERNEL_NS_BEGIN

PMSpindleSignalServerApi::PMSpindleSignalServerApi(IKernel* kernel)
    : TcpServerApi(kernel)
    , last_send_ready_time(std::chrono::system_clock::now())
{
}

void PMSpindleSignalServerApi::initialize() throw(KernelException)
{
    TcpServerApi::initialize();
}

void PMSpindleSignalServerApi::unInitialize()
{
    TcpServerApi::unInitialize();
}

void PMSpindleSignalServerApi::process()
{
    TcpServerApi::process();
    if (!isCommunicationEnabled()) return;

    auto time = std::chrono::system_clock::now();
    int pass = std::chrono::duration_cast<std::chrono::seconds>(time - last_send_ready_time).count();

    if ((pass > 2) && (CommunicationState::NOT_COMMUNICATING == getCommunicationState()) && isOnline())
    {
        std::string msg = "READY";
        std::transform(msg.begin(), msg.end(), msg.begin(),
            [](unsigned char c) { return static_cast<char>(::toupper(c)); });
        if (sendMessage(msg.data(), static_cast<unsigned int>(msg.size()))) {
            last_send_ready_time = time;
        }
    }
}

void PMSpindleSignalServerApi::configure(const std::shared_ptr<KernelConfiguration>& config) throw(KernelException)
{
    TcpServerApi::configure(config);
    logInform(getName().c_str(), "PMSpindleSignalServer configured successfully");
}

std::string getCurrentTimestamp()
{
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

bool PMSpindleSignalServerApi::notifyAxisArrivedProcess(int pmIndex, const std::string& position)
{
    return sendSignalToClients("PM_AXIS_ARRIVED_PROCESS", pmIndex, position);
}

bool PMSpindleSignalServerApi::notifyAxisDepartedProcess(int pmIndex, const std::string& position)
{
    return sendSignalToClients("PM_AXIS_DEPARTED_PROCESS", pmIndex, position);
}

bool PMSpindleSignalServerApi::notifyStartProcess(int pmIndex, const std::string& position)
{
    return sendSignalToClients("PM_START_PROCESS", pmIndex, position);
}

bool PMSpindleSignalServerApi::notifyEndProcess(int pmIndex, const std::string& position)
{
    return sendSignalToClients("PM_END_PROCESS", pmIndex, position);
}

bool PMSpindleSignalServerApi::sendSignalToClients(const std::string& event, int pmIndex, const std::string& position)
{
    std::ostringstream oss;
    oss << "{"
        << "\"event\":\"" << event << "\","
        << "\"pm_index\":" << pmIndex << ","
        << "\"position\":\"" << position << "\","
        << "\"timestamp\":\"" << getCurrentTimestamp() << "\""
        << "}";

    std::string jsonData = oss.str();
    //logMessage(jsonData.c_str(), static_cast<unsigned int>(jsonData.length()), "PMSignal");
    return sendMessage(jsonData.c_str(), static_cast<unsigned int>(jsonData.length()));
}

//void PMSpindleSignalServerApi::onConnect()
//{
//    setRemoteOnLine();
//}
//
//void PMSpindleSignalServerApi::onDisConnect()
//{
//    setLocalOnLine();
//}

KERNEL_NS_END