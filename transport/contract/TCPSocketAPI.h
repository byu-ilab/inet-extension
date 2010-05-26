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
Provides a convenient way of managing TCPSockets.

NOTES TO USER:
	OVERVIEW:
	This TCP socket API is similar to the BSD socket API.  It provides
	functions like socket, bind, connect, send, recv, listen, accept,
	and close.  A socket descriptor is returned from the function socket
	which can thereafter be used to perform other operations on that socket.
	Convenience functions to make and active or passive socket are also
	provided.  Timeouts currently only work with the recv operation.

	The major difference from the BSD API is that this API is completely
	asynchronous so as to fit into the asynchronous architecture of OMNET++.
	Traditionally the functions connect, accept, and recv are blocking
	system calls, meaning that they do not return until a socket is connected,
 	accepted, or data received respectively.  To make these operations
 	work in an asynchronous environment a callback must be used.

	CALLBACK INTERFACE:
 	The callback interface specified by this API defines a corresponding
 	callback function for the functions connect, accept, and recv.  When you
 	first create a socket you must provide a pointer to an object implementing
 	the interface.  Thereafter, when you call connect, accept, or recv you
 	are only indicating to the API that you want that operation to be simulated
 	and/or the function's corresponding callback to be invoked when the
 	simulation of that operation is finished.  After calling connect, accept,
 	or recv your code should then surrender control back to the simulator (i.e.
 	return from the function) so that the simulation can continue.

 	When a socket is finally connected (or fails to connect) the connectCallback
 	is invoked.  When a new connection is accepted on a listening socket the
 	acceptCallback is invoked.  When data is received on a connection (or the
 	connection closes or times out) the recvCallback is invoked.

 	Your implementation of the callback should handle the event and then invoke
 	the next socket calls so that operation will continue smoothly.  The functions
 	accept and recv are not persistent, meaning that you may not invoke them once
 	and expect the corresponding callback to be invoked whenever a connection is
 	accepted or reception occurs on a connection. Thus, if you need to do repeated
 	accept or recv operations remember to reinvoke them in the acceptCallback and
 	recvCallback functions you define; in a synchronous environment this is in
 	essence what a loop in an accepting or receiving thread does.

	When you invoke	connect, accept, or recv you can hand in a pointer to whatever
	data/struct/object that you want to (cast it as a void pointer).  The API will
 	not modify the element pointed to in any way.  When the simulation of a
 	connect, accept, or recv event is complete the corresponding callback will
 	be invoked and the void pointer previously provided will be returned.
 	If desired, this approach can be used to store context associated with a
 	specific socket instead of maintaining a context mapping.

	EXAMPLE:
	In a synchronous environment a common sequence of socket operations is as
	follows (without specific regard to the BSD API):

	<pre>
	some thread ...
		fd = socket()
		bind(fd, local_address, local_port)
		connect(fd, remote_address, remote_port)
		send(fd, some_data)
		repeat until some condition
			data = recv(fd)
			# process the data #
			send(fd, some_more_data)
	</pre>

	Using this asynchronous API, the same functionality can be achieved as
	follows:

	<pre>
	function: connectionInitializer
		fd = socket(pointer to callback interface object)
		bind(fd, local_address, local_port)
		connect(fd, remote_address, remote_port, context_data)
	# end of connectionInitializer #

	function: connectCallback(fd, status, context_data)
		send(fd, some_data)
		recv(fd, context_data)
		// indicate that we want the recvCallback to be invoked when response
		// data is received
	# end of connectCallback #

	function: recvCallback(fd, status, data, context_data)
 		# process the data #
 		send(fd, some_more_data)
		recv(fd, context_data)
		// indicate that we want the recvCallback to be invoked when response
		// data is received
 	# end of recvCallback #
 	</pre>

 	MEMORY MANAGEMENT:
 	The API never assumes responsibility for deleting (from heap space) any
 	information provided in a connect, accept, or recv invocation.  The API will,
 	however, track the provided pointer until a connect, accept, or recv event
 	occurs and hand back the pointer in the corresponding callback.  When the
 	callback is	invoked the API stops tracking the pointer (internally it sets
 	its pointers to NULL).  It is thus your	responsibility to handle the pointer
 	-- either reusing it in a subsequent connect, accept, or recv invocation or
 	to delete it.  When a socket is	closed it will return a void pointer; in the
 	event that close is invoked	after a connect, accept, or recv invocation but
 	before such an event occurs then the pointer provided in that function
 	invocation will	be returned, otherwise NULL will be returned.



