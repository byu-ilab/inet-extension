//==========================================================================80>
/**
 * \file TCPSocketAPI_Base.cc
 *
 * TCPSocketAPI_Base static function definitions.
 *
 * Created: August 2, 2010
 * 
 * @todo Add GPL notice.
 */

#include "TCPSocketAPI_Base.h"

//---

bool TCPSocketAPI_Base::isCallbackError(error_id_t error)
{
	switch(error)
	{
	case TCPSocketAPI_Base::CB_E_UNKNOWN:
	case TCPSocketAPI_Base::CB_E_CLOSED:
	case TCPSocketAPI_Base::CB_E_TIMEOUT:
	case TCPSocketAPI_Base::CB_E_RESET:
	case TCPSocketAPI_Base::CB_E_REFUSED:
		return true;
		break;
	default:
		return false;
	}
}

std::string TCPSocketAPI_Base::getCallbackErrorName(error_id_t error)
{
	switch(error)
	{
	case TCPSocketAPI_Base::CB_E_UNKNOWN:
		return "UNKNOWN";
		break;
	case TCPSocketAPI_Base::CB_E_CLOSED:
		return "CLOSED";
		break;
	case TCPSocketAPI_Base::CB_E_TIMEOUT:
		return "TIMEOUT";
		break;
	case TCPSocketAPI_Base::CB_E_RESET:
		return "RESET";
		break;
	case TCPSocketAPI_Base::CB_E_REFUSED:
		return "REFUSED";
		break;
	default:
		return "UNDEFINED";
	}
}

//---
