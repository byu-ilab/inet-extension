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
#include "MsgByteBuffer.h"

// From omnetpp extension
#include <omnetppextension.h>

#define DEBUG_CLASS false

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
	if (cbobj == NULL)
	{
		throw cRuntimeError("Pointer is NULL.");
	}

	cb_inet_handler_ptr_t inet_cbobj = dynamic_cast<cb_inet_handler_ptr_t>(cbobj);

	if (inet_cbobj == NULL)
	{
		throw cRuntimeError("Pointer is not of type TCPSocketAPI_Inet::CallbackHandler.");
	}
	//else
	return inet_cbobj;
}

//socket_id_t TCPSocketAPI_Inet::socket(cb_base_handler_ptr_t cbobj)
//{
//	return this->socket( verifyCallbackHandlerType(cbobj) );
//}

//void TCPSocketAPI_Inet::listen (
//	socket_id_t id,
//	cb_base_handler_ptr_t cbobj_for_accepted)
//{
//	if (cbobj_for_accepted == NULL)
//	{
//		return this->listen(id, static_cast<cb_inet_handler_ptr_t>(NULL));
//	}
//	else
//	{
//		return this->listen(id, verifyCallbackHandlerType(cbobj_for_accepted));
//	}
//}

//---

typedef TCPSocketAPI_Inet::LogicalAppMsgRecord LAMR;
typedef LAMR * lam_record_ptr_t;
typedef std::deque<lam_record_ptr_t> LAMRQueue;

TCPSocketAPI_Inet::LogicalAppMsgRecord::LogicalAppMsgRecord(cPacket* msg, bytecount_t rcvd_so_far)
{
	setMessage(msg);
	setRcvdBytes(rcvd_so_far);
}

TCPSocketAPI_Inet::LogicalAppMsgRecord::~LogicalAppMsgRecord()
{
	if (_message != NULL)
	{
		delete _message;
	}
}

bytecount_t TCPSocketAPI_Inet::LogicalAppMsgRecord::insertBytes(bytecount_t buffer)
{
	bytecount_t needed = getOutstandingBytes();
	bytecount_t extra = 0;

	if (buffer <= needed)
	{
		_rcvd_bytes += buffer;
	}
	else // fact: needed < buffer
	{
		_rcvd_bytes += needed;
		ASSERT(isComplete());
		extra = buffer - needed;
	}

	return extra;
}

bytecount_t TCPSocketAPI_Inet::LogicalAppMsgRecord::extractAvailableBytes(MsgByteBuffer * buffer, bytecount_t max)
{
	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(buffer != NULL);
	ASSERT(_extracted_bytes <= _rcvd_bytes);

	if (getAvailableBytes() == 0) // available bytes already extracted
	{
		return 0;
	}
	ASSERT(0 < _rcvd_bytes);

	if (_extracted_bytes == 0)
	{
		buffer->addPayloadMessage(_message->dup());
	}

	bytecount_t extracted = getAvailableBytes();
	if (max < extracted && max != BYTECOUNT_NULL)
	{
		extracted = max;
	}
	_extracted_bytes += extracted;

	buffer->setByteLength(buffer->getByteLength() + extracted);

	LOG_DEBUG_FUN_END("");

	return extracted;
}

bool TCPSocketAPI_Inet::LogicalAppMsgRecord::isFullyExtracted() const
{
	return getExtractedBytes() == getExpectedBytes();
}

void TCPSocketAPI_Inet::LogicalAppMsgRecord::setMessage(cPacket * msg)
{
	ASSERT(msg != NULL);
	ASSERT(0 <= msg->getByteLength());

	_message = msg;
	_expected_bytes = _message->getByteLength();
	_extracted_bytes = 0;
}

void TCPSocketAPI_Inet::LogicalAppMsgRecord::setRcvdBytes(bytecount_t rcvd_so_far)
{
	ASSERT(0 <= rcvd_so_far);
	ASSERT(rcvd_so_far <= _expected_bytes);

	_rcvd_bytes = rcvd_so_far;
}

bool TCPSocketAPI_Inet::LogicalAppMsgRecord::isComplete() const
{
	return _expected_bytes == _rcvd_bytes;
}

