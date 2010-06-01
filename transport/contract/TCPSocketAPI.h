// Author: Kevin Black
//
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

#ifndef __INET_TCPSOCKETAPI_H
#define __INET_TCPSOCKETAPI_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "httptLogdefs.h"

#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "IPAddressResolver.h"
#include "SocketTimeoutMsg_m.h"
#include "TCPCommand_m.h"

#include <string.h>
#include <map>
#include <set>

/**
 * Provides a convenient way of managing TCPSockets similar to the BSD socket API.
 *
 * Notes to user:
 *
 * Notes to contributor:
 * 		Assumptions:
 * 		- Because the API forces close to be called before a port can be reused via the
 * 		bind function, the close command will reach the TCP core before the new connect
 * 		or listen command such that the port will be assigned for use successfully.
 *
 * 		To Document
 * 		@todo document usage
 *
 * 		To Improve
 * 		@todo intercept to represent receiving, accepting, timed out, etc. OR make
 * 		an entirely new TCPSocket class
 * 		@todo make different send modes, i.e. can invoke send whenever the underlying
 *		socket is connected or connecting, OR lock-step sending and receiving
 * 		@todo make a way to set error handling (i.e. throw or just return error codes and set an
 * 		error message)
 * 		@todo build a send function that will take a translator and data to get the cMessage
 */
class INET_API TCPSocketAPI : public cSimpleModule, TCPSocket::CallbackInterface
{
public:

	/** @name Callback Information */
	//@{

	/// The type of callbacks expected by the socket API.
	///
	/// CB_T_CONNECT -- callback to handle socket connection
	/// CB_T_ACCEPT  -- callback to handle socket acceptance
	/// CB_T_RECV    -- callback to handle socket reception
	enum CALLBACK_TYPE {CB_T_CONNECT, CB_T_ACCEPT, CB_T_RECV};

	/// Values that may be returned in callbacks to make error identification
	/// more convenient.
	///
	/// CB_E_UNKNOWN -- an unknown error occurred on socket connection
	/// CB_E_CLOSED  -- socket connection closed
	/// CB_E_TIMEOUT -- socket connection timed out
	/// CB_E_RESET   -- socket connection reset
	/// CB_E_REFUSED -- socket connection refused
	enum CALLBACK_ERROR {
		CB_E_UNKNOWN = -1,
		CB_E_CLOSED = -2,
		CB_E_TIMEOUT = -3,
		CB_E_RESET = -4,
		CB_E_REFUSED = -5
	};

	/// Defines the functions that must be implemented to make the TCPSocketAPI's
	/// callbacks work.
	class CallbackInterface {
	public:

		/// Indicates which callback functions are implemented.
		///
		/// @return true if the callback type's corresponding function is implemented
		/// and false if it is not.
		virtual bool hasCallback (CALLBACK_TYPE type) =0;

		/// Handles the connection of the specified socket.
		/// Corresponds to the CB_T_CONNECT value from the CALLBACK_TYPE enumeration.
		/// Assumes responsibility for yourPtr (either to delete it or reuse it).
		///
		/// @param socket_id -- the descriptor for the connected socket
		/// @param ret_status -- the status of the previously invoked connect method
		/// @param yourPtr -- the pointer to the data/struct/object previously passed
		///  	to connect
		///
		/// @details
		/// On success: ret_status will be 0 (zero)
		/// On error: ret_status will be a value from the CALLBACK_ERROR enumeration
		virtual void connectCallback (int socket_id, int ret_status, void * yourPtr) {}

		/// Handles the acceptance of a new socket.
		/// Corresponds to the CB_T_ACCEPT value from the CALLBACK_TYPE enumeration.
		/// Assumes responsibility for yourPtr (either to delete it or reuse it).
		///
		/// @param socket_id -- the descriptor for the listening socket
		/// @param ret_status -- the status of the previously invoked accept method or
		///		the descriptor for the accepted socket
		/// @param yourPtr -- the pointer to the data/struct/object previously passed
		///  	to accept
		///
		/// @details
		/// On success: ret_status will be the descriptor of the accepted socket
		/// On error: ret_status will be CB_E_UNKNOWN from the CALLBACK_ERROR enumeration
		virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr) {}

		/// Handles the reception of data on the specified socket.
		/// Corresponds to the CB_T_RECV value from the CALLBACK_TYPE enumeration.
		/// Assumes responsibility for msg (either to delete it or reuse it).
		/// Assumes responsibility for yourPtr (either to delete it or reuse it).
		///
		/// @param socket_id -- the descriptor of the receiving socket
		/// @param ret_status -- the status of the previously invoked recv method or
		///		the number of bytes received
		/// @param msg -- a pointer to the received message
		/// @param yourPtr -- the pointer to the data/struct/object previously passed
		///		to recv
		///
		/// @details
		/// On success: msg will point to the received message and ret_status will be
		///		the number of bytes in the message
		/// On error: msg will point to NULL and ret_status will be a value from the
		/// 	CALLBACK_ERROR enumeration
		virtual void recvCallback    (int socket_id, int ret_status, cPacket * msg,
										void * yourPtr) {}
	};

	//@}

