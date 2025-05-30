/**
* @file            base_hex_api.h
* @brief           base hex for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Api/HEX

#ifndef _FORTREND_BASE_HEX_API_INCLUDE_ 
#define _FORTREND_BASE_HEX_API_INCLUDE_
#include "Kernel/kernel_exception.h"
#include "kernel/kernel_event_module.h"

#include <memory>
#include <map>
#include <functional>

namespace FC{
class EFEMHexApi;

/**
* hex api of kernel api
*/
class  BaseHexApi :public KernelEventListener
{
public:
	DECLARE_PTR(BaseHexApi)
	typedef std::map<int, std::function<bool (const char* data, size_t len)>> HandlerMap; 
public:
	BaseHexApi(EFEMHexApi* api);
	virtual ~BaseHexApi(){}
	virtual bool handle(const char* data, size_t len) = 0;
	virtual void AddEventListener()=0;
protected:
	EFEMHexApi* api;
};



}

#endif
