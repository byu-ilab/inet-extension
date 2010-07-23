/**
 * \file TCPSocketAPI.h
 *
 * @todo Check GPL notice.
 *
 * Created: ?
 *
 * Author: Kevin Black
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

#ifndef __INET_TCPSOCKETAPI_H
#define __INET_TCPSOCKETAPI_H

// From omnetpp extension
#include <omnetppextension.h>
#include <cmessageeventobserver.h>

// From inet
#include "INETDefs.h"
#include "httptLogdefs.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "TCPPortRangeDefs.h"
#include "IPAddressResolver.h"
#include "SocketTimeoutMsg_m.h"
#include "TCPCommand_m.h"

// From standard C++ libraries
#include <string.h>
#include <map>
#include <set>
#include <deque>

/** Provides a convenient way of managing TCPSockets.

NOTES TO USER:

	OVERVIEW:

	This socket API for TCPSockets is similar to the BSD socket API.  It provides
	functions like socket, bind, connect, send, recv, listen, accept,
	and close.  A socket descriptor is returned from the function socket()
	which can thereafter be used to perform other operations on that socket.
	Convenience functions to make and active or passive socket are also
	provided.  Timeouts currently only work with the recv() operation.

	The major difference from the BSD API is that this API is completely
	asynchronous so as to fit into the asynchronous architecture of OMNET++.
	Traditionally the functions connect(), accept(), and recv() are blocking
	system calls, meaning that they do not return until a socket is connected,
 	accepted, or data received respectively.  To make these operations
 	work in an asynchronous environment a callback must be used.

	CALLBACK INTERFACE:
 	The callback interface specified by this API defines a corresponding
 	callback function for the functions connect(), accept(), and recv().  When you
 	first create a socket you must provide a pointer to an object implementing
 	the interface.  Thereafter, when you call connect(), accept(), or recv() you
 	are only indicating to the API that you want that operation to be simulated
 	and/or the function's corresponding callback to be invoked when the
 	simulation of that operation is finished.  After calling connect(), accept(),
 	or recv() your code should then surrender control back to the simulator (i.e.
 	return from the function) so that the simulation can continue.

 	When a socket is finally connected (or fails to connect) the connectCallback()
 	is invoked.  When a new connection is accepted on a listening socket the
 	acceptCallback() is invoked.  When data is received on a connection (or the
 	connection closes or times out) the recvCallback() is invoked.

 	Your implementation of the callback should handle the event and then invoke
 	the next socket calls so that operation will continue smoothly.  The functions
 	accept() and recv() are not persistent, meaning that you may not invoke them once
 	and expect the corresponding callback to be invoked whenever a connection is
 	accepted or reception occurs on a connection. Thus, if you need to do repeated
 	accept or recv operations remember to reinvoke them in the acceptCallback() and
 	recvCallback() functions you define; in a synchronous environment this is in
 	essence what a loop in an accepting or receiving thread does.

	When you invoke	connect(), accept(), or recv() you can hand in a pointer to whatever
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
		fd = socket ()
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
		# indicate that we want the recvCallback to be
		invoked when response data is received #
	# end of connectCallback #

	function: recvCallback(fd, status, data, context_data)
 		# process the data #
 		send(fd, some_more_data)
		recv(fd, context_data)
		# indicate that we want the recvCallback to be
		invoked when response data is received #
 	# end of recvCallback #
 	</pre>

 	MEMORY MANAGEMENT:

 	The API never assumes responsibility for deleting (from heap space) any
 	information provided in a connect(), accept(), or recv() invocation.  The API will,
 	however, track the provided pointer until a connect, accept, or recv event
 	occurs and hand back the pointer in the corresponding callback.  When the
 	callback is	invoked the API stops tracking the pointer (internally it sets
 	its pointers to NULL).  It is thus your	responsibility to handle the pointer
 	-- either reusing it in a subsequent connect(), accept(), or recv() invocation or
 	to delete it.  When a socket is	closed it will return a void pointer; in the
 	event that close() is invoked after a connect, accept, or recv invocation but
 	before such an event occurs then the pointer provided in that function
 	invocation will	be returned, otherwise NULL will be returned.



NOTES TO CONTRIBUTOR:

	OVERVIEW:

	The API was constructed taking advantage of the already existent TCPSocket
	and TCPSocketMap classes.  The TCPSocket's callback interface is based on
	events more than on socket operations, so one thing that this API does is
	to interpret those events back to the appropriate operations (e.g. the
	socketEstablished() callback could be either for a connect or accept operation).
	One operation that the TCPSocket doesn't have is recv, but it does define a
	socketDataArrived() callback.  This API interprets the socketDataArrived event
	as a recv() operation.

	To support the connect(), accept(), and recv() operations, this API uses and
	manages callback state data to track what operation the user has most recently
	requested.  Not all of the enumerated callback states pertain to an operation's
	callback function (e.g. TCPSocketAPI::CB_S_WAIT), rather they indicate whether an operation
	that requires a callback can be validly invoked as well as indicating how an
	event signaled by the TCPSocket's callback interface should be handled.  For
	example recv() cannot be invoked on a socket before connect(), this is enforced
	by not setting the socket's associated callback data state to TCPSocketAPI::CB_S_WAIT until
	the socket is actually connected.  In addition, this API imposes a timed out
	state on sockets using the TCPSocketAPI::CB_S_TIMEOUT value while the underlying TCPSocket's
	actual state is still TCPSocket::CONNECTED.  This is a messy approach conceptually but
	required because the TCPSocket is not a cSimpleModule so it can't schedule
	timeout events.  This API itself could be improved by making a new TCPSocket
	class that would have access to scheduling events through the API and which
	could then be tasked with keeping track of a TIMED_OUT state, provide a recv
	operation, and demultiplex TCP message events directly to opertions.


  	ASSUMPTIONS:

  	- Because the API forces close() to be called before a port can be reused via the
  	bind() function, the close command will reach the TCP core before the new connect
  	or listen command such that the port will successfully be reused.

  	- The TCP core will always (seemingly -- I haven't checked if there are options
  	to control this) accept incoming
  	connections to a listening socket.  However, the socket may not be accepting
  	at the moment.  This API keeps a list of pending connections such that the
  	next time the application invokes accept() a connection from the pending
  	queue will be accepted.  This seems consistent with what the BSD socket API
  	does (it specifies the queue size for pending connections and only returns
  	a connection when accept is called once a connection is established).

	- This API creates a receive buffer (of cPacket not bytes) in the event that
	packets are received when the application has not invoked recv().

  	TO DOCUMENT:

  	TO IMPROVE: (priority levels: low number = high priority, high number = low priority)

  	@todo Level 1 (DONE) remove the hasCallback function from the callback interface, it isn't
  	used and none of the callback functions will be invoked unless the user
  	has previously invoked the corresponding operation

  	@todo Level 2 determine whether pending connections may become invalidated as they wait in
  	the pending queue.  If they do take appropriate action to remove them from
  	the pending connections queue.

  	@todo Level 2 assign ports when not specified in bind or connect since the underlying
  	TCP core only assigns ports within the range 1025-5000

  	@todo Level 3 build a translator that will take in essential message parameters and return
  	an appropriately encapsulated cMessage that can be sent, and then receive a cMessage
  	and extract out the essential parameters

  	@todo Level 4 make a way to set error handling, different error handling options are to
  	throw exceptions (which it does currently) or to return error codes and set an
  	error description string

  	@todo Level 4 make a way to signal to the application that the socket has been closed
  	on a PEER_CLOSED message so that the application will not use that socket
  	anymore?  Alternatively return an error code indicating that the socket is closed when
  	an invalid operation is requested (i.e. recv).  In this case send will still be okay
  	but recv will not be.

  	@todo Level 5 make a new TCPSocket class which has a TIMED_OUT state and has recv, accept,
  	and timeout options, it could be a friend of the API or be given a pointer to the
  	API so that it can schedule events (i.e. timeouts)

  	@todo Level 5 make different send modes, currently the user may call send whenever the
  	underlying TCPSocket is in the TCPSocket::CONNECTED or TCPSocket::CONNECTING or
  	TCPSocket::PEER_CLOSED state, perhaps
  	it would be nice to have a lock step mode such that messages could only be sent when
  	the socket is actually connected

  	@see TCPSocket, TCPSocketMap, IPAddressResolver, SocketTimeoutMsg, TCPCommand
 */
