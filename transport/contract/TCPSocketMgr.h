//==========================================================================80>
/**
 * \file TCPSocketMgr.h
 *
 * TCPSocketMgr class declarations.
 *
 * Created: August 2, 2010
 * 
 * @todo Add GPL notice.
 */

#ifndef __INET__TCPSOCKETMGR_H_
#define __INET__TCPSOCKETMGR_H_

// From inet
#include "TCPSocketAPI_Inet.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "IPAddressResolver.h"
#include "SocketTimeoutMsg_m.h"

// From standard C++ libraries
#include <map>
#include <set>
#include <deque>

// Type defs
/** Maps socket id to socket timeout message pointer. */
typedef std::map<socket_id_t, SocketTimeoutMsg *> 	Id_TimeoutMsgMap;

/** Set of socket ids. */
typedef std::set<socket_id_t> IdSet;

/** Maps port to socket id set. */
typedef std::map<port_t, IdSet > Port_IdSetMap;

/** Deque of socket ids. */
typedef std::deque<socket_id_t> IdDeque;

/** Maps socket id to socket id deque. */
typedef std::map<socket_id_t, IdDeque > Id_IdDequeMap;

/** cPacket pointer deque. */
typedef std::deque<cPacket *> PacketDeque;

/** Maps socket id to cPacket pointer deque. */
typedef std::map<socket_id_t, PacketDeque > Id_PacketDequeMap;



