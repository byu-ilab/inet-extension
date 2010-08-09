//==========================================================================80>
/**
 * @file TCPSocketAPI_Base.h
 *
 * TCPSocketAPI_Base interface and TCPSocketAPI_Base::CallbackHandler class
 * declarations.
 *
 * Created: August 2, 2010
 *
 * @todo Add GPL notice.
 */

/* potential documentation template
 * @author Kevin Black
 * @par Created:
 * August 2, 2010
 *
 * @attention
 * @par
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * @par
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * @par
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef _TCPSOCKETAPI_BASE_H_
#define _TCPSOCKETAPI_BASE_H_

// Do not include the omnetpp.h because this header file needs to be independent
// of omnet stuff so as to be compatible with the emulation framework.

// From inet
#include "InternetTypeDefsWithStandardTypes.h"

// From standard C++ libraries
#include <string>

// #defines
#ifndef NULL
#define NULL 0
#endif

// Type definitions
// TODO move to namespace or internal to the class?
typedef int 	cb_status_t;
typedef int		error_id_t;
typedef void *	user_data_ptr_t;

// Class documentation at bottom of file.
class TCPSocketAPI_Base
{

public:

	/** @name Callback Information */
	//@{

	/**
	 * Values that may be returned in callbacks to make error
	 * identification more convenient.
	 */
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
	 * values from the TCPSocketAPI_Base::CallbackError enumeration,
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
	 * Defines functions that should be extended to handle connect or
	 * accept operations.
	 *
	 * @details Extensions to this socket API should define a function to
	 * handle the recv operation specific to the application data type with
	 * the following prototype:
	 * @code
	 * virtual void recvCallback(socket_id_t id, cb_status_t status,
	 *                           app_msg_ptr_t msg, user_data_ptr_t context);
	 * @endcode
	 */
	class CallbackHandler
	{
	public:

		/** Empty. */
		virtual ~CallbackHandler() {}

		/** 
		 * By default does nothing.
		 *
		 * @details Subclasses should handle the connection of the specified
		 * socket specific to their application.  Subclasses should also
		 * assume responsibility for the user data pointer if it is non-NULL (e.g.
		 * reuse it or deallocate it)
		 *
		 * @param id -- Descriptor of the connected socket.
		 * @param result -- Result of the previous connect() invocation.
		 * @param context -- User data element pointer provided in previous
		 * 		connect() invocation.
		 *
		 * On connection success: @em result will be 0 (zero).
		 *
		 * On connection error: @em result will be a value from the enumeration
		 * TCPSocketAPI_Base::CallbackError.
		 */
		virtual void connectCallback (socket_id_t id, cb_status_t result,
				user_data_ptr_t context) {}


		/**
		 * By default does nothing.
		 *
		 * @details Subclasses should handles the acceptance of a new socket
		 * specific to their application.  Subclasses should also assume
		 * responsibility for the user data pointer if it is non-NULL (e.g. reuse
		 * it or deallocate it).
		 *
		 * @param id -- Descriptor of the listening socket
		 * @param result -- Result of previous accept() invocation.
		 * @param context -- User data element pointer provided in previous
		 * 		accept() invocation.
		 *
		 * On acceptance success: @em result will be the descriptor of the
		 * accepted socket.
		 *
		 * On acceptance error: @em result will be a value from the
		 * enumeration TCPSocketAPI_Base::CallbackError.
		 */
		virtual void acceptCallback  (socket_id_t id, cb_status_t result,
				user_data_ptr_t context) {}
	};

	/**
	 * Callback handler pointer type specific to TCPSocketAPI_Base.
	 */
	typedef TCPSocketAPI_Base::CallbackHandler * cb_base_handler_ptr_t;

	//@}



	/** @name Destructor */
	//@{

	/** Empty. */
	virtual ~TCPSocketAPI_Base () {}

	///@}



	/** @name TCP Socket Port and Address Accessors */
	//@{

	/**
	 * Accesses the local port of the indicated socket.
	 *
	 * @param id -- Socket descriptor.
	 *
	 * @return The local port number; PORT_NULL if the local port is not set.
	 *
	 * @exception Throws a std::exception if the provided socket descriptor
	 * doesn't refer to a current socket.
	 */
	virtual port_t getLocalPort (socket_id_t id) =0;


	/**
	 * Accesses the remote port of the indicated socket.
	 *
	 * @param id -- Socket descriptor.
	 *
	 * @return The remote port number; PORT_NULL if the remote port is not set.
	 *
	 * @exception Throws a std::exception if the provided socket descriptor
	 * doesn't refer to a current socket.
	 */
	virtual port_t getRemotePort (socket_id_t id) =0;


	/**
	 * Accesses the local address of the indicated socket.
	 *
	 * @param id -- Socket descriptor.
	 *
	 * @return The local IP address; ADDRESS_NULL if not set.
	 *
	 * @exception Throws a std::exception if the provided socket descriptor
	 * doesn't refer to a current socket.
	 */
	virtual address_t getLocalAddress (socket_id_t id) =0;


	/**
	 * Accesses the remote address of the indicated socket.
	 *
	 * @param id -- Socket descriptor.
	 *
	 * @return The remote IP address; ADDRESS_NULL if not set.
	 *
	 * @throw Throws a std::exception if the provided socket descriptor
	 * doesn't refer to a current socket.
	 */
	virtual address_t getRemoteAddres (socket_id_t id) =0;

	//@}



	/** @name BSD-like API Operations */
	//@{

	/**
	 * Creates a new socket.
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket.
	 *
	 * The @em cbobj pointer must not be NULL.
	 *
	 * @return The descriptor of the new socket.
	 *
	 * @exception Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t socket (cb_base_handler_ptr_t cbobj) =0;


	/**
	 * Binds the specified socket to the indicated local address and
	 * local port.
	 *
	 * Via this function ports cannot be used concurrently (call close() to
	 * free the port); however, listening sockets will spawn off active
	 * sockets on the same port as itself.
	 *
	 * @param id -- Socket descriptor of the socket to be bound.
	 * @param local_address -- Address to bind to.
	 * @param local_port -- Port to bind to.
	 *
	 * When a socket is first created (via socket()) its local address is
	 * ADDRESS_NULL and its local port is PORT_NULL.  The bind function
	 * should only be used if at least one of the local parameters should have
	 * a value other than the default.
	 *
	 * For @em local_port to be valid it must be between TCP_PORT_RANGE_MIN and
	 * TCP_PORT_RANGE_MAX inclusively.
	 *
	 * If @em local_address is ADDRESS_NULL and @em local_port is valid,
	 * then the socket will be bound to any available IP address.
	 *
	 * If @em local_port is PORT_NULL and @em local_address is valid then the
	 * socket will be bound to any available port.
	 *
	 * @exception Throws a std::exception if an error occurs, or @em
	 * local_address is ADDRESS_NULL and @em local_port is PORT_NULL.
	 */
	virtual void bind (socket_id_t id, address_cref_t local_address,
			port_t local_port) =0;

	/**
	 * Begins to connect the specified socket to the indicated remote
	 * address and remote port.
	 *
	 * When the connection is established then the connectCallback()
	 * function will be invoked on the socket's callback handler.
	 *
	 * @param id -- Socket descriptor of the socket to be connected.
	 * @param remote_address -- Address to connect to.
	 * @param remote_port -- Port to connect to.
	 *
	 * Both @em remote_address and @em remote_port must be valid.
	 *
	 * @throws Throws a std::exception if an error occurs
	 */
	virtual void connect (socket_id_t id, address_cref_t remote_address,
			port_t remote_port) =0;


	/**
	 * Begins to connect the specified socket to the indicated remote
	 * address and remote port.
	 *
	 * Same as TCPSocketAPI_Base::connect(socket_id_t, address_cref_t, port_t)
	 * except that it allows a user to provide a context element pointer.
	 *
	 * @param context -- User data element pointer to maintain context.
	 *
	 * @deprecated Use of the @em context field is discouraged.  In the event
	 * that the simulation terminates abnormally the element pointed to by @em
	 * context cannot be deleted by this API since the underlying type is a
	 * void pointer.  However, the user application can delete it if it stores
	 * the context elements and associates them with the sockets it creates.
	 * That is the preferred practice.
	 */
	virtual void connect (socket_id_t id, address_cref_t remote_address,
			port_t remote_port, user_data_ptr_t context) =0;


	/**
	 * Makes the specified socket a passive (listening) socket.
	 *
	 * The socket must be bound to a valid port (see bind()) before it can be
	 * made passive.
	 *
	 * @param id -- Socket descriptor of the socket to be made passive.
	 * @param cbobj_for_accepted -- A pointer to the callback handler for the
	 * 		sockets spawned (accepted) by the listening socket.
	 *
	 * If @em cbobj_for_accepted is NULL (the default) then the callback
	 * handler for the passive socket will be used as the handler for the
	 * spawned (i.e. accepted) sockets as well.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void listen (socket_id_t id,
				cb_base_handler_ptr_t cbobj_for_accepted=NULL) =0;


	/**
	 * Makes an active socket by invoking socket(), bind(), and
	 * connect().
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket.
	 * @param local_address -- Address to bind to.
	 * @param local_port -- Port to bind to.
	 * @param remote_address -- Address to connect to.
	 * @param remote_port -- Port to connect to.
	 *
	 * Skips bind() (i.e. lets connect() fill in the local address and port) if
	 * @em local_address is ADDRESS_NULL and @em local_port is PORT_NULL.
	 *
	 * @return The descriptor of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 *
	 * @see socket(), bind(), and connect().
	 */
	virtual socket_id_t makeActiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address,  port_t local_port,
			address_cref_t remote_address, port_t remote_port);


	/**
	 * Makes an active socket by invoking socket(), bind(), and
	 * connect().
	 *
	 * Same as TCPSocketAPI_Base::makeActiveSocket(cb_base_handler_ptr_t,
	 * address_cref_t,port_t,address_cref_t,port_t) except that it allows a
	 * user to provide a context element pointer.
	 *
	 * @param context -- User data element pointer to maintain context.
	 *
	 * @deprecated See connect(socket_id_t,address_cref_t,port_t,
	 * user_data_ptr_t) for reason.
	 *
	 * @see socket(), bind(), and connect().
	 */
	virtual socket_id_t makeActiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address,  port_t local_port,
			address_cref_t remote_address, port_t remote_port,
			user_data_ptr_t context) =0;


	/**
	 * Makes a passive socket by invoking socket(), bind(), and
	 * listen().
	 *
	 * @param cbobj -- A pointer to the callback handler for the socket.
	 * @param local_address -- Address to bind to.
	 * @param local_port -- Port to bind to.
	 * @param cbobj_for_accepted -- A pointer to the callback handler for the
	 * 		sockets spawned (accepted) by the listening socket.
	 *
	 * If @em cbobj_for_accepted is NULL (the default) then the callback
	 * handler for the passive socket will be used as the handler for the
	 * spawned (i.e. accepted) sockets as well.
	 *
	 * @return The descriptor of the new socket.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual socket_id_t makePassiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address, port_t local_port,
			cb_base_handler_ptr_t cbobj_for_accepted=NULL) =0;


	/**
	 * Signals to the (passive) socket that it should accept an incoming
	 * connection (or notify the callback handler of a buffered accepted
	 * connection).
	 *
	 * When a connection is actually accepted the acceptCallback() function
	 * will be invoked on the socket's associated callback handler.
	 *
	 * @param id -- Socket descriptor of the (passive) socket that should
	 * 		accept an incoming connection.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void accept (socket_id_t id) =0;


	/**
	 * Signals to the (passive) socket that it should accept an incoming
	 * connection (or notify the callback handler of a buffered accepted
	 * connection).
	 *
	 * Same as TCPSocketAPI_Base::accept(socket_id_t) except that it allows a
	 * user to provide a context element pointer.
	 *
	 * @param context -- User data element pointer to maintain context.
	 *
	 * @deprecated Use of the @em context field is discouraged.  In the event that the
	 * simulation terminates abnormally the element pointed to by @em context
	 * cannot be deleted by this API since the underlying type is a void
	 * pointer.  However, the user application can delete it if it stores
	 * the context elements and associates them with the sockets it creates.
	 * That is the preferred practice.
	 */
	virtual void accept (socket_id_t id, user_data_ptr_t context) =0;


	/**
	 * Special receive modes for the recv() operation.
	 */
	enum RecvMode
	{
		/**
		 * Indicates that incoming data should be buffered and only returned
		 * once all of the bytes pertaining to a logical application message
		 * have been received.  Only useful in a scenario where the TCP core
		 * passes up byte buffer packets instead of buffering the whole
		 * message before passing up packets.
		 */
		RECV_MODE_PACKET = 0,

		/**
		 * Indicates that incoming data should not be buffered and rather
		 * immediately passed back in the recvCallback() function.  Acts as
		 * RECV_MODE_PACKET does if the TCP core buffers packets until the
		 * whole logical application message has been received.
		 */
		RECV_MODE_NO_BUFFER = -1
	};


	/**
	 * Signals to the (active) socket that it should receive incoming
	 * data.
	 *
	 * When data is actually received the recvCallback() function will be
	 * invoked on the socket's associated callback handler.
	 *
	 * @param id -- Socket descriptor of the (active) socket that should
	 * 		receive incoming data.
	 * @param byte_mode -- Indicates how incoming data should be handled and
	 * 		passed back in the recvCallback().
	 *
	 * If @em byte_mode is a positive value then incoming data is buffered
	 * and up to @em byte_mode bytes are passed back in the recvCallback()
	 * when ready.  The byte range containing a logical application's first
	 * byte will pass back the actual logical application message.
	 * Subsequent byte ranges associated with a logical application message
	 * will be passed back with a NULL message pointer.
	 *
	 * If @em byte_mode is RECV_MODE_PACKET (the default) then incoming data
	 * is buffered until all of the bytes of logical application message have
	 * been received, where after the logical application message is passed
	 * back.  See RECV_MODE_PACKET for further notes.
	 *
	 * If @em byte_mode is RECV_MODE_NO_BUFFER then incoming data is passed
	 * back as soon as it is received.  See RECV_MODE_NO_BUFFER for further
	 * notes.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void recv (socket_id_t id, bytecount_t byte_mode=RECV_MODE_PACKET) =0;


	/**
	 * Signals to the (active) socket that it should receive incoming
	 * data.
	 *
	 * Same as recv(socket_id_t,bytecount_t) except that it allows the user
	 * to provide a context element pointer.
	 *
	 * @param context -- User data element pointer to maintain context.
	 *
	 * @deprecated Use of the @em context field is discouraged.  In the event
	 * that the simulation terminates abnormally the element pointed to by @em
	 * context cannot be deleted by this API since the underlying type is a
	 * void pointer.  However, the user application can delete it if it stores
	 * the context elements and associates them with the sockets it creates.
	 * That is the preferred practice.
	 */
	virtual void recv (socket_id_t id, bytecount_t byte_mode,
			user_data_ptr_t context) =0;


	/**
	 * Removes the timeout on the indicated (active) socket.
	 *
	 * @param id -- Socket descriptor of the socket to have it timeout removed.
	 *
	 * @return True if there was a timeout set on the socket and it was
	 * removed; false otherwise.
	 */
	virtual bool removeTimeout (socket_id_t id) =0;


	/**
	 * Closes the specified socket.
	 *
	 * The socket descriptor is now invalid for all socket operations (including
	 * getMyPtr).
	 *
	 * @param id -- Socket descriptor of the socket to be closed.
	 *
	 * @return The context element pointer provided in a previous accept(),
	 * connect(), or recv() invocation if the corresponding callback has not
	 * been called, or NULL.
	 *
	 * @throws Throws a std::excpetion if an error occurs.
	 */
	virtual user_data_ptr_t close (socket_id_t id) =0;

	//@}


	/** @name User data access */
	//@{

	/**
	 * Returns the pointer to the context element pointer provided in a
	 * connect(), accept(), or recv() invocation on the indicated socket.
	 * Provided to facilitate tracking the context data you associated with the
	 * socket.
	 *
	 * @param id -- Socket descriptor of the socket whose associated context
	 * 		element pointer is to be returned.
	 *
	 * @deprecated Use of the @em context parameter in the accept(), connect(),
	 * or recv() functions is discouraged.  In the event that the simulation
	 * terminates abnormally the element pointed to by @em context cannot be
	 * deleted by this API since the underlying type is a void pointer.
	 * However, the user application can delete it if it stores the context
	 * elements and associates them with the sockets it creates.  That is the
	 * preferred practice.
	 *
	 * @return The context element pointer associated with the socket; NULL
	 * if @em socket_id doesn't to a current socket, if no context element
	 * pointer was provided previously, or if the context element pointer
	 * was previously returned in a callback.
	 *
	 * @todo Refactor to getUserContext
	 */
	virtual user_data_ptr_t getMyPtr (socket_id_t id) =0;

	//@}
};