class INET_API TCPSocketAPI : public cSimpleModule, private TCPSocket::CallbackInterface // already noncopyable
{
public:

	/** @name Callback Information */
	//@{

	/** The type of callbacks expected by the socket API. */
	enum CALLBACK_TYPE
	{
		CB_T_CONNECT, /**< Callback to handle socket connection. */
		CB_T_ACCEPT,  /**< Callback to handle socket acceptance. */
		CB_T_RECV     /**< Callback to handle reception on a socket. */
	};

	/** Values that may be returned in callbacks to make error identification more convenient. */
	enum CALLBACK_ERROR
	{
		CB_E_UNKNOWN = -1, /**< An unknown error occurred on socket connection. */
		CB_E_CLOSED = -2,  /**< Socket connection closed. */
		CB_E_TIMEOUT = -3, /**< Socket connection timed out. */
		CB_E_RESET = -4,   /**< Socket connection reset. */
		CB_E_REFUSED = -5  /**< Socket connection refused. */
	};

	/** Defines the functions that must be implemented to make the TCPSocketAPI's callbacks work. */
	class CallbackInterface
	{
	public:

		// Indicates which callback functions are implemented.
		//
		// @return true if the callback type's corresponding function is implemented
		// and false if it is not.
		//virtual bool hasCallback (CALLBACK_TYPE type) =0;