bytecount_t TCPSocketAPI_Inet::LogicalAppMsgRecord::getAvailableBytes() const
{
	return _rcvd_bytes - _extracted_bytes;
}

//---

TCPSocketAPI_Inet::ReceiveBuffer::ReceiveBuffer()
{
}

TCPSocketAPI_Inet::ReceiveBuffer::~ReceiveBuffer()
{
	LAMRQueue::iterator bitr = _buffer.begin();
	for ( ; bitr != _buffer.end(); bitr++)
	{
		if ((*bitr)->getMessage() != NULL)
		{
			delete (*bitr)->getMessage();
		}
	}
}

void TCPSocketAPI_Inet::ReceiveBuffer::insertData(cPacket * msg)
{
	ASSERT(msg != NULL);
	ASSERT(0 <= msg->getByteLength()); // TODO should virtual virtual messages be allowed (i.e. of size 0 bytes)?

	LOG_DEBUG_FUN_BEGIN("msgs in buffer: "<<_buffer.size());

	LOG_DEBUG_APPEND_LN("inserting msg: "<<msg->getName()<<" of size: "<<msg->getByteLength());

	MsgByteBuffer * mbb = dynamic_cast<MsgByteBuffer *>(msg);
	if (mbb == NULL)
	{
		lam_record_ptr_t record = new LAMR(msg, msg->getByteLength());
		ASSERT(record->isComplete());
		_buffer.push_back(record);
	}
	else // fact: mbb != NULL
	{
		lam_record_ptr_t record = (!_buffer.empty()) ? _buffer.back() : NULL;
		bytecount_t remainder = mbb->getByteLength();

		// while there are unaccounted bytes
		while (0 < remainder)
		{
			LOG_DEBUG_APPEND_LN("remaining rcvd bytes: "<<remainder);
			LOG_DEBUG_APPEND_LN("bytes to rcv: "<<(record != NULL ? record->getOutstandingBytes() : -1));
			LOG_DEBUG_APPEND_LN("byte buffer payload array size: "<<mbb->getPayloadArraySize());

			// If there is not a reply that is currently being received
			if (record == NULL)
			{
				// then extract it from the byte buffer
				ASSERT(0 < mbb->getPayloadArraySize());

				cPacket * plmsg = mbb->removeFirstPayloadMessage();

				ASSERT(plmsg != NULL);
				LOG_DEBUG_APPEND_LN("beginning to receive "<<plmsg->getName());

				record = new LAMR(plmsg, 0);
				_buffer.push_back(record);
			}

			remainder = record->insertBytes(remainder);

			if (record->isComplete())
			{
				record = NULL;
			}

			ASSERT(0 <= remainder);
			if (0 == remainder)
			{
				ASSERT(0 == mbb->getPayloadArraySize());
//				ASSERT(record == NULL); // may not be null if the mbb doesn't have whole message
			}
			else // fact: 0 < remainder
			{
				ASSERT(0 < mbb->getPayloadArraySize());
			}
		}
	}

	LOG_DEBUG_FUN_END("msgs in buffer: "<<_buffer.size());
}

cPacket * TCPSocketAPI_Inet::ReceiveBuffer::extractAvailableBytes(bytecount_t recv_mode)
{
	LOG_DEBUG_FUN_BEGIN("");

	if (_buffer.empty())
	{
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}

	cPacket * ret_packet = NULL;

	switch(recv_mode)
	{

	case TCPSocketAPI_Base::RECV_MODE_WHOLE:
		ret_packet = extractWhole();
		break;

	case TCPSocketAPI_Base::RECV_MODE_INSTANT_MAINTAIN_BOUNDARIES:
		ret_packet = extractInstantMaintainBoundaries();
		break;

	case TCPSocketAPI_Base::RECV_MODE_INSTANT_NO_BUFFER:
		ret_packet = extractInstantNoBuffer();
		break;

	default:
		ASSERT(0 < recv_mode); // if not, unknown alternate recv mode
		ret_packet = extractUpto(recv_mode);
		break;
	}

	if (!_buffer.empty())
	{
		ASSERT(!_buffer.front()->isFullyExtracted()); // if not, then handling
		// code didn't delete the extracted record from the buffer
	}

	LOG_DEBUG_FUN_END("extracted "<<(ret_packet != NULL ? ret_packet->getByteLength() : 0) <<" bytes");

	return ret_packet;
}