protected:

	/** @name Instance Members */
	//@{
	TCPSocketMap _socket_map;

	/// maps socket id to timeout message
	std::map<int, SocketTimeoutMsg *> _timeout_timers;

	IPAddressResolver _resolver;

	///
	/// maps port to socket id
	std::map<int, std::set<int> > _bound_ports;

	enum CALLBACK_STATE {CB_S_NONE, CB_S_CONNECT, CB_S_ACCEPT,
			CB_S_RECV, CB_S_CLOSE, CB_S_TIMEOUT, CB_S_WAIT};

	struct CallbackData {
		int socket_id;
		CALLBACK_STATE state;
		CallbackInterface * cbobj;
		void * userptr;
		CallbackInterface * cbobj_for_accepted;
	};

	/// Tracks the callback data for listening sockets that are accepting.
	///
	/// maps port to callback data
	std::map<int, CallbackData *> _accept_callbacks;

	///
	/// maps socket id to callback data
	std::map<int, CallbackData *> _registered_callbacks;

	//@}

public:

	TCPSocketAPI ();
	virtual ~TCPSocketAPI ();

private: // prevent copying and assignment
	TCPSocketAPI(const TCPSocketAPI & other);
	TCPSocketAPI & operator=(const TCPSocketAPI & other);

public:

	/** @name TCPSocketAPI static functions */
	//@{
	static bool isCallbackError(int error);

	static std::string getCallbackErrorName(int error);

	static bool isCallbackType(int type);

	static std::string getCallbackTypeName(int type);
	//@}

	/** @name TCPSocketAPI member functions */
	//@{

	virtual int getLocalPort(int socket_id);
	virtual int getRemotePort(int socket_id);
	virtual IPvXAddress getLocalAddress(int socket_id);
	virtual IPvXAddress getRemoteAddres(int socket_id);

	/// Creates a new socket.
	///
	/// @param cbobj -- a pointer to an object implementing the callback interface
	///		may not be NULL
	///
	/// @return the id of the new socket
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual int socket (CallbackInterface * cbobj);

	/// Binds the specified socket to the indicated local address and local port.
	/// Via this function ports cannot be used concurrently (call close to free the port),
	/// however, accepting sockets will correctly spin off sockets that are active on the
	/// same port as the listening socket.
	///
	/// @param socket_id -- the descriptor to identify the socket to be bound
	/// @param local_address -- the address to bind to, if it is empty (i.e. "") then
	///		the socket will be bound to any available IP address
	/// @param local_port -- the local port to bind to, if it is -1 and local_address
	/// 	is specified then the socket will be bound to any available port
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void bind (int socket_id, std::string local_address, int local_port);

	/// Begins to connect the specified socket to the indicated remote address
	/// and remote port.  When the connection is established then the connectCallback
	/// function will be invoked on the socket's callback object.
	///
	/// @param socket_id -- the descriptor to identify the socket to be connected
	/// @param remote_address -- the remote address to connect to (module names are
	/// 	allowed as addresses)
	/// @param remote_port -- the remote port to connect to
	/// @param yourPtr -- a pointer to whatever data/struct/object you want, default
	///		is NULL
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void connect (int socket_id, std::string remote_address, int remote_port,
				void * yourPtr=NULL);

	/// Makes the specified socket a passive socket.  The socket must be bound to
	/// a specific port (call bind without -1) before it can be made passive.
	///
	/// @param socket_id -- the descriptor to identify the socket to be made into a
	/// 	passive (listening) socket
	/// @param cbobj_for_accepted -- a pointer to an object implementing the
	///		CallbackInterface to be used as the callback object for sockets spawned
	/// 	(accepted) by this listening socket, default is NULL in which case the
	/// 	spawned sockets will use this socket's callback object
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void listen (int socket_id, CallbackInterface * cbobj_for_accepted=NULL);

	/// Makes an active socket by invoking socket, bind, and connect.  It will skip
	/// bind (i.e. let connect fill in the address) if local_address is empty AND
	/// local_port is negative.
	///
	/// @param cbobj -- a pointer to an object implementing the callback interface
	///		may not be NULL
	/// @param local_address -- the address to bind to, if it is empty (i.e. "") then
	///		will bind to any available IP address
	/// @param local_port -- the local port to bind to, if it is -1 and local_address
	/// 	is specified then the socket will be bound to any available port
	/// @param remote_address -- the remote address to connect to (module names are
	/// 	allowed as addresses)
	/// @param remote_port -- the remote port to connect to
	/// @param yourPtr -- a pointer to whatever data/struct/object you want, default
	///		is NULL
	///
	/// @return the id of the new socket
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual int makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr=NULL);

	/// Makes a passive socket by invoking socket, bind, and listen.
	///
	/// @param cbobj -- a pointer to an object implementing the callback interface
	///		may not be NULL
	/// @param local_address -- the address to bind to, if it is empty (i.e. "") then
	///		will bind to any available IP address
	/// @param local_port -- the local port to bind to
	/// @param cbobj_for_accepted -- a pointer to an object implementing the
	///		CallbackInterface to be used as the callback object for sockets spawned
	/// 	(accepted) by this listening socket, default is NULL in which case the
	/// 	spawned sockets will use this socket's callback object
	///
	/// @return the id of the new socket
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual int makePassiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, CallbackInterface * cbobj_for_accepted=NULL);

	/// Signals to the (passive) socket that it should now accept incoming
	/// connections.  When a connection is actually accepted the acceptCallback function
	/// will be invoked on the socket's callback object.
	///
	/// @param socket_id -- the descriptor to identify the (passive) socket that should
	/// 	now accept incoming connections
	/// @param yourPtr -- a pointer to whatever data/struct/object you want, default
	///		is NULL
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void accept (int socket_id, void * yourPtr=NULL);

	/// Sends the indicated message on the specified socket.  Currently the
	/// whole message is sent so there isn't a need to check for how many bytes were
	/// sent.  Takes control of the message (see documentation for cOwnedObject for
	/// more details on message taking).
	///
	/// @param socket_id -- the descriptor to identify the (active) socket to send
	///		the message on
	/// @param msg -- the message to be sent
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void send (int socket_id, cMessage * msg);

	/// Signals to the (active) socket that it should receive incoming data.
	/// When data is actually received the recvCallback function will be invoked on the
	/// socket's callback object.
	///
	/// @param socket_id -- the descriptor to identify the (active) socket that should
	/// 	now receive incoming data
	/// @param yourPtr -- a pointer to whatever data/struct/object you want, default
	///		is NULL
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void recv (int socket_id, void * yourPtr=NULL);

	/// Sets a timeout on the indicated (active) socket.  Timeouts are currently
	/// only supported on sockets that have been signaled to receive (see recv).
	///
	/// @param socket_id -- the descriptor to identify the (active) socket that should
	/// 	have a timeout set.
	/// @param timeout_interval -- the interval on which the socket should timeout,
	/// 	MUST NOT be negative
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void setTimeout(int socket_id, simtime_t timeout_interval);

	/// Removes the timeout on the indicated (active) socket.
	///
	/// @param socket_id -- the descriptor to identify the (active) socket that should
	/// 	have its timeout removed.
	///
	/// @return true if there was a timeout set and it was removed and false otherwise
	virtual bool removeTimeout(int socket_id);

	/// Closes the specified socket.  The socket descriptor is now invalid for
	/// all socket operations (including getMyPtr).
	///
	/// @param socket_id -- the descriptor to identify the socket to be closed
	///
	/// @throws throws a cRuntimeError if an error occurs
	virtual void * close (int socket_id);

	/// Returns the pointer to the data/struct/object you specified in a connect,
	/// listen, or recv invocation on the specified socket.  Provided to facilitate
	/// tracking the data you associated with the socket outside callback invocations.
	///
	/// @param socket_id -- the descriptor to identify the socket whose associated
	/// 	data/struct/object is to be returned
	///
	/// @return the void pointer to the data previously set in a connect, accept, or
	/// recv call.  Returns NULL if socket_id doesn't pertain to a current socket,
	/// if the void pointer set in a previous connect, accept, or recv invocation
	/// was NULL, or if the data was already returned in a callback invocation.
	virtual void * getMyPtr (int socket_id);

	//@}