		/** Handles the connection of the specified socket.
		 *
		 * Corresponds to the TCPSocketAPI::CB_T_CONNECT value from the
		 * TCPSocketAPI::CALLBACK_TYPE enumeration.
		 *
		 * Assumes responsibility for yourPtr (either to delete it or reuse it).
		 *
		 * @param socket_id -- The descriptor for the connected socket.
		 * @param ret_status -- The status of the previously invoked connect method.
		 * @param yourPtr -- The pointer to the data/struct/object previously passed
		 *   	to connect.
		 *
		 * On success: ret_status will be 0 (zero).
		 *
		 * On error: ret_status will be a value from the TCPSocketAPI::CALLBACK_ERROR
		 * enumeration.
		 *
		 * @return Nothin.
		 */
		virtual void connectCallback (int socket_id, int ret_status, void * yourPtr) {}

		/** Handles the acceptance of a new socket.
		 *
		 * Corresponds to the TCPSocketAPI::CB_T_ACCEPT value from the
		 * TCPSocketAPI::CALLBACK_TYPE enumeration.
		 *
		 * Assumes responsibility for yourPtr (either to delete it or reuse it).
		 *
		 * @param socket_id -- The descriptor for the listening socket
		 * @param ret_status -- The status of the previously invoked accept method or
		 * 		the descriptor for the accepted socket.
		 * @param yourPtr -- The pointer to the data/struct/object previously passed
		 *   	to accept.
		 *
		 * On success: ret_status will be the descriptor of the accepted socket.
		 *
		 * On error: ret_status will be TCPSocketAPI::CB_E_UNKNOWN from the
		 * TCPSocketAPI::CALLBACK_ERROR enumeration.
		 *
		 * @return Nothing.
		 */
		virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr) {}

