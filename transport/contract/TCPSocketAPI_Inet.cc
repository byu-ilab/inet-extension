/**
 * \file TCPSocketAPI_Inet.cc
 *
 * Created: August 3, 2010
 * Author: Kevin Black
 *
 * @todo Check GPL notice.
 */

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

// From inet
#include "TCPSocketAPI_Inet.h"

//---

//bool TCPSocketAPI_Inet::isCallbackError(error_id_t error)
//{
//	return TCPSocketAPI_Base::isCallbackError(error);
//}

//std::string TCPSocketAPI_Inet::getCallbackErrorName(error_id_t error)
//{
//	return TCPSocketAPI_Base::getCallbackErrorName(error);
//}

//---

void TCPSocketAPI_Inet::CallbackHandler::recvCallback(
	socket_id_t id, 
	cb_status_t return_value,
	cPacket * msg,
	user_data_ptr_t yourPtr )
{
	if (msg != NULL)
	{
		delete msg;
	}
}

//---

cb_inet_handler_ptr_t TCPSocketAPI_Inet::verifyCallbackHandlerType(cb_base_handler_ptr_t cbobj)
{
	cb_inet_handler_ptr_t inet_cbobj = dynamic_cast<cb_inet_handler_ptr_t>(cbobj);

	if (inet_cbobj == NULL)
	{
		throw cRuntimeError("Pointer is not of type TCPSocketAPI_Inet::CallbackHandler.");
	}
	//else
	return inet_cbobj;
}

socket_id_t TCPSocketAPI_Inet::socket(cb_base_handler_ptr_t cbobj)
{
	return this->socket( verifyCallbackHandlerType(cbobj) );
}

void TCPSocketAPI_Inet::listen (
	socket_id_t id,
	cb_base_handler_ptr_t cbobj_for_accepted)
{
	if (cbobj_for_accepted == NULL)
	{
		return this->listen(id, static_cast<cb_inet_handler_ptr_t>(NULL));
	}
	else
	{
		return this->listen(id, verifyCallbackHandlerType(cbobj_for_accepted));
	}
}

