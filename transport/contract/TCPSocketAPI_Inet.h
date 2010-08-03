/**
 * \file TCPSocketAPI_Inet.h
 *
 * Created: August 2, 2010
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

#ifndef __INET__TCPSOCKETAPI_INET_H_
#define __INET__TCPSOCKETAPI_INET_H_

// From inet
#include "TCPSocketAPI_Base.h"
#include "INETDefs.h"

/**
 * Extends the TCPSocketAPI_Base.  Intended for use with in the INET 
 * framework.  Specifies that the setTimeout function accepts time values
 * of type simtime_t (i.e. SimTime object), that the send function accepts
 * cPacket pointers, and that the recvCallback returns a cPacket pointer.
 * 
 * @see TCPSocketAPI_Base
 */
class INET_API TCPSocketAPI_Inet : public TCPSocketAPI_Base
{

public:

	/** @name Callback Handler extension */
	//@{

	/**
	 * @return True if the provided value corresponds to one of the
	 * values from the TCPSocketAPI_Base::CallbackError enumeration
	 * and false if it does not.
	 */
	static bool isCallbackError(error_id_t error);


	/**
	 * @return The name of the error corresponding to the indicated
	 * value from the TCPSocketAPI::CallbackError enumeration; the
	 * default is "UNDEFINED".
	 */
	static std::string getCallbackErrorName(error_id_t error);
	
	/** Defines a CallbackHandler extension with a recvCallback*/
	class CallbackHandler : public TCPSocketAPI_Base::CallbackHandler
	{
	public:
		/**
		 * By default just deletes the received cPacket.  Subclasses should handle
		 * the reception of the data specific to their application.  Subclasses should
		 * also assume responsibility for the user data pointer if it is non-NULL (e.g.
		 * reuse it or deallocate it).  Subclasses should also assume responsibility
		 * for the message pointer if it is non-NULL.
		 *
		 * @param id -- The descriptor of the receiving socket.
		 * @param return_value -- The return value of the previously invoked recv()
		 * 		method or the number of bytes received.
		 * @param msg -- A pointer to the received message.
		 * @param yourPtr -- The pointer to the data/struct/object previously passed
		 * 		to recv.
		 *
		 * On success: msg will point to the received message and return_value will be
		 * 		the number of bytes in the message.
		 *
		 * On error: msg will point to NULL and return_value will be a value from the
		 * TCPSocketAPI::CALLBACK_ERROR enumeration.
		 */
		virtual void recvCallback(socket_id_t id, 
				cb_status_t return_value,
				cPacket * msg,
				user_data_ptr_t yourPtr);
	};

	typedef TCPSocketAPI_Inet::CallbackHandler * cb_inet_handler_ptr_t;

	//@}



	/** @name Virtual Destructor */
	//@{

	/** Empty virtual destructor. */
	virtual ~TCPSocketAPI_Inet () {}

	///@}



	/** @name BSD-like functions */
	//@{

	/**
	 * Creates a new socket.
	 * 
	 * Checks that the callback handler is of type 
	 * TCPSocketAPI_Inet::CallbackHandler and calls the socket function
	 * specific to that type of handler.
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket; must
	 * 	not be NULL.
	 *
	 * @return The id/descriptor of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t socket(cb_base_handler_ptr_t cbobj);


	/**
	 * Creates a new socket.
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket; must
	 * 	not be NULL.
	 *
	 * @return The id/descriptor of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t socket(cb_inet_handler_ptr_t cbobj) =0;


	/**
	 * Makes the specified socket a passive socket.
	 * 
	 * If a non-NULL callback handler is specified for the accepted sockets, checks
	 * that the callback handler is of type TCPSocketAPI_Inet::CallbackHandler and
	 * calls the listen function specific to that type of handler.
	 *
	 * The socket must be bound to a specific port (call bind() without -1) before
	 * it can be made passive.
	 *
	 * @param id -- The descriptor to identify the socket to be made into a
	 * 		passive (listening) socket.
	 * @param cbobj_for_accepted -- A pointer to an object implementing the
	 * 		CallbackHandler to be used as the callback object for sockets spawned
	 *	 		(accepted) by this listening socket; default is NULL in which case the
	 *	 		spawned sockets will use the listening socket's callback handler.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void listen (socket_id_t id,
				cb_base_handler_ptr_t cbobj_for_accepted=NULL);


	/**
	 * Makes the specified socket a passive socket.
	 * 
	 * The socket must be bound to a specific port (call bind() without -1) before
	 * it can be made passive.
	 *
	 * @param id -- The descriptor to identify the socket to be made into a
	 * 		passive (listening) socket.
	 * @param cbobj_for_accepted -- A pointer to an object implementing the
	 * 		CallbackHandler to be used as the callback object for sockets spawned
	 *	 		(accepted) by this listening socket; default is NULL in which case the
	 *	 		spawned sockets will use the listening socket's callback handler.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void listen (socket_id_t id,
				cb_inet_handler_ptr_t cbobj_for_accepted) =0;


	/**
	 * Sends the indicated message on the specified socket.
	 *
	 * Currently the whole message is sent so there isn't a need to check for how
	 * many bytes were sent.  Takes control of the message (see documentation for
	 * cOwnedObject for more details on message taking).
	 *
	 * @param socket_id -- The descriptor to identify the (active) socket to send
	 * 		the message on.
	 * @param msg -- The message to be sent.
	 * 
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void send (socket_id_t id, cPacket * msg) =0;


	/**
	 * Sets a timeout on the indicated (active) socket.
	 *
	 * @param socket_id -- The descriptor to identify the (active) socket that should
	 *  	have a timeout set.
	 * @param timeout_period -- The period after which the socket should timeout if
	 * 	there has been no activity; must not be negative.
	 * 
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void setTimeout(socket_id_t id,
			simtime_t timeout_period) =0;

	//@}


protected:
	/** @name CallbackHandler checking */
	//@{

	/**
	 * Verifies that the specified callback handler is of type
	 * TCPSocketAPI_Inet::CallbackHandler.
	 *
	 * @return The callback handler as type TCPSocketAPI_Inet::CallbackHandler.
	 * 
	 * @throw Throws a std::exception if the callback handler is
	 * not of type TCPSocketAPI_Inet::CallbackHandler.
	 */
	virtual cb_inet_handler_ptr_t verifyCallbackHandlerType(cb_base_handler_ptr_t cbobj);

	//@}
};

// After the fact type def
typedef TCPSocketAPI_Inet::cb_inet_handler_ptr_t cb_inet_handler_ptr_t;

#endif /* __INET__TCPSOCKETAPI_INET_H_ */