		/** Handles the reception of data on the specified socket.
		 *
		 * By default just deletes the received cPacket.
		 *
		 * Corresponds to the TCPSocketAPI::CB_T_RECV value from the
		 * TCPSocketAPI::CALLBACK_TYPE enumeration.
		 *
		 * Assumes responsibility for msg (either to delete it or reuse it).
		 * Assumes responsibility for yourPtr (either to delete it or reuse it).
		 *
		 * @param socket_id -- The descriptor of the receiving socket.
		 * @param ret_status -- The status of the previously invoked recv method or
		 * 		the number of bytes received.
		 * @param msg -- A pointer to the received message.
		 * @param yourPtr -- The pointer to the data/struct/object previously passed
		 * 		to recv.
		 *
		 * On success: msg will point to the received message and ret_status will be
		 * 		the number of bytes in the message.
		 *
		 * On error: msg will point to NULL and ret_status will be a value from the
		 * TCPSocketAPI::CALLBACK_ERROR enumeration.
		 *
		 * @return Nothing.
		 */
		virtual void recvCallback(int socket_id, int ret_status, cPacket * msg,
										void * yourPtr) {delete msg;}

		//virtual void closeNotice (int socket_id, void * yourPtr) {};
	};

	//@}

	/** @name Static public functions */
	//@{

	/**
	 * @return True if the provided value corresponds to one of the
	 * values from the TCPSocketAPI::CALLBACK_ERROR enumeration and false if it does not.
	 */
	static bool isCallbackError(int error);


	/**
	 * @return the name of the error corresponding to the indicated
	 * value from the TCPSocketAPI::CALLBACK_ERROR enumeration; the default is "UNDEFINED".
	 */
	static std::string getCallbackErrorName(int error);

	/**
	 * @return True if the provided value corresponds to one of the
	 * values from the TCPSocketAPI::CALLBACK_TYPE enumeration and false if it does not.
	 */
	static bool isCallbackType(int type);

	/**
	 * @return the name of the type corresponding to the indicated
	 * value from the TCPSocketAPI::CALLBACK_TYPE enumeration; default is "UNDEFINED".
	 */
	static std::string getCallbackTypeName(int type);

	//@}