// After the fact type def
typedef TCPSocketAPI_Base::cb_base_handler_ptr_t cb_base_handler_ptr_t;

/**
 * @class TCPSocketAPI_Base
 *
 * Defines a BSD-like socket API for network communications using TCP
 * sockets.
 *
 * @details
 *
 * @par Overview:
 * This base API defines functions such as socket, bind, listen, accept,
 * connect, recv, and close.  Extensions of this API are expected to define
 * a send function specific to the desired application message type (e.g.
 * TCPSocketAPI_Inet deals in cPacket objects; TCPSocketAPI_Emulation deals
 * in MetaAppMsg structs)  As in the BSD API, a socket descriptor is
 * returned from the function socket() which is thereafter used to perform
 * other operations on that socket.  Convenience functions to make active
 * or passive sockets are also defined.
 *
 * @par	TCP Socket Callback Handling:
 * The major difference from the BSD API is that this API is intended to be
 * completely asynchronous so as to fit into the asynchronous architecture of
 * OMNET++.  Traditionally the functions connect(), accept(), and recv() are
 * blocking system calls, meaning that they do not return until a socket is
 * connected, accepted, or data has been received over the socket respectively.
 * To make these operations work in an asynchronous environment a callback
 * must be used.
 *
 * @par
 * The callback handler specified by this base API defines a corresponding
 * callback function for the connect and accept operations.  Extensions of
 * this API should also extend the callback handler to define a callback
 * function for the recv operation specific to desired application message
 * type.  Applications that use the API should extend the callback handler
 * (or have an aggregate delegate handler) so they can appropriately process
 * the socket event.  When a socket is first created (via socket()) a pointer
 * to a callback handler must be provided, specifically a handler that extends
 * the callback handler class specific to the API extension (e.g. a
 * TCPSocketAPI_Inet::CallbackHandler is required when using the
 * TCPSocketAPI_Inet API extension).
 *
 * @par
 * After creating a socket as explained, a connect(), accept(), or recv() call
 * only informs the API that such an operation should be simulated.  After
 * calling connect(), accept(), or recv() the calling code should surrender
 * control back to the simulator (i.e. return) so that the requested operation
 * can be simulated.  When the operation finishes simulation the corresponding
 * callback will be invoked, namely: the connectCallback() is invoked when a
 * a socket is connected or fails to connect; the acceptCallback() is invoked
 * when a new connection is accepted on a listening socket; the recvCallback()
 * is invoked when data is received over a connection, the connection closes,
 * or the the connection times out.
 *
 * @par
 * The implementation of the callback should handle the event and then invoke
 * further appropriate socket operations.  The functions accept() and recv()
 * are not persistent, meaning that they may not be invoked once and thereafter
 * be expected to trigger their corresponding callback whenever an accept or
 * receive event occurs.  Generally, if repeated accept or recv operations are
 * required they should be reinvoked in the acceptCallback() or recvCallback()
 * implementation respectively; in a synchronous environment this is in	essence
 * what a loop in an accepting or receiving thread does.
 *
 * @par
 * When connect(), accept(), or recv() are invoked, a pointer to any
 * element (data/struct/object) can be provided (cast it as a void pointer).
 * The API will not modify the element pointed to.  This pointer will be
 * returned in the operation's corresponding callback when the operation
 * finishes simulation.  This feature was provided with the intent that context
 * associated with a specific socket could be maintained via this API; however,
 * use of this feature is now discouraged since the API cannot deallocate the
 * element in the event that the simulation terminates abnormally, yet the using
 * application can deallocate the element if it stores the context information.
 *
 * @par	Synchronous to Asynchronous Pseudocode Example:
 * In a synchronous environment, a client application may have the following
 * sequence of socket operations (without specific regard to the BSD API;
 * comments are between '#' symbols):
 *
 * @par
 * <pre>
 * some thread ...
 *     fd = socket ()
 *     bind(fd, local_address, local_port)
 *     connect(fd, remote_address, remote_port)
 *     send(fd, some_data)
 *     repeat until some condition
 *         data = recv(fd)
 *         # process the data #
 *         send(fd, some_more_data)
 * </pre>
 *
 * @par
 * Using this asynchronous API, the same functionality can be achieved as
 * follows:
 *
 * @par
 * <pre>
 * function: connectionInitializer
 *     fd = socket(pointer to callback interface object)
 *     bind(fd, local_address, local_port)
 *     connect(fd, remote_address, remote_port, context_data)
 *     # connectCallback will be called when the socket is connected)
 * # end of connectionInitializer #
 * </pre>
 *
 * @par
 * <pre>
 * function: connectCallback(fd, status, context_data)
 *     send(fd, some_data)
 *     recv(fd, desired_number_of_bytes, context_data)
 *     # recvCallback will be called when data is received #
 * # end of connectCallback #
 * </pre>
 *
 * @par
 * <pre>
 * function: recvCallback(fd, status, data, context_data)
 *     # process the data #
 *     send(fd, some_more_data)
 *     recv(fd, desired_number_of_bytes, context_data)
 *     # recvCallback will be called when data is received #
 * # end of recvCallback #
 * </pre>
 *
 * @par Memory Management Notes:
 * If a non-NULL element pointer is provided to a connect(), accept(), or
 * recv() invocation (this use is discouraged) the application should be
 * aware of the following memory management issues:
 *
 * @par
 * The API never assumes responsibility for deallocating from heap space any
 * non-NULL element pointer provided in a connect(), accept(), or recv()
 * invocation.  The API will, however, track the provided pointer until a
 * connect, accept, or recv operation finishes simulation and pass back the
 * pointer in the corresponding callback.  When this occurs the API stops
 * tracking the element pointer (internally it sets	its pointers to NULL).
 * The callback implementation should therefore handle the element pointer--
 * either reusing it in a subsequent connect, accept, or recv invocation, or
 * deleting	it, etc.  When a socket is closed it will return a void pointer;
 * in the event that close() is invoked after a connect, accept, or recv
 * invocation, but before the operation finishes simulation, then the pointer
 * provided in the invocation will be returned.
 *
 * @par API Extension Requirements:
 * This base API defines application data type and time type independent
 * socket operations, and is therefore functionally incomplete.  API
 * extensions should provide send and setTimeout functions specific to the
 * desired application data type and time type respectively;  API extensions
 * should also extend the CallbackHandler class to provide a recvCallback
 * function specific to the desired application data type.
 *
 * @par Possible Improvements:
 * (priority levels: low number = high priority, high number = low priority)
 *
 * @todo Level 1 Instrument with different error handling options: throw
 * exceptions (currently does); return error codes and set an error
 * description string.
 *
 * @todo Level 1 Add an emergency callback function to the callback handler
 * class that will notify the handler of abnormal socket failure, or define
 * a diferent exception to be thrown that will indicate that condition when
 * an operation is requested on a failed/unstable socket.
 */

#endif /* _TCPSOCKETAPI_BASE_H_ */
