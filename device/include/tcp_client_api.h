/**
* @file            tcp_client_api.h
* @brief           tcp client
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/TcpClient

#ifndef _XLH_FORTREND_TCP_CLIENT_API_INCLUDE_ 
#define _XLH_FORTREND_TCP_CLIENT_API_INCLUDE_
#include "Kernel/kernel_api.h"
#include "Kernel/kernel_exception.h"
#include "Kernel/Fortrend/fortrend_macros.h"
#include <memory>



KERNEL_NS_BEGIN
class IKernel;

/**
*@brief  tcp client api of kernel
*/
class  TcpClientApi : public KernelAbstractApi//KernelFortrend_API
{
public:
	DECLARE_PTR(TcpClientApi)
	TcpClientApi(IKernel*  kernel);
public:
	virtual bool enable();
	virtual bool disable();
	virtual void setOffLine();
	virtual void setLocalOnLine();
	virtual void setRemoteOnLine();
	virtual ControlState getControlState()const;
	virtual CommunicationState getCommunicationState()const;
	virtual bool isCommunicationEnabled()const;
	virtual std::string getName()const override { return "EFEMClient"; };
public:
	virtual bool sendMessage(const char* data, unsigned int len);
protected:
	virtual void initialize() throw(KernelException)override;
	virtual void unInitialize()override;
	virtual void process()override;
	virtual void configure(const std::shared_ptr<KernelConfiguration> & config) throw(KernelException) override;
	void setCommunicationState(KernelApi::CommunicationState communicationState);
	
	//connect / disconnect event
	virtual void onConnect() {}
	virtual void onDisConnect(){}
	virtual void logMessage(const char* data, unsigned int len, const std::string& prefix);

protected:
	//buffer operation
	void	clearBuffer();
	size_t  readBuffer(char* data, size_t btr);
	size_t  peekBuffer(size_t skip_count, char* data, size_t btp);
	size_t  getBufferSize() const;

private:
	DECLARE_PRIVATE(TcpClientApi)
};



KERNEL_NS_END

#endif
