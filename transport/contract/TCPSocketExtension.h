//==========================================================================80>
/**
 * @file TCPSocketExtension.h
 *
 * TCPSocketExtension class declarations.
 *
 * Created July 28, 2010
 *
 * @todo Add GPL notice.
 */

#ifndef __INET__TCPSOCKETEXTENSION_H_
#define __INET__TCPSOCKETEXTENSION_H_

// From inet
#include "TCPSocket.h"
#include "TCPSocketAPI_Inet.h"
#include "IPAddressResolver.h"
#include "SocketTimeoutMsg_m.h"

// From standard C++ libraries
#include <deque>

/**
 * This class forms a wrapper around a regular TCPSocket object.
 *
 * The TCPSocketExtension adds recv() and accept() functions to the TCPSocket.
 * Buffers received bytes and returns them as requested by the application.
 * Queues connections that have been established on a passive socket until the
 * application asks to accept them.
 *
 * The interface is based off of the BSD-like interface that TCPSocketAPI_Inet
 * defines, thus this socket extension is specific to INET time and message
 * types.
 *
 * @see TCPSocket, TCPSocket::CallbackInterface, TCPSocketAPI_Inet,
 * TCPSocketAPI_Inet::CallbackInterface.
 */
class INET_API TCPSocketExtension : public TCPSocket
{
private:
	/** @name Instance Members */
	//@{

	cb_inet_handler_ptr_t _cb_handler;
	cb_inet_handler_ptr_t _cb_handler_for_accepted;

	user_data_ptr_t _user_context_data;

	cSimpleModule * _timeout_scheduler;
	SocketTimeoutMsg * _timeout_msg;

	IPAddressResolver _resolver;
	IPAddressResolver::ResolutionMode _resolution_mode;

	typedef std::deque<TCPSocketExtension *> SocketQueue;
	SocketQueue _sockets_pending_acceptance;

	bytecount_t _recv_mode;

	TCPSocketAPI_Inet::ReceiveBuffer _recv_buffer;
	//@}

	/** Initializes instance members with default values. */
	void initialize(cb_inet_handler_ptr_t handler,
			cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode);

public:

	/** @name Constructors and Destructor */
	//@{

	/**
	 * Constructs a TCPSocketExtension instance with no callback handler
	 * nor timeout scheduler, and an address resolution mode of
	 * IPAddressResolver::ADDR_PREFER_IPv4.
	 */
	TCPSocketExtension();

	/**
	 * @copydoc TCPSocket::TCPSocket(cMessage*)
	 */
	TCPSocketExtension(cMessage * msg);

	/**
	 * Constructs a TCPSocketExtension instance with the provided callback
	 * handler, timeout scheduler, and address resolution mode.
	 *
	 * @param handler -- Callback handler for this socket.
	 * @param scheduler -- Timeout scheduling interface for this socket.
	 * @param mode -- Address resolution mode.
	 *
	 * If @em scheduler is NULL then timeouts will not be supported.
	 *
	 * The default for @em mode is IPAddressResolver::ADDR_PREFER_IPv4
	 */
	TCPSocketExtension(cb_inet_handler_ptr_t handler,
			cSimpleModule * scheduler, IPAddressResolver::ResolutionMode mode=
					IPAddressResolver::ADDR_PREFER_IPv4);

	/** Empty. */
	virtual ~TCPSocketExtension();

	//@}


	/** @name BSD-like Socket Operations */
	//@{