protected:

	/** @name Instance members */
	//@{

	/** Indicates whether signals to map tcp connections should be emitted. */
	bool _should_map_tcp_connections;

	/** Indicates whether signals to track duplicate message name events should be emitted. */
	bool _should_track_dup_msg_names;

	/** Tracks the current TCPSocket objects. */
	TCPSocketMap _socket_map;

	// Tracks the sockets that come with an TCP_I_ESTABLISHED message but
	// which are not already in the _socket_map and there is no passive
	// socket accepting on their port.  Close is called on the socket and
	// when the close sequence terminates then the socket can be deleted
	// from memory.
	//TCPSocketMap _rejected_sockets_map;

	/** Tracks the timeout messages associated with a given socket.
	 *
	 * Maps socket id to timeout message.
	 */
	std::map<int, SocketTimeoutMsg *> _timeout_timers;

	/** Used to resolve provided string addresses to IPvXAddress objects. */
	IPAddressResolver _resolver;

	/** Tracks the ports that are being used (bound) and which
	 * socket(s) is (are) using it.
	 *
	 * Maps port to socket id.
	 */
	std::map<int, std::set<int> > _bound_ports;

	/** Values to control socket operation sequence, and to indicate
	 * which callback should be invoked and how to interpret events
	 * defined by the TCPSocket::CallbackInterface.
	 */
	enum CALLBACK_STATE
	{
		CB_S_NONE,		/**< No callback requiring operation invoked
							and/or an invalid state. */
		CB_S_CONNECT,	/**< The connect operation was invoked and
							the connectCallback should be called when
							a connection is established or an error occurs. */
		CB_S_ACCEPT,	/**< The accept operation was invoked and the
							acceptCallback should be called when a
							connection is accepted. */
		CB_S_RECV,		/**< The recv operation was invoked and the
							recvCallback should be called when data
							is received on the connection, the
							connection closes or times out or an error occurs. */
		CB_S_CLOSE,		/**< The close operation was invoked so no other
							socket calls are valid and/or the connection
							was closed so only the close operation can
							be invoked. */
		CB_S_TIMEOUT,	/**< The connection is timed out, no socket
							operations are valid except close. */
		CB_S_WAIT		/**< The socket is waiting for an accept or
							recv operation to be invoked and/or a
							connection was accepted or data was
							received but the user didn't reinvoke
							accept or recv. */
	};

	/** Data necessary to execute callbacks associated with a given socket. */
	struct CallbackData
	{
		/** The socket this callback data pertains to and the socket
		 * id to be returned in the callback.
		 */
		int socket_id;

		/** The callback state of the socket.  See TCPSocketAPI::CALLBACK_STATE. */
		CALLBACK_STATE state;

		/** The CallbackInterface assigned to handle events on this socket. */
		CallbackInterface * cbobj;

		/** The data/struct/object the user provides. */
		void * userptr;

		/** The CallbackInterface to be used by accepted connections.
		 * Useful if the user defines different handlers for active
		 * and passive sockets.  May be NULL in which case cbobj
		 * is used.
		 */
		CallbackInterface * cbobj_for_accepted;
	};

	/** Tracks the callback data for passive sockets.
	 *
	 * Note that this separate map is needed because the the _bound_ports
	 * map doesn't have a feature to distinguish passive sockets
	 * from the active sockets that have been accepted on the listening
	 * socket's port.
	 *
	 * Maps port to callback data.
	 */
	std::map<int, CallbackData *> _passive_callbacks;

	/** Tracks the callback data associated with a given socket.
	 *
	 * Maps socket id to callback data.
	 */
	std::map<int, CallbackData *> _registered_callbacks;

	/** Tracks the pending connections awaiting acceptance by passive sockets. */
	TCPSocketMap _pending_sockets_map;

	/** Tracks the pending connections awaiting acceptance by passive sockets.
	 *
	 * Maps passive socket id to id of pending TCPSocket in _pending_sockets_map.
	 */
	std::map<int, std::deque<int> > _pending_connections;

	/** Tracks received data not yet requested by the application.
	 *
	 * Maps active socket id to a list of cPacket pointers.
	 */
	std::map<int, std::deque<cPacket *> > _reception_buffers;

	//@} // End Instance members



	/** @name Static proctected functions */
	//@{

	/** Returns the name of the indicated value from the CALLBACK_STATE enumeration.
	 *
	 * @param state -- The value from the CALLBACK_STATE enumeration
	 * 		whose name is to be returned.
	 *
	 * @return The name of the CALLBACK_STATE value or "unknown" if the state is not
	 * recognized.
	 */
	static std::string getStateName(CALLBACK_STATE state);

	//@}

public:
	/** @name Constructor and Destructor */
	//@{

	TCPSocketAPI ();
	virtual ~TCPSocketAPI ();

	///@}

