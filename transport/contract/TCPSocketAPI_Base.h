/**
 * \file TCPSocketAPI_Base.h
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

#ifndef _TCPSOCKETAPI_BASE_H_
#define _TCPSOCKETAPI_BASE_H_

// Do not include the omnetpp.h because this header file needs to be independent
// of omnet stuff so as to be compatible with the emulation framework.

// From standard C++ libraries
#include <string>

// #defines
#ifndef NULL
#define NULL 0
#endif

// Type defs
typedef int 					socket_id_t;
typedef int 					cb_status_t;
typedef int 					port_t;
typedef std::string 			address_t;
typedef const std::string &		address_cref_t;
typedef int						error_id_t;
typedef void *					user_data_ptr_t;

typedef const std::string &		str_cref_t;

// Class documentation at bottom of file.
class TCPSocketAPI_Base
{

public:

	/** @name Callback Information */
	//@{

	/** Values that may be returned in callbacks to make error identification more convenient. */
	enum CallbackError
	{
		CB_E_UNKNOWN = -1, /**< An unknown error occurred on socket. */
		CB_E_CLOSED  = -2, /**< Socket closed. */
		CB_E_TIMEOUT = -3, /**< Socket timed out. */
		CB_E_RESET   = -4, /**< Socket reset. */
		CB_E_REFUSED = -5  /**< Socket refused. */
	};


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

	/**
	 * Defines the functions that can be extended to handle the connect or accept
	 * operations specific to a given application.  Subclasses should define a 
	 * recvCallback function specific to a desired application data type with 
	 * socket id, return status, application data pointer, and void element pointer
	 * parameters.
	 */
	class CallbackHandler
	{
	public:

		/** 
		 * By default does nothing.  Subclasses should handle the connection of the
		 * specified socket specific to their application.  Subclasses should also
		 * assume responsibility for the void pointer if it is non-NULL (e.g. reuse
		 * it or deallocate it)
		 *
		 * @param id -- The descriptor for the connected socket.
		 * @param return_value -- The return value of the previously invoked connect()
		 * 		function.
		 * @param yourPtr -- The pointer to the element previously passed	to connect().
		 *
		 * On success: return_value will be 0 (zero).
		 *
		 * On error: return_value will be a value from the TCPSocketAPI_Base::CallbackError
		 * enumeration.
		 */
		virtual void connectCallback (socket_id_t id,
					cb_status_t return_value,
					user_data_ptr_t yourPtr) {}


		/**
		 * By default does nothing.  Subclasses should handles the acceptance of a
		 * new socket specific to their application.  Subclass should also assume
		 * responsibility for the void pointer if it is non-NULL (e.g. reuse it or
		 * deallocate it).
		 *
		 * @param id -- The descriptor for the listening socket
		 * @param return_value -- The return value of the previously invoked accept()
		 * 		method or the descriptor for the accepted socket.
		 * @param yourPtr -- The pointer to the data/struct/object previously passed
		 *   	to accept.
		 *
		 * On success: return_value will be the descriptor of the accepted socket.
		 *
		 * On error: return_value will be a value from the TCPSocketAPI_Base::CallbackError
		 * enumeration.
		 */
		virtual void acceptCallback  (socket_id_t id,
					cb_status_t return_value,
					user_data_ptr_t yourPtr) {}
	};

	typedef TCPSocketAPI_Base::CallbackHandler * cb_base_handler_ptr_t;

	//@}

	/** @name Virtual Destructor */
	//@{

	/** Empty virtual destructor. */
	virtual ~TCPSocketAPI_Base () {}

	///@}



	/** @name TCP Socket port and address accessors */
	//@{

	/** Accesses the local port number of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The local port number, -1 if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual port_t getLocalPort (socket_id_t id) =0;


	/** Accesses the remote port number of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote port number, -1 if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual port_t getRemotePort (socket_id_t id) =0;


	/** Accesses the local address of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The local address, empty ("") if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual address_t getLocalAddress (socket_id_t id) =0;


	/** Accesses the remote address of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote address, empty ("") if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual address_t getRemoteAddres (socket_id_t id) =0;

	//@}



	/** @name Base BSD-like API functions */
	//@{

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
	virtual socket_id_t socket (cb_base_handler_ptr_t cbobj) =0;


	/**
	 * Binds the specified socket to the indicated local address and local port.
	 *
	 * Via this function ports cannot be used concurrently (call close() to free the port),
	 * however, accepting sockets will correctly spin off sockets that are active on the
	 * same port as the listening socket.
	 *
	 * @param id -- The descriptor to identify the socket to be bound.
	 * @param local_address -- The address to bind to; if it is empty (i.e. "") and
	 * 		the local port paremeter is not -1 then the socket will be bound to any	
	 * 		available IP address.
	 * @param local_port -- The local port to bind to; if it is -1 and the local_address
	 * 		parameter is a valid address then the socket will be bound to any available
	 *			port.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 *
	 * Note: When a socket is first created (via socket()) its local address is empty
	 * and its local port is -1.  The bind function should only be used if at least
	 * one of the local parameters should have a value other than the default.  Thus
	 * if bind is called with an empty local address and a local port or -1 an error
	 * will be thrown.
	 */
	virtual void bind (socket_id_t id,
			address_cref_t local_address,
			port_t local_port) =0;


	/**
	 * Begins to connect the specified socket to the indicated remote address and
	 * remote port.
	 *
	 * When the connection is established then the connectCallback()
	 * function will be invoked on the socket's callback object.
	 *
	 * @param id -- The descriptor to identify the socket to be connected.
	 * @param remote_address -- The remote address to connect to (module names are
	 * 		allowed as addresses).
	 * @param remote_port -- The remote port to connect to.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 * Use of the yourPtr field is discouraged.  In the event that the simulation
	 * terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 * cannot be deleted by this API since it is a void pointer; the caller can
	 * certainly delete it but they must thus store a map of the objects which
	 * defeats the anticipated purpose of providing this void * parameter.  The
	 * preferred usage is therefore that the caller does maintain a map of socket
	 * descriptors to related socket state data and lookup the associated data
	 * whenever a callback occurs.
	 *
	 *  @throws Throws a std::exception if an error occurs
	 */
	virtual void connect (socket_id_t id,
				address_cref_t remote_address,
				port_t remote_port,
				user_data_ptr_t yourPtr=NULL) =0;


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
				cb_base_handler_ptr_t cbobj_for_accepted=NULL) =0;


	/**
	 * Makes an active socket by invoking socket(), bind(), and connect().
	 *
	 * It will skip bind() (i.e. let connect fill in the address) if local_address is
	 * empty and local_port is negative.
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket; must not be
	 * 		NULL.
	 * @param local_address -- The address to bind to, if it is empty (i.e. "") then
	 * 		will bind to any available IP address.
	 * @param local_port -- The local port to bind to, if it is -1 and local_address
	 *  		is specified then the socket will be bound to any available port.
	 * @param remote_address -- The remote address to connect to (module names are
	 *  		allowed as addresses).
	 * @param remote_port -- The remote port to connect to.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 * @return The descriptor/id of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t makeActiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address,
			port_t local_port,
			address_cref_t remote_address,
			port_t remote_port,
			user_data_ptr_t yourPtr=NULL) =0;


	/**
	 * Makes a passive socket by invoking socket(), bind(), and listen().
	 *
	 * @param cbobj -- A pointer to an object implementing the callback interface
	 * 		may not be NULL.
	 * @param local_address -- The address to bind to, if it is empty (i.e. "") then
	 * 		will bind to any available IP address.
	 * @param local_port -- The local port to bind to.
	 * @param cbobj_for_accepted -- A pointer to an object implementing the
	 * 		CallbackHandler to be used as the callback object for sockets spawned
	 *  		(accepted) by this listening socket; default is NULL in which case the
	 *  		spawned sockets will use the lisenting socket's callback handler.
	 *
	 * @return The descriptor/id of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t makePassiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address,
			port_t local_port,
			cb_base_handler_ptr_t cbobj_for_accepted=NULL) =0;


	/**
	 * Signals to the (passive) socket that it should now accept incoming connections.
	 *
	 * When a connection is actually accepted the acceptCallback() function
	 * will be invoked on the socket's callback object.
	 *
	 * @param id -- The descriptor to identify the (passive) socket that should
	 *  		now accept incoming connections.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 * Use of the yourPtr field is discouraged.  In the event that the simulation
	 * terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 * cannot be deleted by this API since it is a void pointer; the caller can
	 * certainly delete it but they must thus store a map of the objects which
	 * defeats the anticipated purpose of providing this void * parameter.  The
	 * preferred usage is therefore that the caller does maintain a map of socket
	 * descriptors to related socket state data and lookup the associated data
	 * whenever a callback occurs.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void accept (socket_id_t id,
			user_data_ptr_t yourPtr=NULL) =0;


	/**
	 * Signals to the (active) socket that it should receive incoming data.
	 *
	 * When data is actually received the recvCallback() function will be invoked on the
	 * socket's callback object.
	 *
	 * @param id -- The descriptor to identify the (active) socket that should
	 * 		now receive incoming data.
	 * @param yourPtr -- A pointer to whatever data/struct/object you want, default
	 * 		is NULL.
	 *
	 * Use of the yourPtr field is discouraged.  In the event that the simulation
	 * terminates abnormally the allocated data/struct/object pointed to by yourPtr
	 * cannot be deleted by this API since it is a void pointer; the caller can
	 * certainly delete it but they must thus store a map to the data which
	 * defeats the anticipated purpose of providing this void * parameter.  The
	 * preferred usage is therefore that the caller does maintain a map of socket
	 * descriptors to related socket state data and lookup the associated data
	 * whenever a callback occurs.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void recv (socket_id_t id, user_data_ptr_t yourPtr=NULL) =0;


	/**
	 * Removes the timeout on the indicated (active) socket.
	 *
	 * @param id -- the descriptor to identify the (active) socket that should
	 *  		have its timeout removed.
	 *
	 * @return True if there was a timeout set on the socket and it was removed; false
	 * otherwise.
	 */
	virtual bool removeTimeout (socket_id_t id) =0;

	/**
	 * Closes the specified socket.
	 *
	 * The socket descriptor is now invalid for all socket operations (including getMyPtr).
	 *
	 * @param id -- The descriptor to identify the socket to be closed.
	 *
	 * @return The data/struct/object provided in a previous accept(), connect(), or recv()
	 * call, or NULL.
	 *
	 * @throws Throws a std::excpetion if an error occurs.
	 */
	virtual user_data_ptr_t close (socket_id_t id) =0;

	//@}


	/** @name User data access */
	//@{

	/** Returns the pointer to the data/struct/object you specified in a connect(),
	 * listen(), or recv() invocation on the specified socket.  Provided to facilitate
	 * tracking the data you associated with the socket outside callback invocations.
	 *
	 * @param id -- The descriptor to identify the socket whose associated
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
	virtual user_data_ptr_t getMyPtr (socket_id_t id) =0;

	//@}
};

// After the fact type def
typedef TCPSocketAPI_Base::cb_base_handler_ptr_t cb_base_handler_ptr_t;

/** \class TCPSocketAPI_Base
	OVERVIEW:
 
	Defines a BSD like API for doing network communications using TCP.  This
	base API defines functions such as socket, bind, listen, accept, connect,
	and close.  Extensions of this interface should define send and recv for
	their specific application message formats (e.g. TCPSocketAPI_cPacket
	deals in cPackets; TCPSocketAPI_MetaHTTP deals in MetaHTTP structs; and
	so on).  As in the BSD API a socket descriptor is returned from the
	function socket() which can thereafter be used to perform other
	operations on that socket.  Convenience functions to make active or
	passive socket are also defined.

	TCP SOCKET CALLBACK HANDLER API:

	The major difference from the BSD API is that this API is intended to be
	completely asynchronous so as to fit into the asynchronous architecture of
	OMNET++.  Traditionally the functions connect(), accept(), and recv() are
	blocking system calls, meaning that they do not return until a socket is
	connected, accepted, or data has been received over the socket respectively.
	To make these operations work in an asynchronous environment a callback
	must be used.

 	The callback interface specified by this base API defines a corresponding
 	callback function for the operations connect and accept.  Extensions of
	this interface should define a callback function for the recv operation.
	When a socket is first created (via socket()) a pointer to a callback
	handler must be provided, specifically a handler that extends the internal
	CallbackHandler class of the API extension that is being used (e.g. a
	TCPSocketAPI_cPacket::CallbackHandler is required when using the
	TCPSocketAPI_cPacket API extension).

	After creating a socket as explained, a connect(), accept(), or recv() call
	only informs the API that such an operation should be simulated.  After
	calling connect(), accept(), or recv() the calling code should surrender
	control back to the simulator (i.e. return) so that the requested operation
	can be simulated.  When the operation finishes simulation the corresponding
	callback will be invoked, namely: the connectCallback() is invoked when a
	a socket is connected or fails to connect; the acceptCallback() is invoked 
	when a new connection is accepted on a listening socket; the recvCallback()
	is invoked when data is received over a connection or the connection closes
	or the the connection times out.

 	The implementation of the callback should handle the event and then invoke
 	further appropriate socket operations.  The functions accept() and recv()
	are not persistent, meaning that they may not be invoked once and thereafter
	be expected to trigger their corresponding callback whenever an accept or
	receive event occurs.  Generally, if repeated accept or recv operations are
	required they should be reinvoked in the acceptCallback() or recvCallback()
	implementation respectively; in a synchronous environment this is in	essence
	what a loop in an accepting or receiving thread does.

	When connect(), accept(), or recv() are invoked a pointer to any
	data/struct/object can be provided (cast it as a void pointer).  The API will
	not modify the element pointed to.  This pointer will be returned in the
	operation's corresponding callback when the operation finishes simulation.
	This feature was provided with the intent that context associated with a
	specific socket could be maintained via this API; however, use of this 
	feature is now discouraged since the API cannot deallocate the element in the
	event that the simulation terminates abnormally, yet the using application
	can deallocate the element if it stores the context information.

	EXAMPLE:

	In a synchronous environment a common sequence of socket operations is as
	follows (without specific regard to the BSD API; comments are between '#'
	symbols):

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

	If a non-NULL element pointer is provided to a connect(), accept(), or
	recv() invocation (this use is discouraged) the application should be
	aware of the following memory management issues:

 	The API never assumes responsibility for deallocating from heap space any
 	non-NULL element pointer provided in a connect(), accept(), or recv()
	invocation.  The API will, however, track the provided pointer until a
	connect, accept, or recv operation finishes simulation and pass back the
	pointer in the corresponding callback.  When this occurs the API stops
	tracking the element pointer (internally it sets	its pointers to NULL).
	The callback implementation should therefore handle the element pointer--
	either reusing it in a subsequent connect, accept, or recv invocation, or
	deleting	it, etc.  When a socket is closed it will return a void pointer;
	in the event that close() is invoked after a connect, accept, or recv
	invocation but before the operation finishes simulation then the pointer
	provided in the invocation will be returned.

  	TO IMPROVE: (priority levels: low number = high priority, high number = low priority)

  	@todo Level 1 make a way to set error handling, different error handling options are to
  	throw exceptions (which it does currently) or to return error codes and set an
  	error description string

  	@todo Level 1 make a way to signal to the application that the socket has been closed
  	on a PEER_CLOSED message so that the application will not use that socket
  	anymore?  Alternatively return an error code indicating that the socket is closed when
  	an invalid operation is requested (i.e. recv).  In this case send will still be okay
  	but recv will not be.

	SUBCLASSING:
	
	This base API is incomplete.  Sub classes should provide send and setTimeout functions
	specific to the application data type and time type desired.  Subclasses should also
	define an extension of the CallbackHandler class which as a recvCallback function
	specific to the desired application data type.
 */

#endif /* _TCPSOCKETAPI_BASE_H_ */
