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

#include "httptMessageObserver.h"

#define DEBUG_CLASS false

Register_Class(httptMessageObserverDatagram);

httptMessageObserverDatagram::httptMessageObserverDatagram()
{
	initialize();
}

httptMessageObserverDatagram::httptMessageObserverDatagram(httptBaseMessage * msg, int id)
{
	initialize();
	setMessage(msg);
	setInterfaceId(id);
}

httptMessageObserverDatagram::httptMessageObserverDatagram(const httptMessageObserverDatagram & other)
{
	initialize();
	copy(other);
}

httptMessageObserverDatagram::~httptMessageObserverDatagram()
{
	clean();
}

httptMessageObserverDatagram & httptMessageObserverDatagram::operator = (const httptMessageObserverDatagram & other)
{
	if (this != &other)
	{
		copy(other);
	}
	return *this;
}

void httptMessageObserverDatagram::initialize()
{
	_req_msg = NULL;
	_rep_msg = NULL;
	_interface_id = -1;
}

void httptMessageObserverDatagram::copy(const httptMessageObserverDatagram & other)
{
	/* Precondition checking */
	ASSERT(this != &other);
	ASSERT(other._req_msg != NULL || other._rep_msg != NULL);
	/* End precondition checking */

	clean();

	if (other._req_msg != NULL)
	{
		this->_req_msg = other._req_msg->dup();
	}
	else if (other._rep_msg != NULL)
	{
		this->_rep_msg = other._rep_msg->dup();
	}

	setInterfaceId(other._interface_id);
}

void httptMessageObserverDatagram::clean()
{
	deleteSafe(_req_msg);
	deleteSafe(_rep_msg);
	_interface_id = -1;
}

void httptMessageObserverDatagram::setMessage(const httptBaseMessage * msg)
{
	/* Precondition checking */
	ASSERT(msg != NULL);
	/* End precondition checking */

	if (dynamic_cast<const httptRequestMessage *>(msg) != NULL)
	{
		setRequestMessage(dynamic_cast<const httptRequestMessage *>(msg));
	}
	else if (dynamic_cast<const httptReplyMessage *>(msg) != NULL)
	{
		setReplyMessage(dynamic_cast<const httptReplyMessage *>(msg));
	}
	else
	{
		throw cRuntimeError(this, "Message %s is not a known httpt message.", msg->getName());
	}
}

void httptMessageObserverDatagram::setRequestMessage(const httptRequestMessage * msg)
{
	/* Precondition checking */
	ASSERT(msg != NULL);
	/* End precondition checking */

	this->_req_msg = msg->dup();
	deleteSafe(_rep_msg);
}

void httptMessageObserverDatagram::setReplyMessage(const httptReplyMessage * msg)
{
	/* Precondition checking */
	ASSERT(msg != NULL);
	/* End precondition checking */

	this->_rep_msg = msg->dup();
	deleteSafe(_req_msg);
}

void httptMessageObserverDatagram::setInterfaceId(int id)
{
	_interface_id = id;
}

httptBaseMessage * httptMessageObserverDatagram::getMessage() const
{
	/* Precondition checking */
	ASSERT(_req_msg != NULL || _rep_msg != NULL);
	/* End precondition checking */

	if (_req_msg != NULL)
	{
		return _req_msg;
	}

	return _rep_msg; // default is NULL anyways
}

httptRequestMessage * httptMessageObserverDatagram::getRequestMessage() const
{
	return _req_msg;
}

httptReplyMessage * httptMessageObserverDatagram::getReplyMessage() const
{
	return _rep_msg;
}

int httptMessageObserverDatagram::getInterfaceId() const
{
	return _interface_id;
}













Define_Module(httptMessageObserver);

void httptMessageObserver::initialize()
{
	_logfilename = par("logFilename").stdstringValue();
	_duplicates = 0;
	LOG_DEBUG("ready.");
}

//void httptMessageObserver::handleMessage(cMessage *msg)
//{
//	throw cRuntimerError()
//}

#define WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,RECORDS,TYPE) \
	for (URIVarientSimTimeMap::iterator r_itr = RECORDS.begin(); r_itr != RECORDS.end(); r_itr++) \
	{ \
		if (r_itr->second.message_id != 0) \
		{ \
			WHERE << TYPE <<": interface: "<<r_itr->first.getVarient()<< \
					" uri: "<<r_itr->first.getURI()<< \
					" duplicates: "<<r_itr->second.message_id<< \
					" last time: "<<r_itr->second.time<<std::endl; \
		} \
	}

#define WRITE_LOG(WHERE) \
	WHERE << "Detected duplicates: "<<_duplicates<<endl; \
	WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,_request_message_records,"REQ") \
	WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,_reply_message_records,"REP")

void httptMessageObserver::finish()
{
	if (!_logfilename.empty())
	{
		std::ofstream logfile(_logfilename.c_str());
		if (!logfile.fail() && !logfile.bad())
		{
			WRITE_LOG(logfile);
			return;
		}
	}
	// Print out to the environment
	WRITE_LOG(EV);
}

#undef WRITE_LOG
#undef WRITE_DUPLICATES_IN_MSG_RECORDS

void httptMessageObserver::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
	/* Precondition checking */
	ASSERT(source != NULL);
	ASSERT(obj != NULL);

	httptMessageObserverDatagram * datagram = check_and_cast<httptMessageObserverDatagram *>(obj);
	/* End precondition checking */

	if (datagram->getRequestMessage() != NULL)
	{
		URIVarientKey key(datagram->getRequestMessage()->uri(), datagram->getInterfaceId());
		updateValue(_request_message_records, key);
	}
	else // should be a reply message
	{
		URIVarientKey key(datagram->getReplyMessage()->relatedUri(), datagram->getInterfaceId());
		updateValue(_reply_message_records, key);
	}
}

void httptMessageObserver::updateValue(URIVarientSimTimeMap & records, URIVarientKey & key)
{
	URIVarientSimTimeMap::iterator r_itr = records.find(key);
	if (r_itr != records.end())
	{
		(r_itr->second.message_id)++;
		r_itr->second.time = simTime();
		_duplicates++;
	}
	else
	{
		records[key] = MsgIdTimestamp(0, simTime());
	}
}