// Class documentation at bottom of file
class INET_API TCPSocketMgr
	: public cSimpleModule /* already noncopyable */,
	  public TCPSocketAPI_Inet,
	  private TCPSocket::CallbackInterface
{
public:

	/** @name Static public functions */
	//@{

	/**
	 * @return True if the provided value corresponds to one of the
	 * values from the TCPSocketAPI_Base::CallbackError enumeration
	 * and false if it does not.
	 */
	//static bool isCallbackError(error_id_t error);


	/**
	 * @return The name of the error corresponding to the indicated
	 * value from the TCPSocketAPI::CallbackError enumeration; the
	 * default is "UNDEFINED".
	 */
	//static std::string getCallbackErrorName(error_id_t error);

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

	/** Tracks the timeout messages associated with a given socket.
	 *
	 * Maps socket id to timeout message.
	 */
	Id_TimeoutMsgMap _timeout_timers;

	/** Used to resolve provided string addresses to IPvXAddress objects. */
	IPAddressResolver _resolver;

	/** Tracks the ports that are being used (bound) and which
	 * socket(s) is (are) using it.
	 */
	Port_IdSetMap _bound_ports;

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

public:
	/** Data necessary to execute callbacks associated with a given socket. */
	struct CallbackData
	{
		/** The socket this callback data pertains to and the socket
		 * id to be returned in the callback.
		 */
		socket_id_t socket_id;

		/** The callback state of the socket.  See TCPSocketAPI::CALLBACK_STATE. */
		CALLBACK_STATE state;

		/** The CallbackInterface assigned to handle events on this socket. */
		cb_inet_handler_ptr_t cbobj;

		/** The data/struct/object the user provides. */
		user_data_ptr_t userptr;

		/** The CallbackInterface to be used by accepted connections.
		 * Useful if the user defines different handlers for active
		 * and passive sockets.  May be NULL in which case cbobj
		 * is used.
		 */
		cb_inet_handler_ptr_t cbobj_for_accepted;
	};

	// These type defs should also be included at the end of the file if they
	// are the return type of a function.

	/** CallbackData pointer type. */
	typedef TCPSocketMgr::CallbackData * cb_data_ptr_t;
	/** Maps socket port to callback data pointer. */
	typedef std::map<port_t, cb_data_ptr_t> Port_CBDataMap;
	/** Maps socket port to callback data pointer. */
	typedef std::map<port_t, cb_data_ptr_t> Id_CBDataMap;

protected:

	/** Tracks the callback data for passive sockets.
	 *
	 * Note that this separate map is needed because the the _bound_ports
	 * map doesn't have a feature to distinguish passive sockets
	 * from the active sockets that have been accepted on the listening
	 * socket's port.
	 */
	Port_CBDataMap _passive_callbacks;

	/** Tracks the callback data associated with a given socket. */
	Id_CBDataMap _registered_callbacks;

	/** Tracks the pending connections awaiting acceptance by passive sockets. */
	TCPSocketMap _pending_sockets_map;

	/** Tracks the pending connections awaiting acceptance by passive sockets.
	 *
	 * Maps passive socket id to id of pending TCPSocket in _pending_sockets_map.
	 */
	Id_IdDequeMap _pending_connections;

	/** Tracks received data not yet requested by the application.
	 *
	 * Maps active socket id to a list of cPacket pointers.
	 */
	Id_PacketDequeMap _reception_buffers;

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

	TCPSocketMgr ();
	virtual ~TCPSocketMgr ();

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
	virtual port_t getLocalPort (socket_id_t id);


	/** Accesses the remote port number of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote port number, -1 if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual port_t getRemotePort (socket_id_t id);


	/** Accesses the local address of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The local address, empty ("") if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual address_t getLocalAddress (socket_id_t id);


	/** Accesses the remote address of the indicated socket.
	 *
	 * @param id -- The descriptor to identify the socket in question.
	 *
	 * @return The remote address, empty ("") if not specified.
	 *
	 * @throw Throws a std::exception if the provided socket id doesn't refer
	 * to a current socket.
	 */
	virtual address_t getRemoteAddres (socket_id_t id);

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
	virtual socket_id_t socket(cb_inet_handler_ptr_t cbobj);


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
			port_t local_port);


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
	virtual void connect (socket_id_t id, address_cref_t remote_address,
				port_t remote_port);

	virtual void connect (socket_id_t id, address_cref_t remote_address,
			port_t remote_port,	user_data_ptr_t yourPtr);


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
				cb_inet_handler_ptr_t cbobj_for_accepted=NULL);


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
				address_cref_t local_address,  port_t local_port,
				address_cref_t remote_address, port_t remote_port);

	virtual socket_id_t makeActiveSocket (cb_base_handler_ptr_t cbobj,
			address_cref_t local_address,  port_t local_port,
			address_cref_t remote_address, port_t remote_port,
			user_data_ptr_t yourPtr);


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
			cb_base_handler_ptr_t cbobj_for_accepted=NULL);


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
	virtual void accept (socket_id_t id);

	virtual void accept (socket_id_t id, user_data_ptr_t yourPtr);


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
	virtual void send (socket_id_t id, cPacket * msg);


	/**
	 * Signals to the (active) socket that it should receive incoming data.
	 *
	 * When data is actually received the recvCallback() function will be invoked on the
	 * socket's callback object.
	 *
	 * @param id -- The descriptor to identify the (active) socket that should
	 * 		now receive incoming data.
	 * @param byte_mode -- Indicates how incoming data should be buffered and passed back in
	 * 		the recvCallback().  A positive value indicates that incoming data should be
	 * 		buffered and that up to that many bytes	should be passed back in the
	 * 		recvCallback() when ready.  A value of RECV_MODE_PACKET indicates that incoming
	 * 		data should be buffered and that the recvCallback() should only be invoked when
	 * 		all of the bytes for the application message have been received (only applicable
	 * 		when using TCPMsgByteBufferSend/Recv queues).  A value of RECV_MODE_NO_BUFFER
	 * 		indicates that incoming data should not be buffered and rather be passed in the
	 * 		recvCallback() as soon as it is received.  The default is RECV_MODE_PACKET.
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
	virtual void recv (socket_id_t id, bytecount_t byte_mode=RECV_MODE_PACKET);
	virtual void recv (socket_id_t id, bytecount_t byte_mode, user_data_ptr_t yourPtr);


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
			simtime_t timeout_period);


	/**
	 * Removes the timeout on the indicated (active) socket.
	 *
	 * @param id -- the descriptor to identify the (active) socket that should
	 *  		have its timeout removed.
	 *
	 * @return True if there was a timeout set on the socket and it was removed; false
	 * otherwise.
	 */
	virtual bool removeTimeout (socket_id_t id);

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
	virtual user_data_ptr_t close (socket_id_t id);

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
	virtual user_data_ptr_t getMyPtr (socket_id_t id);

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
	virtual void cleanupSocket(socket_id_t id);

	/** Removes the indicated socket_id from the set of sockets associated with the socket's local port. */
	virtual void freePort(socket_id_t id);

	/** Finds the socket associated with the indicated id in the socket map
	 * and verifies that it is valid, if it is not it signals a function error
	 * using the indicated function name.
	 */
	virtual TCPSocket * findAndCheckSocket(socket_id_t id, str_cref_t fname);

