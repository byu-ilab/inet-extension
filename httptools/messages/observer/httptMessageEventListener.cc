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

void httptMessageEventObserver::handleSignal(cComponent *source, simsignal_t signalID, cMessageEventDatagram * obj)
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

Enforce_Single_Class_Instance_Definitions(DuplicateHttptMessageEventObserver)

DuplicateHttptMessageEventObserver::DuplicateHttptMessageEventObserver()
{
	_duplicates = 0;
}

DuplicateHttptMessageEventObserver::~DuplicateHttptMessageEventObserver()
{
}

void DuplicateHttptMessageEventObserver::handleRequestMessageEvent(cComponent * source, httptRequestEventDatagram * datagram)
{
	const httptRequestMessage * req_ptr = datagram->getRequestMessage();
	DuplicateRecordKey key(source->getFullPath(), req_ptr->uri(), datagram->getInterfaceID());
	updateValue(_request_message_records, key);
}

void DuplicateHttptMessageEventObserver::handleReplyMessageEvent(cComponent * source, httptReplyEventDatagram * datagram)
{
	const httptReplyMessage * rep_ptr = datagram->getReplyMessage();
	DuplicateRecordKey key(source->getFullPath(), rep_ptr->relatedUri(), datagram->getInterfaceID());
	updateValue(_reply_message_records, key);
}

void DuplicateHttptMessageEventObserver::updateValue(DuplicateRecordMap & records, DuplicateRecordKey & key)
{
	DuplicateRecordMap::iterator r_itr = records.find(key);
	if (r_itr != records.end())
	{
		r_itr->second.push_back(simTime());
		_duplicates++;
	}
	else
	{
		DuplicateMessageEvents entry;
		entry.push_back(simTime());
		records[key] = entry;
	}
}

void DuplicateHttptMessageEventObserver::finish(cComponent *component, simsignal_t signalID)
{
	if (!getLogFilename().empty())
	{
		std::ofstream logfile(getLogFilename().c_str());
		if (!logfile.fail() && !logfile.bad())
		{
			printReport(logfile);
			return;
		}
	}

	// Print out to the environment
	printReport(ev.getOStream());
}

void DuplicateHttptMessageEventObserver::printReport(std::ostream & out_stream)
{
	out_stream << "Detected duplicates: "<<_duplicates<<endl;
	printDuplicateRecords(out_stream, _request_message_records, "REQ");
	printDuplicateRecords(out_stream, _reply_message_records, "REP");
}

void DuplicateHttptMessageEventObserver::printDuplicateRecords(
		std::ostream & out_stream, DuplicateRecordMap & records, const std::string & prefix)
{
	for (DuplicateRecordMap::const_iterator r_itr = records.begin(); r_itr != records.end(); r_itr++)
	{
		if (1 < r_itr->second.size())
		{
			out_stream << prefix <<": source: "<<r_itr->first.getSource()<<
					" interface: "<<r_itr->first.getInterface()<<
					" uri: "<<r_itr->first.getName()<<
					" duplicates: "<<(r_itr->second.size() - 1)<<
					"\n\ttimes: ";
			std::list<SimTime>::const_iterator et_end_itr = r_itr->second.end();
			for (std::list<SimTime>::const_iterator et_itr = r_itr->second.begin();
				et_itr != et_end_itr; et_itr++)
			{
				out_stream << (*et_itr) << " ";
			}
			out_stream << std::endl;
		}
	}
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
