/**
* @file            fortrend_loadlock_defined.h
* @brief           Fortrend LoadLock defined
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock


#ifndef _XLH_FORTREND_LOADLOCK_SUBSYSTEM_DEFINED_INCLUDE_
#define _XLH_FORTREND_LOADLOCK_SUBSYSTEM_DEFINED_INCLUDE_ 

namespace FC{

	/**
	* @brief    LoadLock Error Command
	* @enum    LoadLock Error Command
	*/
	typedef enum{
		Reset = 1,
		MoveToSlot = 2,
		Mappping = 3,
		
	} LoadLockErrorCommand;
	/**
	* @brief    LoadLock Valve Opening
	* @enum    LoadLockValveOpening
	*/
	typedef enum{
		LoadLock_Slow = 1, //慢充
		LoadLock_Fast = 2, //快充
		LoadLock_Both = 3, //全开

	} LoadLockValveOpening;
}
#endif