public:
	/** Returns a string representing the indicated socket as a string.  Because
	 * of the pseudo TIMED_OUT state we don't want the user to worry about this
	 * string representation -- could intercept it and rewrite it.
	 */
	virtual std::string socketToString(socket_id_t id);

protected:
	/** Makes a new CallbackData object with the indicated values.  cbobj_for_accepted
	 * is always NULL.
	 */
	virtual cb_data_ptr_t makeCallbackData(socket_id_t id, cb_inet_handler_ptr_t cbobj,
			user_data_ptr_t userptr, CALLBACK_STATE type);

	/** Looks up the passive callback data associated with the indicated port.
	 *
	 * @return the callback data if it exists and NULL if there is no passive
	 * socket on the indicated port, or if the passive socket is not in the accept
	 * state
	 */
	virtual cb_data_ptr_t getAcceptCallback(port_t port);

	/** Looks up the passive callback data associated with the indicated port.
	 *
	 * @return the callback data if it exists and NULL if there is no passive
	 * socket on the indicated port.
	 */
	virtual cb_data_ptr_t getPassiveCallback(port_t port);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalFunctionError(str_cref_t fname, str_cref_t details);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBNullError(str_cref_t fname);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBStateReceptionError(str_cref_t fname, CALLBACK_STATE state);

	/** @throw Throws a cRuntimeError--on purpose! */
	virtual void signalCBStateInconsistentError(str_cref_t fname, CALLBACK_STATE state);

	/** Prints a notice on the simulation environment output. */
	virtual void printFunctionNotice(str_cref_t fname, str_cref_t notice);

	/** Prints a notice on the simulation environment output. */
	virtual void printCBStateReceptionNotice(str_cref_t fname, CALLBACK_STATE state);

	/** Emits tcp connection information from the given socket. */
	virtual void emitTCPConnInfo(TCPSocket * socket);

	/** Emits message event information given the socket descriptor and message. */
	virtual void emitMessageEvent(const cMessage * msg, int interface_id);

	//@}
};

// After the fact type defs
typedef TCPSocketMgr::cb_data_ptr_t cb_data_ptr_t;
//typedef TCPSocketMgr::Port_CBDataMap Port_CBDataMap;
//typedef TCPSocketMgr::Id_CBDataMap Id_CBDataMap;