public:
	/** @name TCP Socket port and address accessors */
	//@{

	/** Accesses the local port number of the indicated socket.
	 *
	 * @param socket_id -- The descriptor to identify the socket in question.
	 *
	 * @return The local port number, -1 if not specified.
	 *
	 * @throw Throws a cRuntimeError if the provided socket id doesn't refer to a current socket.
	 */
	virtual int getLocalPort(int socket_id);

	/** Accesses the remote port number of the indicated socket.
	 *
	 * @param socket_id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote port number, -1 if not specified.
	 *
	 * @throw Throws a cRuntimeError if the provided socket id doesn't refer to a current socket.
	 */
	virtual int getRemotePort(int socket_id);

	/** Accesses the local address of the indicated socket.
	 *
	 * @param socket_id -- The descriptor to identify the socket in question.
	 *
	 * @return The local address.
	 *
	 * @throw Throws a cRuntimeError if the provided socket id doesn't refer to a current socket.
	 */
	virtual IPvXAddress getLocalAddress(int socket_id);

	/** Accesses the remote address of the indicated socket.
	 *
	 * @param socket_id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote address.
	 *
	 * @throw Throws a cRuntimeError if the provided socket id doesn't refer to a current socket.
	 */
	virtual IPvXAddress getRemoteAddres(int socket_id);

	//@}


	/** @name BSD-like API functions */
	//@{

	/** Creates a new socket.
	 *
	 * @param cbobj -- A pointer to an object implementing the callback interface;
	 * 		may not be NULL.
	 *
	 * @return The id/descriptor of the new socket.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual int socket (CallbackInterface * cbobj);

	/** Binds the specified socket to the indicated local address and local port.
	 *
	 * Via this function ports cannot be used concurrently (call close() to free the port),
	 * however, accepting sockets will correctly spin off sockets that are active on the
	 * same port as the listening socket.
	 *
	 * @param socket_id -- The descriptor to identify the socket to be bound.
	 * @param local_address -- The address to bind to, if it is empty (i.e. "") then
	 * 		the socket will be bound to any available IP address.
	 * @param local_port -- The local port to bind to, if it is -1 and local_address
	 * 		is specified then the socket will be bound to any available port.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void bind (int socket_id, std::string local_address, int local_port);

	/** Begins to connect the specified socket to the indicated remote address and remote port.
	 *
	 *  When the connection is established then the connectCallback()
	 *  function will be invoked on the socket's callback object.
	 *
	 *  @param socket_id -- The descriptor to identify the socket to be connected.
	 *  @param remote_address -- The remote address to connect to (module names are
	 *   	allowed as addresses).
	 *  @param remote_port -- The remote port to connect to.
	 *  @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 *  	is NULL.
	 *
	 *  Use of the yourPtr field is discouraged.  In the event that the simulation
	 *  terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 *  cannot be deleted by this API since it is a void pointer; the caller can
	 *  certainly delete it but they must thus store a map of the objects which
	 *  defeats the anticipated purpose of providing this void * parameter.  The
	 *  preferred usage is therefore that the caller does maintain a map of socket
	 *  descriptors to related socket state data and lookup the associated data
	 *  whenever a callback occurs.
	 *
	 *  @return Nothing.
	 *
	 *  @throws throws a cRuntimeError if an error occurs
	 */
	virtual void connect (int socket_id, std::string remote_address, int remote_port,
				void * yourPtr=NULL);

	/** Makes the specified socket a passive socket.
	 *
	 * The socket must be bound to a specific port (call bind() without -1) before
	 * it can be made passive.
	 *
	 * @param socket_id -- The descriptor to identify the socket to be made into a
	 * 		passive (listening) socket.
	 * @param cbobj_for_accepted -- A pointer to an object implementing the
	 * 		CallbackInterface to be used as the callback object for sockets spawned
	 *	 	(accepted) by this listening socket, default is NULL in which case the
	 *	 	spawned sockets will use this socket's callback object.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void listen (int socket_id, CallbackInterface * cbobj_for_accepted=NULL);

	/** Makes an active socket by invoking socket(), bind(), and connect().
	 *
	 * It will skip bind() (i.e. let connect fill in the address) if local_address is
	 * empty AND local_port is negative.
	 *
	 * @param cbobj -- A pointer to an object implementing the callback interface
	 * 		may not be NULL.
	 * @param local_address -- The address to bind to, if it is empty (i.e. "") then
	 * 		will bind to any available IP address.
	 * @param local_port -- The local port to bind to, if it is -1 and local_address
	 *  	is specified then the socket will be bound to any available port.
	 * @param remote_address -- The remote address to connect to (module names are
	 *  	allowed as addresses).
	 * @param remote_port -- The remote port to connect to.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 * @return The descriptor/id of the new socket.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual int makeActiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, std::string remote_address, int remote_port, void * yourPtr=NULL);

	/** Makes a passive socket by invoking socket(), bind(), and listen().
	 *
	 * @param cbobj -- A pointer to an object implementing the callback interface
	 * 		may not be NULL.
	 * @param local_address -- The address to bind to, if it is empty (i.e. "") then
	 * 		will bind to any available IP address.
	 * @param local_port -- The local port to bind to.
	 * @param cbobj_for_accepted -- A pointer to an object implementing the
	 * 		CallbackInterface to be used as the callback object for sockets spawned
	 *  	(accepted) by this listening socket, default is NULL in which case the
	 *  	spawned sockets will use this socket's callback object.
	 *
	 * @return The descriptor/id of the new socket.
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual int makePassiveSocket (CallbackInterface * cbobj, std::string local_address,
			int local_port, CallbackInterface * cbobj_for_accepted=NULL);

	/** Signals to the (passive) socket that it should now accept incoming connections.
	 *
	 * When a connection is actually accepted the acceptCallback() function
	 * will be invoked on the socket's callback object.
	 *
	 * @param socket_id -- The descriptor to identify the (passive) socket that should
	 *  	now accept incoming connections.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 *  Use of the yourPtr field is discouraged.  In the event that the simulation
	 *  terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 *  cannot be deleted by this API since it is a void pointer; the caller can
	 *  certainly delete it but they must thus store a map of the objects which
	 *  defeats the anticipated purpose of providing this void * parameter.  The
	 *  preferred usage is therefore that the caller does maintain a map of socket
	 *  descriptors to related socket state data and lookup the associated data
	 *  whenever a callback occurs.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void accept (int socket_id, void * yourPtr=NULL);

	/** Sends the indicated message on the specified socket.
	 *
	 * Currently the whole message is sent so there isn't a need to check for how
	 * many bytes were sent.  Takes control of the message (see documentation for
	 * cOwnedObject for more details on message taking).
	 *
	 * @param socket_id -- The descriptor to identify the (active) socket to send
	 * 		the message on.
	 * @param msg -- The message to be sent.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void send (int socket_id, cMessage * msg);

	/** Signals to the (active) socket that it should receive incoming data.
	 *
	 * When data is actually received the recvCallback() function will be invoked on the
	 * socket's callback object.
	 *
	 * @param socket_id -- The descriptor to identify the (active) socket that should
	 * 		now receive incoming data.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 *  Use of the yourPtr field is discouraged.  In the event that the simulation
	 *  terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 *  cannot be deleted by this API since it is a void pointer; the caller can
	 *  certainly delete it but they must thus store a map to the data which
	 *  defeats the anticipated purpose of providing this void * parameter.  The
	 *  preferred usage is therefore that the caller does maintain a map of socket
	 *  descriptors to related socket state data and lookup the associated data
	 *  whenever a callback occurs.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void recv (int socket_id, void * yourPtr=NULL);

	/** Sets a timeout on the indicated (active) socket.
	 *
	 * Timeouts are currently only supported on sockets that have been signaled to
	 * receive (see recv()).
	 *
	 * @param socket_id -- The descriptor to identify the (active) socket that should
	 *  	have a timeout set.
	 * @param timeout_interval -- The interval on which the socket should timeout,
	 * 		MUST NOT be negative.
	 *
	 * @return Nothing.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void setTimeout(int socket_id, simtime_t timeout_interval);

	/** Removes the timeout on the indicated (active) socket.
	 *
	 * @param socket_id -- the descriptor to identify the (active) socket that should
	 *  	have its timeout removed.
	 *
	 * @return True if there was a timeout set and it was removed and false otherwise.
	 */
	virtual bool removeTimeout(int socket_id);

	/** Closes the specified socket.
	 *
	 * The socket descriptor is now invalid for all socket operations (including getMyPtr).
	 *
	 * @param socket_id -- The descriptor to identify the socket to be closed.
	 *
	 * @return The data/struct/object provided in a previous accept(), connect(), or recv()
	 * call, or NULL.
	 *
	 * @throws Throws a cRuntimeError if an error occurs.
	 */
	virtual void * close (int socket_id);

	//@}



	/** @name User data access */
	//@{

	/** Returns the pointer to the data/struct/object you specified in a connect(),
	 * listen(), or recv() invocation on the specified socket.  Provided to facilitate
	 * tracking the data you associated with the socket outside callback invocations.
	 *
	 * @param socket_id -- The descriptor to identify the socket whose associated
	 *  	data/struct/object is to be returned.
	 *
	 * Use of the yourPtr field in the accept(), connect(), or recv() functions is
	 * discouraged.  In the event that the simulation terminates abnormally the
	 * allocated data/struct/object pointed to by yourPtr cannot be deleted by this
	 * API since it is a void pointer; the caller can certainly delete it but they
	 * must thus store a map to the data which defeats the anticipated purpose of
	 * providing the void * parameter.  The preferred usage is therefore that the
	 * caller does maintain a map of socket descriptors to related socket state data
	 * and lookup the associated data whenever a callback occurs.
	 *
	 * @return The void pointer to the data previously set in a connect(), accept(), or
	 * recv() call.  Returns NULL if socket_id doesn't pertain to a current socket, if
	 * the void pointer set in a previous connect, accept, or recv invocation
	 * was NULL, or if the data was already returned in a callback invocation.
	 */
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

	/** Handles a timeout.
	 *
	 * NOT part of the TCPSocket::CallbackInteface
	 */
	virtual void socketTimeout(int connId, void * yourPtr);

	/** Cleans up all data associated with a socket */
	virtual void cleanupSocket(int socket_id);

	/** Removes the indicated socket_id from the set of sockets associated with the socket's local port. */
	virtual void freePort(int socket_id);

	/** Finds the socket associated with the indicated id in the socket map
	 * and verifies that it is valid, if it is not it signals a function error
	 * using the indicated function name.
	 */
	virtual TCPSocket * findAndCheckSocket(int socket_id, const std::string & fname);

