/**
* @file            fortrend_tm_cavity_defined.h
* @brief           Fortrend TM Cavity defined
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Subsystem/LoadLock


#ifndef _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_
#define _XLH_FORTREND_TM_CAVITY_SUBSYSTEM_DEFINED_INCLUDE_ 

namespace FC{


	/**
	* @brief    TM Cavity Valve Opening
	* @enum    TMCavityValveOpening
	*/
	typedef enum{
		TMCavity_Slow = 1, //慢充
		TMCavity_Fast = 2, //快充
		TMCavity_Both = 3  //

	} TMCavityValveOpening;
}
#endif