/** \class TCPSocketMgr
 * Implements the TCPSocketAPI_Inet interface.  Uses TCPSocket objects to do so.
 * 
 * See documentation in TCPSocketAPI_Base for usage instructions.
 *
 * NOTES TO CONTRIBUTOR:
 *
 *	OVERVIEW:
 *
 *	The API was constructed taking advantage of the already existent TCPSocket
 *	and TCPSocketMap classes.  The TCPSocket's callback interface is based on
 *	events more than on socket operations, so one thing that this API does is
 *	to interpret those events back to the appropriate operations (e.g. the
 *	socketEstablished() callback could be either for a connect or accept operation).
 *	One operation that the TCPSocket doesn't have is recv, but it does define a
 *	socketDataArrived() callback.  This API interprets the socketDataArrived event
 *	as a recv() operation.
 *
 *	To support the connect(), accept(), and recv() operations, this API uses and
 *	manages callback state data to track what operation the user has most recently
 *	requested.  Not all of the enumerated callback states pertain to an operation's
 *	callback function (e.g. TCPSocketAPI::CB_S_WAIT), rather they indicate whether an operation
 *	that requires a callback can be validly invoked as well as indicating how an
 *	event signaled by the TCPSocket's callback interface should be handled.  For
 *	example recv() cannot be invoked on a socket before connect(), this is enforced
 *	by not setting the socket's associated callback data state to TCPSocketAPI::CB_S_WAIT until
 *	the socket is actually connected.  In addition, this API imposes a timed out
 *	state on sockets using the TCPSocketAPI::CB_S_TIMEOUT value while the underlying TCPSocket's
 *	actual state is still TCPSocket::CONNECTED.  This is a messy approach conceptually but
 *	required because the TCPSocket is not a cSimpleModule so it can't schedule
 *	timeout events.  This API itself could be improved by making a new TCPSocket
 *	class that would have access to scheduling events through the API and which
 *	could then be tasked with keeping track of a TIMED_OUT state, provide a recv
 *	operation, and demultiplex TCP message events directly to opertions.
 *
 *
 *	ASSUMPTIONS:
 *
 *	- Because the API forces close() to be called before a port can be reused via the
 *	bind() function, the close command will reach the TCP core before the new connect
 *	or listen command such that the port will successfully be reused.
 *
 *	- The TCP core will always (seemingly -- I haven't checked if there are options
 *	to control this) accept incoming
 *	connections to a listening socket.  However, the socket may not be accepting
 *	at the moment.  This API keeps a list of pending connections such that the
 *	next time the application invokes accept() a connection from the pending
 *	queue will be accepted.  This seems consistent with what the BSD socket API
 *	does (it specifies the queue size for pending connections and only returns
 *	a connection when accept is called once a connection is established).
 *
 *	- This API creates a receive buffer (of cPacket not bytes) in the event that
 *	packets are received when the application has not invoked recv().
 *
 *	TO IMPROVE: (priority levels: low number = high priority, high number = low priority)
 *
 *	@todo Level 1 (DONE) remove the hasCallback function from the callback interface, it isn't
 *	used and none of the callback functions will be invoked unless the user
 *	has previously invoked the corresponding operation
 *
 *	@todo Level 2 determine whether pending connections may become invalidated as they wait in
 *	the pending queue.  If they do take appropriate action to remove them from
 *	the pending connections queue.
 *
 *	@todo Level 2 assign ports when not specified in bind or connect since the underlying
 *	TCP core only assigns ports within the range 1025-5000
 *
 *	@todo Level 3 build a translator that will take in essential message parameters and return
 *	an appropriately encapsulated cMessage that can be sent, and then receive a cMessage
 *	and extract out the essential parameters
 *
 *	@todo Level 4 make a way to set error handling, different error handling options are to
 *	throw exceptions (which it does currently) or to return error codes and set an
 * error description string
 *
 *	@todo Level 4 make a way to signal to the application that the socket has been closed
 *	on a PEER_CLOSED message so that the application will not use that socket
 *	anymore?  Alternatively return an error code indicating that the socket is closed when
 *	an invalid operation is requested (i.e. recv).  In this case send will still be okay
 * but recv will not be.
 *
 *	@todo Level 5 make a new TCPSocket class which has a TIMED_OUT state and has recv, accept,
 *	and timeout options, it could be a friend of the API or be given a pointer to the
 *	API so that it can schedule events (i.e. timeouts)
 *
 * @todo Level 5 make different send modes, currently the user may call send whenever the
 * underlying TCPSocket is in the TCPSocket::CONNECTED or TCPSocket::CONNECTING or
 *	TCPSocket::PEER_CLOSED state, perhaps
 * it would be nice to have a lock step mode such that messages could only be sent when
 *	the socket is actually connected
 *
 *	@see TCPSocketAPI_Base, TCPSocketAPI_Inet, TCPSocket, TCPSocketMap, IPAddressResolver,
 * SocketTimeoutMsg, TCPCommand.
 */

#endif /* __INET__TCPSOCKETMGR_H_ */