cPacket *  TCPSocketAPI_Inet::ReceiveBuffer::extractWhole()
{
	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(!_buffer.empty()); // should be checked by extractAvailableBytes

	lam_record_ptr_t record = _buffer.front();

	if (!record->isComplete())
	{
		LOG_DEBUG_FUN_END("no whole message");
		return NULL;
	}
	// else

	MsgByteBuffer * ret_buffer = new MsgByteBuffer("byte buffer");

	bytecount_t extracted = record->extractAvailableBytes(ret_buffer);

	ASSERT(0 < extracted);
	ASSERT(record->isFullyExtracted());

	_buffer.pop_front();
	delete record;
	record = NULL;

	if (ret_buffer->getPayloadArraySize() == 1)
	{
		cPacket * ret_packet = ret_buffer->removeFirstPayloadMessage();
		delete ret_buffer;

		LOG_DEBUG_FUN_END("one whole message");
		return ret_packet;
	}
	// else
	LOG_DEBUG_FUN_END("more than one whole message?");
	return ret_buffer;
}

cPacket * TCPSocketAPI_Inet::ReceiveBuffer::extractInstantMaintainBoundaries()
{
	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(!_buffer.empty()); // should be checked by extractAvailableBytes

	lam_record_ptr_t record = _buffer.front();

	if (record->getAvailableBytes() == 0)
	{
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}

	MsgByteBuffer * ret_buffer = new MsgByteBuffer("byte buffer");

	bytecount_t extracted = record->extractAvailableBytes(ret_buffer);

	if (record->isFullyExtracted())
	{
		_buffer.pop_front();
		delete record;
		record = NULL;
	}

	if (0 == extracted)
	{
		delete ret_buffer;
		LOG_DEBUG_FUN_END("");
		return NULL;
	}
	// else
	LOG_DEBUG_FUN_END("");
	return ret_buffer;
}

cPacket * TCPSocketAPI_Inet::ReceiveBuffer::extractInstantNoBuffer()
{
	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(!_buffer.empty()); // should be checked by extractAvailableBytes

	lam_record_ptr_t record = _buffer.front();

	if (record->getAvailableBytes() == 0)
	{
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}

	MsgByteBuffer * ret_buffer = new MsgByteBuffer("byte buffer");

	bytecount_t extracted = 0;

	do
	{
		extracted = record->extractAvailableBytes(ret_buffer);
		if (record->isFullyExtracted())
		{
			_buffer.pop_front();
			delete record;
			record = (!_buffer.empty()) ? _buffer.front() : NULL;
		}
	} while (record != NULL && 0 < extracted);

	if (0 == ret_buffer->getByteLength())
	{
		delete ret_buffer;
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}
	// else
	LOG_DEBUG_FUN_END("message byte buffer returned");
	return ret_buffer;
}

cPacket * TCPSocketAPI_Inet::ReceiveBuffer::extractUpto(bytecount_t max)
{
	LOG_DEBUG_FUN_BEGIN("");

	ASSERT(!_buffer.empty()); // should be checked by extractAvailableBytes
	ASSERT(0 < max);

	lam_record_ptr_t record = _buffer.front();

	if (record->getAvailableBytes() == 0)
	{
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}

	MsgByteBuffer * ret_buffer = new MsgByteBuffer("byte buffer");

	bytecount_t extracted = record->extractAvailableBytes(ret_buffer, max);

	ASSERT(extracted <= max);

	if (extracted == 0)
	{
		delete ret_buffer;
		LOG_DEBUG_FUN_END("no available bytes");
		return NULL;
	}
	else if (record->isFullyExtracted())
	{
		_buffer.pop_front();
		delete record;
	}

	LOG_DEBUG_FUN_END("returning "<<extracted<<" bytes");
	return ret_buffer;
}







