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

// From standard C++ libraries
#include <deque>

// Forward declarations		// source should				// from
class MsgByteBuffer;		// #include "MsgByteBuffer.h"	// inet

/**
 * Extends the TCPSocketAPI_Base.  Intended for use with in the INET 
 * framework.  Specifies that the setTimeout() function accepts time values
 * of type simtime_t (i.e. SimTime object), that the send() function accepts
 * cPacket pointers, and that the recvCallback()'s message type is a
 * cPacket pointer.
 * 
 * @see TCPSocketAPI_Base
 */
class INET_API TCPSocketAPI_Inet : public TCPSocketAPI_Base
{

public:

	/** @name Callback Handler Extension */
	//@{

	/**
	 * Extends TCPSocketAPI_Base::CallbackHandler with a callback for recv
	 * operations for cPacket type application data.
	 */
	class CallbackHandler : public TCPSocketAPI_Base::CallbackHandler
	{
	public:
		/** Empty. */
		virtual ~CallbackHandler() {}

		/**
		 * @breif By default just deletes the received cPacket.
		 *
		 * @details Subclasses should: 1) handle the reception of the data
		 * specific to their application; 2) assume responsibility for the
		 * message pointer if it is non-NULL; 3) assume responsibility for
		 * user data pointer if it is non-NULL (e.g. reuse it or deallocate
		 * it).
		 *
		 * @param id -- Descriptor of the receiving socket.
		 * @param result -- Result of the previous recv() invocation.
		 * 		method or the number of bytes received.
		 * @param msg -- Pointer to the received message.
		 * @param context -- User data element pointer provided in previous
		 * 		recv() invocation.
		 *
		 * On reception success: @em msg will point to the received message
		 * and @em result will be the number of byes in the message.  The @em
		 * msg may be NULL and @em result a positive number if what was
		 * received was a range of bytes pertaining to a previously received
		 * logical application message.
		 *
		 * On reception error: @em msg will point to NULL and @em result will
		 * be a value from the enumeraion TCPSocketAPI::CALLBACK_ERROR.
		 */
		virtual void recvCallback(socket_id_t id, cb_status_t return_value,
				cPacket * msg, user_data_ptr_t yourPtr);
	};

	typedef TCPSocketAPI_Inet::CallbackHandler * cb_inet_handler_ptr_t;

	//@}


	/** @name Receive Buffer */
	//@{

	class LogicalAppMsgRecord
	{
	private:
		cPacket * _message;
		bytecount_t _expected_bytes;
		bytecount_t _rcvd_bytes;
		bytecount_t _extracted_bytes;

	public:
		LogicalAppMsgRecord(cPacket* msg, bytecount_t rcvd_so_far);
		virtual ~LogicalAppMsgRecord();

		cPacket * 	getMessage() const { return _message; }
		bytecount_t getExpectedBytes() const { return _expected_bytes; }
		bytecount_t getRcvdBytes() const { return _rcvd_bytes; }
		bytecount_t getOutstandingBytes() const { return _expected_bytes - _rcvd_bytes; }
		bytecount_t getExtractedBytes() const { return _extracted_bytes; }

		/** @return The number of bytes from @em buffer not needed by this LAMR. */
		virtual bytecount_t insertBytes(bytecount_t buffer);

		/** Buffer must not be NULL.
		 *
		 * If no bytes have been extracted before then the message object will be added
		 * to the buffer, otherwise just the byte length will be changed.
		 */
		virtual bytecount_t extractAvailableBytes(MsgByteBuffer * buffer, bytecount_t max=BYTECOUNT_NULL);

		virtual void setMessage(cPacket * msg);
		virtual void setRcvdBytes(bytecount_t rcvd_so_far);

		virtual bool isComplete() const;
		virtual bytecount_t getAvailableBytes() const;
	};

	class ReceiveBuffer
	{
	private:
		std::deque<LogicalAppMsgRecord *> _buffer;

	public:
		ReceiveBuffer();
		virtual ~ReceiveBuffer();

		virtual void insertData(cPacket * msg);
		virtual cPacket * extractAvailableBytes(bytecount_t recv_mode);

	protected:
		virtual cPacket * extractWhole();
		virtual cPacket * extractInstantMaintainBoundaries();
		virtual cPacket * extractInstantNoBuffer();
		virtual cPacket * extractUpto(bytecount_t max);
	};


	//@}


	/** @name Destructor */
	//@{

	/** @brief Empty. */
	virtual ~TCPSocketAPI_Inet () {}

	///@}



	/** @name BSD-like API Operations */
	//@{

//	/**
//	 * @copydoc TCPSocketAPI_Base::socket()
//	 *
//	 * Checks that the provided callback handler is of type
//	 * TCPSocketAPI_Inet::CallbackHandler and invokes socket() specific
//	 * to that handler type.
//	 */
//	virtual socket_id_t socket(cb_base_handler_ptr_t cbobj);
//
//
//	/** @copydoc TCPSocketAPI_Base::socket() */
//	virtual socket_id_t socket(cb_inet_handler_ptr_t cbobj) =0;
//
//
//	/**
//	 * @copydoc TCPSocketAPI_Base::listen()
//	 *
//	 * If a non-NULL callback handler is specified for the accepted sockets, checks
//	 * that the callback handler is of type TCPSocketAPI_Inet::CallbackHandler and
//	 * invokes listen() specific to that handler type.
//	 */
//	virtual void listen (socket_id_t id,
//				cb_base_handler_ptr_t cbobj_for_accepted=NULL);
//
//
//	/** @copydoc TCPSocketAPI_Base::listen() */
//	virtual void listen (socket_id_t id,
//				cb_inet_handler_ptr_t cbobj_for_accepted=NULL) =0;


	/**
	 * @brief Sends the indicated message on the specified socket.
	 *
	 * Currently the whole message is sent so there isn't a need to check for
	 * how many bytes were sent.  Takes control of the message (see
	 * documentation for cOwnedObject for more details on message taking).
	 *
	 * @param id -- Socket descriptor of the socket on which to send the
	 * 		message.
	 * @param msg -- Message to be sent.
	 *
	 * The @em msg must not be NULL.
	 * 
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void send (socket_id_t id, cPacket * msg) =0;


	/**
	 * @brief Sets a timeout on the indicated (active) socket.
	 *
	 * @param id -- Socket descriptor of the socket on which to set a timeout.
	 * @param timeout_period -- Period after which the socket should timeout if
	 * 		there has been no activity.
	 *
	 * The @em timeout_period must not be negative.
	 * 
	 * @throws Throws a std::exception if an error occurs.
	 */
	virtual void setTimeout(socket_id_t id, simtime_t timeout_period) =0;

	//@}


protected:
	/** @name CallbackHandler checking */
	//@{

	/**
	 * @brief Verifies that the specified callback handler is of type
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