	/**
	 * Resolves the given address to an IP address (should be an
	 * IPAddressResolver resolvable addresss).
	 *
	 * More convenient bind interface.
	 *
	 * When a socket is first created (via socket()) its local address is
	 * ADDRESS_NULL and its local port is PORT_NULL.  This bind function
	 * should only be used if at least one of the local parameters should have
	 * a value other than the default.
	 *
	 * @param local_address -- Address to bind to.
	 * @param local_port -- Port to bind to.
	 *
	 * The @em local_port must be on the range [TCP_PORT_RANGE_MIN,
	 * TCP_PORT_RANGE_MAX].
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
	virtual void bind(address_cref_t local_address, port_t local_port);

	/**
	 * Makes the socket a passive (listening) socket.
	 *
	 * The socket must be bound to a valid port (see bind()) before it can be
	 * made passive.
	 *
	 * @param cbobj_for_accepted -- A pointer to the callback handler for the
	 * 		sockets spawned (accepted) by this listening socket.
	 *
	 * If @em cbobj_for_accepted is NULL (the default) then the callback
	 * handler for the passive socket will be used as the handler for the
	 * spawned (i.e. accepted) sockets as well.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void listen (cb_inet_handler_ptr_t cb_handler_for_accepted=NULL);

	/**
	 * Makes the socket a passive (listening) socket that will only accept one
	 * connection.
	 *
	 * The socket must be bound to a valid port (see bind()) before it can be
	 * made passive.  Will only accept one connection.
	 *
	 * @param cbobj_for_accepted -- A pointer to the callback handler for the
	 * 		sockets spawned (accepted) by this listening socket.
	 *
	 * If @em cbobj_for_accepted is NULL (the default) then the callback
	 * handler for the passive socket will be used as the handler for the
	 * spawned (i.e. accepted) sockets as well.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void listenOnce(cb_inet_handler_ptr_t cb_handler_for_accepted=NULL);


	/**
	 * Signals to this (passive) socket that it should accept an incoming
	 * connection (or notify the callback handler of a buffered accepted
	 * connection).
	 *
	 * When a connection is actually accepted the acceptCallback() function
	 * will be invoked on the socket's associated callback handler.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void accept ();


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
	 * @deprecated Use of the @em context parameter is discouraged.  In the
	 * event that the simulation terminates abnormally the element pointed to
	 * by @em context cannot be deleted by this API since the underlying type
	 * is a void pointer.  However, the user application can delete it if it
	 * stores the context elements and associates them with the sockets it
	 * creates.  That is the preferred usage.
	 */
	virtual void accept (user_data_ptr_t context);


	/**
	 * Adds the indicated socket to the queue of pending accepted sockets.
	 *
	 * This socket must be a passive socket, that is it must be in either the
	 * LISTENING or ACCEPTING state.
	 *
	 * Sets the callback handler for the socket to be the callback handler
	 * for accepted sockets as set in previous call to listen().
	 *
	 * If the queue of pending accepted sockets is empty and this socket
	 * is in the ACCEPTING state then the acceptCallback() will be called
	 * immediately and the indicated socket will not be added.  Otherwise,
	 * the socket will be appended to the queue of pending sockets until
	 * the accept() operation is performed.
	 *
	 * @param socket -- Socket to be appended.
	 *
	 * The @em socket must not be NULL.
	 *
	 * @exception Throws a cRuntimeError if socket is NULL.
	 */
	virtual void appendAcceptedSocket(TCPSocketExtension * socket);


	/**
	 * Begins to connect the socket to the indicated remote address and remote
	 * port.
	 *
	 * When the connection is established then the connectCallback()
	 * function will be invoked on the socket's callback handler.
	 *
	 * @param remote_address -- Address to connect to.
	 * @param remote_port -- Port to connect to.
	 *
	 * Both @em remote_address and @em remote_port must be valid.
	 *
	 * @throws Throws a std::exception if an error occurs
	 */
	virtual void connect(address_cref_t remote_address, port_t remote_port);

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
	virtual void connect (address_cref_t remote_address, port_t remote_port,
			user_data_ptr_t context);

	/**
	 * @brief Sends the indicated message on the socket.
	 *
	 * Currently the whole message is sent so there isn't a need to check for
	 * how many bytes were sent.
	 *
	 * @param msg -- Message to be sent.
	 *
	 * The @em msg must not be NULL.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void send(cMessage *msg);


	/**
	 * Signals to this (active) socket that it should receive incoming
	 * data.
	 *
	 * When data is actually received the recvCallback() function will be
	 * invoked on the socket's associated callback handler.
	 *
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
	 * If @em byte_mode is TCPSocketAPI_Base::RECV_MODE_PACKET (the default)
	 * then incoming data is buffered until all of the bytes of logical
	 * application message have been received, where after the logical
	 * application message is passed back.  See
	 * TCPSocketAPI_Base::RECV_MODE_PACKET for further notes.
	 *
	 * If @em byte_mode is TCPSocketAPI_Base::RECV_MODE_NO_BUFFER then
	 * incoming data is passed back as soon as it is received.  See
	 * TCPSocketAPI_Base::RECV_MODE_NO_BUFFER for further notes.
	 *
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void recv (bytecount_t byte_mode=
			TCPSocketAPI_Base::RECV_MODE_WHOLE);


	/**
	 * Signals to this (active) socket that it should receive incoming
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
	virtual void recv (bytecount_t byte_mode, user_data_ptr_t context);

	/**
	 * @brief Sets a timeout on this (active) socket.
	 *
	 * @param timeout_period -- Period after which the socket should timeout if
	 * 		there has been no activity.
	 *
	 * The @em timeout_period must not be negative.
	 *
	 * This socket must be an active socket (i.e. in the CONNECTING, CONNECTED,
	 * or RECEIVING state).
	 *
	 * A timeout scheduler must be set for the timeout to work (see
	 * setTimeoutScheduler()).
	 *
	 * @throws Throws a std::exception if an error occurs.  Throws a cRuntimeError
	 * if there is not a timeout scheduler set or if the socket state will not
	 * allow the timeout to be set.
	 */
	virtual void setTimeout(simtime_t timeout_period);


