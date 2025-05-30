/**
* @file            base_hex_api.h
* @brief           base hex for fortrend for gui
* @author		   xielonghua
*/

// Library: Fortrend
// Package: Api/HEX

#include "base_hex_api.h"
#include "efem_hex_api.h"

namespace FC{

BaseHexApi::BaseHexApi(EFEMHexApi* api)
	:api(api){

}


}