protected:

	/** @name Overridden functions from cSimpleModule */
	//@{
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
	//@}

	 /** @name Overridden functions from TCPSocket::CallbackInterface */
	//@{
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
	virtual void socketEstablished(int connId, void *yourPtr);
	virtual void socketPeerClosed(int connId, void *yourPtr);
	virtual void socketClosed(int connId, void *yourPtr);
	virtual void socketFailure(int connId, void *yourPtr, int code);

	// implement if other things should be done other than delete the status info
	//virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);

	//@}

	/** @name Utility / convenience functions */
	//@{

	/// NOT part of the TCPSocket::CallbackInteface
	virtual void socketTimeout(int connId, void * yourPtr);

	virtual void cleanupSocket(int socket_id);
	virtual void freePort(int socket_id);

	virtual TCPSocket * findAndCheckSocket(int socket_id, const std::string & fname);

	virtual std::string socketToString(int socket_id);

	virtual CallbackData * makeCallbackData(int socket_id, CallbackInterface * cbobj,
			void * function_data, CALLBACK_STATE type);

	virtual CallbackData * getAcceptCallback(int port);

	static std::string getStateName(CALLBACK_STATE state);

	virtual void signalFunctionError(const std::string & fname, const std::string & details);
	virtual void signalCBNullError(const std::string & fname);
	virtual void signalCBStateReceptionError(const std::string & fname, CALLBACK_STATE state);
	virtual void signalCBStateInconsistentError(const std::string & fname, CALLBACK_STATE state);

	virtual void printFunctionNotice(const std::string & fname, const std::string & notice);
	virtual void printCBStateReceptionNotice(const std::string & fname, CALLBACK_STATE state);
	//@}
};

#endif