public:
	/** Returns a string representing the indicated socket as a string.  Because
	 * of the pseudo TIMED_OUT state we don't want the user to worry about this
	 * string representation -- could intercept it and rewrite it.
	 */
	virtual std::string socketToString(int socket_id);

protected:
	/** Makes a new CallbackData object with the indicated values.  cbobj_for_accepted
	 * is always NULL.
	 */
	virtual CallbackData * makeCallbackData(int socket_id, CallbackInterface * cbobj,
			void * userptr, CALLBACK_STATE type);

	/** Looks up the passive callback data associated with the indicated port.
	 *
	 * @return the callback data if it exists and NULL if there is no passive
	 * socket on the indicated port, or if the passive socket is not in the accept
	 * state
	 */
	virtual CallbackData * getAcceptCallback(int port);

	/** Looks up the passive callback data associated with the indicated port.
	 *
	 * @return the callback data if it exists and NULL if there is no passive
	 * socket on the indicated port.
	 */
	virtual CallbackData * getPassiveCallback(int port);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalFunctionError(const std::string & fname, const std::string & details);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBNullError(const std::string & fname);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBStateReceptionError(const std::string & fname, CALLBACK_STATE state);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBStateInconsistentError(const std::string & fname, CALLBACK_STATE state);

	/** Prints a notice on the simulation environment output. */
	virtual void printFunctionNotice(const std::string & fname, const std::string & notice);

	/** Prints a notice on the simulation environment output. */
	virtual void printCBStateReceptionNotice(const std::string & fname, CALLBACK_STATE state);

	/** Emits tcp connection information from the given socket. */
	virtual void emitTCPConnInfo(TCPSocket * socket);

	/** Emits message event information given the socket descriptor and message. */
	virtual void emitMessageEvent(const cMessage * msg, int interface_id);

	//@}
};

#endif