NOTES TO CONTRIBUTOR:
	OVERVIEW:
	The API was constructed taking advantage of the already existent TCPSocket
	and TCPSocketMap classes.  The TCPSocket's callback interface is based on
	events more than on socket operations, so one thing that this API does is
	to interpret those events back to the appropriate operations (e.g. the
	socketEstablished callback could be either for a connect or accept operation).
	One operation that the TCPSocket doesn't have is recv, but it does define a
	socketDataArrived callback.  This API interprets the socketDataArrived event
	as a recv operation.

	To support the connect, accept, and recv operations, this API uses and
	manages callback state data to track what operation the user has most recently
	requested.  Not all of the enumerated callback states pertain to an operation's
	callback function (e.g. CB_S_WAIT), rather they indicate whether an operation
	that requires a callback can be validly invoked as well as indicating how an
	event signaled by the TCPSocket's callback interface should be handled.  For
	example recv cannot be invoked on a socket before connect, this is enforced
	by not setting the socket's associated callback data state to CB_S_WAIT until
	the socket is actually connected.  In addition, this API imposes a timed out
	state on sockets using the CB_S_TIMEOUT value while the underlying TCPSocket's
	actual state is still CONNECTED.  This is a messy approach conceptually but
	required because the TCPSocket is not a cSimpleModule so it can't schedule
	timeout events.  This API itself could be improved by making a new TCP socket
	class that would have access to scheduling events through the API and which
	could then be tasked with keeping track of a TIMED_OUT state, provide a recv
	operation, and which demultiplex TCP message events directly to opertions.


  	ASSUMPTIONS:
  	- Because the API forces close to be called before a port can be reused via the
  	bind function, the close command will reach the TCP core before the new connect
  	or listen command such that the port will successfully be reused.

  	TO DOCUMENT:
  	@todo document design decisions/considerations

  	TO IMPROVE:
  	@todo intercept to represent receiving, accepting, timed out, etc. OR make
  	an entirely new TCPSocket class
  	@todo make different send modes, i.e. can invoke send whenever the underlying
 	socket is connected or connecting, OR lock-step sending and receiving
  	@todo make a way to set error handling (i.e. throw or just return error codes and set an
  	error message)
  	@todo build a send function that will take a translator and data to get the cMessage

  	@see TCPSocket, TCPSocketMap
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
	/// Tracks the TCPSocket objects.
	TCPSocketMap _socket_map;

	/// Tracks the timeout messages associated with a given socket.
	///
	/// maps socket id to timeout message
	std::map<int, SocketTimeoutMsg *> _timeout_timers;

	/// Used to resolve provided string addresses to IPvXAddress
	/// objects.
	IPAddressResolver _resolver;

	/// Tracks the ports that are being used (bound) and which
	/// socket(s) is using it.
	///
	/// maps port to socket id
	std::map<int, std::set<int> > _bound_ports;

	/// Values to control socket operation sequence, and to indicate
	/// which callback should be invoked and how to interpret events
	/// defined by the TCPSocket::CallbackInterface.
	///
	/// CB_S_NONE -- no callback requiring operation invoked and/or
	///		an invalid state
	/// CB_S_CONNECT -- the connect operation was invoked and the
	///		connectCallback should be called when a connection is
	///		established or an error occurs
	/// CB_S_ACCEPT -- the accept operation was invoked and the
	///		acceptCallback should be called when a connection is
	///		accepted
	/// CB_S_RECV -- the recv operation was invoked and the
	///		recvCallback should be called when data is received on
	///		the connection, the connection closes or times out
	///		or an error occurs
	/// CB_S_CLOSE -- the close operation was invoked so no other
	///		socket calls are valid and/or the connection was closed
	///		so only the close operation can be invoked
	/// CB_S_TIMEOUT -- the connection is timed out, no socket
	///		operations are valid except close
	///	CB_S_WAIT -- the socket is waiting for an accept or
	///		recv operation to be invoked and/or a connection was
	///		accepted or data was received but the user didn't
	///		reinvoke accept or recv
	enum CALLBACK_STATE {CB_S_NONE, CB_S_CONNECT, CB_S_ACCEPT,
			CB_S_RECV, CB_S_CLOSE, CB_S_TIMEOUT, CB_S_WAIT};

	/// Data necessary to execute callbacks associated with a given
	/// socket.
	struct CallbackData {
		/// The socket this callback data pertains to and the socket
		/// id to be returned in the callback
		int socket_id;

		/// The callback state of the socket.  See CALLBACK_STATE.
		CALLBACK_STATE state;

		/// The CallbackInterface assigned to handle events on this
		/// socket.
		CallbackInterface * cbobj;

		/// The data/struct/object the user provides.
		void * userptr;

		/// The CallbackInterface to be used by accepted connections.
		/// Useful if the user defines different handlers for active
		/// and passive sockets.  May be NULL in which case cbobj
		/// is used.
		CallbackInterface * cbobj_for_accepted;
	};

	/// Tracks the callback data for passive sockets.
	///
	/// Note that this separate map is needed because the the _bound_ports
	/// map doesn't have a feature to distinguish passive sockets
	/// from the active sockets that have been accepted on the listening
	/// socket's port.
	///
	/// maps port to callback data
	std::map<int, CallbackData *> _accept_callbacks;

	/// Tracks the callback data associated with a given socket.
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
