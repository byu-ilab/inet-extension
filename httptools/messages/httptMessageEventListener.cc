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

#include "httptMessageEventListener.h"

//---

void httptMessageEventListener::handleSignal(cComponent *source, simsignal_t signalID, cMessageEventDatagram * obj)
{
	if (dynamic_cast<httptMessageEventDatagram *>(obj) == NULL)
	{
		throw cRuntimeError("The object pointer is not of type httptMessageEventDatagram.");
	}

	// else if

	httptRequestEventDatagram * req_datagram = dynamic_cast<httptRequestEventDatagram *>(obj);
	if (req_datagram != NULL)
	{
		handleRequestMessageEvent(source, req_datagram);
		return;
	}

	// else if

	httptReplyEventDatagram * rep_datagram = dynamic_cast<httptReplyEventDatagram *>(obj);
	if (rep_datagram != NULL)
	{
		handleReplyMessageEvent(source, rep_datagram);
		return;
	}

	// else

	throw cRuntimeError("Unknown httptMessageEventDatagram object pointer.");
}

//---

Enforce_Single_Class_Instance_Definitions(httptDuplicateMessageEventListener)

httptDuplicateMessageEventListener::httptDuplicateMessageEventListener()
{
	_logfilename = "";
	_duplicates = 0;
}

httptDuplicateMessageEventListener::~httptDuplicateMessageEventListener()
{
}

void httptDuplicateMessageEventListener::handleRequestMessageEvent(cComponent * source, httptRequestEventDatagram * datagram)
{
	const httptRequestMessage * req_ptr = datagram->getRequestMessage();
//	URIVarientKey key(req_ptr->uri(), datagram->getInterfaceID());
	DuplicateRecordKey key(source->getFullPath(), req_ptr->uri(), datagram->getInterfaceID());
	updateValue(_request_message_records, key);//, source->getFullPath());
}

void httptDuplicateMessageEventListener::handleReplyMessageEvent(cComponent * source, httptReplyEventDatagram * datagram)
{
	const httptReplyMessage * rep_ptr = datagram->getReplyMessage();
	//URIVarientKey key(rep_ptr->relatedUri(), datagram->getInterfaceID());
	DuplicateRecordKey key(source->getFullPath(), rep_ptr->relatedUri(), datagram->getInterfaceID());
	updateValue(_reply_message_records, key);//, source->getFullPath());
}

/*
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
*/
#define WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,RECORDS,TYPE) \
	for (DuplicateRecordMap::iterator r_itr = RECORDS.begin(); r_itr != RECORDS.end(); r_itr++) \
	{ \
		if (r_itr->second.size() > 1) \
		{ \
			WHERE << TYPE <<": source: "<<r_itr->first.getSource()<<" interface: "<<r_itr->first.getInterface()<< \
					" uri: "<<r_itr->first.getName()<< \
					" duplicates: "<<(r_itr->second.size() - 1)<< \
					"\n\ttimes: "; \
			std::list<SimTime>::iterator et_end_itr = r_itr->second.end(); \
			for (std::list<SimTime>::iterator et_itr = r_itr->second.begin(); \
				et_itr != et_end_itr; et_itr++) { WHERE << (*et_itr) << " "; } \
			WHERE << std::endl; \
		} \
	}

#define WRITE_LOG(WHERE) \
	WHERE << "Detected duplicates: "<<_duplicates<<endl; \
	WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,_request_message_records,"REQ") \
	WRITE_DUPLICATES_IN_MSG_RECORDS(WHERE,_reply_message_records,"REP")

void httptDuplicateMessageEventListener::finish(cComponent *component, simsignal_t signalID)
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

void httptDuplicateMessageEventListener::updateValue(DuplicateRecordMap & records,
		/*URIVarientSimTimeMap & records,*/ DuplicateRecordKey & key)
{
	DuplicateRecordMap::iterator r_itr = records.find(key);
	if (r_itr != records.end())
	{
//		r_itr->second.event_times.push_back(simTime());
		r_itr->second.push_back(simTime());
		_duplicates++;
	}
	else
	{
		DuplicateMessageEvents entry;//(source_name);
		//entry.event_times.push_back(simTime());
		entry.push_back(simTime());
		records[key] = entry;
	}
	/*
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
	*/
}

//---

void httptRequestEventDatagram::copy(const httptRequestEventDatagram & other)
{
	ASSERT(this != &other);

	this->deleteMessage();

	if (other.getRequestMessage() != NULL)
	{
		this->setRequestMessage(other.getRequestMessage());
	}

	this->setInterfaceID(other.getInterfaceID());
}

httptRequestEventDatagram & httptRequestEventDatagram::operator = (const httptRequestEventDatagram & other)
{
	if (this != &other)
	{
		copy(other);
	}

	return *this;
}

void httptRequestEventDatagram::setMessage(const cMessage * msg)
{
	const httptRequestMessage * req_ptr = dynamic_cast<const httptRequestMessage *>(msg);
	if (req_ptr == NULL)
	{
		throw cRuntimeError("Message pointer is not of type httptRequestMessage.");
	}

	setRequestMessage(req_ptr);
}

void httptRequestEventDatagram::setRequestMessage (const httptRequestMessage * msg)
{
	cMessageEventDatagram::setMessage(msg);
}

const httptRequestMessage * httptRequestEventDatagram::getRequestMessage() const
{
	return dynamic_cast<const httptRequestMessage *>(getMessage());
}

//---

void httptReplyEventDatagram::copy(const httptReplyEventDatagram & other)
{
	ASSERT(this != &other);

	this->deleteMessage();

	if (other.getReplyMessage() != NULL)
	{
		this->setReplyMessage(other.getReplyMessage());
	}

	this->setInterfaceID(other.getInterfaceID());
}

httptReplyEventDatagram & httptReplyEventDatagram::operator = (const httptReplyEventDatagram & other)
{
	if (this != &other)
	{
		copy(other);
	}

	return *this;
}

void httptReplyEventDatagram::setMessage(const cMessage * msg)
{
	const httptReplyMessage * rep_ptr = dynamic_cast<const httptReplyMessage *>(msg);
	if (rep_ptr == NULL)
	{
		throw cRuntimeError("Message pointer is not of type httptRequestMessage.");
	}

	setReplyMessage(rep_ptr);
}

void httptReplyEventDatagram::setReplyMessage(const httptReplyMessage * msg)
{
	cMessageEventDatagram::setMessage(msg);
}

const httptReplyMessage * httptReplyEventDatagram::getReplyMessage() const
{
	return dynamic_cast<const httptReplyMessage *>(getMessage());
}