	/**
	 * Removes the timeout on this (active) socket.
	 *
	 * @return True if there was a timeout set on the socket and it was
	 * removed; false otherwise.
	 *
	 * @exception Throws a cRuntimeError if there is no timeout scheduler set or if
	 * the socket state will not allow the timeout to be removed.
	 */
	virtual bool removeTimeout ();

	/**
	 * @return True if the socket state will allow the timeout period to be modified;
	 * false if the timeout period cannot be modified in the current state.
	 */
	virtual bool canModifyTimeout() const;


	/**
	 * Closes this socket.
	 *
	 * @throws Throws a std::excpetion if an error occurs.
	 */
	virtual void close ();

	//@}


	/** @name User Context Data Access */
	//@{

	/**
	 * Returns the pointer to the context element pointer provided in a
	 * connect(), accept(), or recv() invocation.
	 *
	 * Provided to facilitate tracking the context data a user associates with
	 * the socket.
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
	 */
	virtual user_data_ptr_t getUserContext ();

	virtual user_data_ptr_t removeUserContext ();

	//@}


	/** @name Timeout Scheduler */
	//@{

	/**
	 * Sets the timeout scheduler (cSimpleModule) to be used to perform
	 * timeouts.
	 *
	 * This function may only be called once with a non-NULL value so that
	 * timeout messages will not become invalid.
	 *
	 * @param scheduler -- Scheduler to use for timeouts.
	 *
	 * @return True if the scheduler was set for use and false if it was not.
	 */
	virtual bool setTimeoutScheduler(cSimpleModule * scheduler);

	//@}


	/** @name Address Resolution Accessors */
	//@{

	/**
	 * Specifies how string addresses should be resolved.
	 *
	 * @param mode -- How string address should be resolved.
	 */
	virtual void setAddressResolutionMode(IPAddressResolver::ResolutionMode mode);

	/**
	 * @retun The resolution mode currently being used to resolve string addresses
	 * to IPvXAddresses.
	 */
	IPAddressResolver::ResolutionMode getAddressResolutionMode();

	//@}


	/** @name Callback Handling Accessors */
	//@{

	/**
	 * @exception Throws an error since this is feature is not supported for the
	 * TCPSocketExtension.
	 *
	 * @see setCallbackHandler().
	 */
	virtual void setCallbackObject(TCPSocket::CallbackInterface *cb, void *yourPtr=NULL);

	/**
	 * Sets the callback handler for this socket.
	 *
	 * The callback handler must be designed to handle accept, connect, and recv
	 * callbacks instead of event callbacks like the regular TCPSocket uses.
	 *
	 * @param handler -- Handler for accept, connect, and recv callbacks.
	 *
	 * The @em handler must not be NULL.
	 */
	virtual void setCallbackHandler(cb_inet_handler_ptr_t handler);

	//@} End Callback Handlign Accessors


	/** @name Event Processing */
	//@{

	/**
	 * Processes the provided message.
	 *
	 * Processes timeout messages as well as standard TCP event messages.
	 * Updates state, accept queues, recv buffers, etc. accordingly and invokes
	 * any appropriate callbacks.
	 *
	 * @param msg -- Event message to be processed.
	 */
	virtual void processMessage (cMessage * msg);

protected:

	/** @todo Document. */
	virtual void processDataArrived(cPacket *msg, bool urgent);

	/** @todo Document. */
	virtual void processEstablished();

	/** @todo Document. */
	virtual void processPeerClosed();

	/** @todo Document. */
	virtual void processClosed();

	/** @todo Document. */
	virtual void processFailure(int code);

	/** @todo Document. */
	virtual void processStatusArrived(TCPStatusInfo *status);

	//@} End Event Processing
};

#endif /* __INET__TCPSOCKETEXTENSION_H_ */
