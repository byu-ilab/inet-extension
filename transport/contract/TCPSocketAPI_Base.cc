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

socket_id_t TCPSocketAPI_Base::makeActiveSocket (cb_base_handler_ptr_t cbobj,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port)
{
	return makeActiveSocket(cbobj, local_address,local_port,
			remote_address,remote_port,NULL);
}

socket_id_t TCPSocketAPI_Base::makeActiveSocket (cb_base_handler_ptr_t cb_handler,
	address_cref_t local_address, port_t local_port,
	address_cref_t remote_address, port_t remote_port, user_data_ptr_t context)
{
	//TODO need Enter_Method_Silent here or will it work fine if socket,bind,and connect have it?
	socket_id_t id = socket(cb_handler);
	// check if bind should be skipped
	if (local_address != ADDRESS_NULL || local_port != PORT_NULL)
	{
		bind(id, local_address, local_port);
	}
	connect(id, remote_address, remote_port, context);

	return id;
}

socket_id_t TCPSocketAPI_Base::makePassiveSocket (cb_base_handler_ptr_t cb_handler,
	address_cref_t local_address, port_t local_port,
	cb_base_handler_ptr_t cb_handler_for_accepted)
{
	//TODO need Enter_Method_Silent here or will it work fine if socket,bind,and listen have it?
	socket_id_t id = socket(cb_handler);

	bind(id, local_address, local_port);

	listen(id, cb_handler_for_accepted);

	return id;
}
