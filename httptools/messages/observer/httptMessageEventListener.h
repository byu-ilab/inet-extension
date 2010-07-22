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

#ifndef HTTPTMESSAGEEVENTLISTENER_H_
#define HTTPTMESSAGEEVENTLISTENER_H_

#include <cmessageeventobserver.h>
#include "URIVarientSimTimeMap.h"
#include "httptMessages_m.h"
#include <fstream>
#include "DuplicateRecord.h"

class httptMessageEventDatagram;
class httptRequestEventDatagram;
class httptReplyEventDatagram;

class httptMessageEventObserver : public cMessageEventObserver
{
private:
	std::string __default_signal_name;

protected:
	/** @name Implemented from cMessageEventObserver */
	//@{

	/**
	 * Checks that the datagram pointer is of type httptMessageEventDatagram and
	 * forwards it to the appropriate handler (e.g. Request or Reply).
	 *
	 * @throw throws a cRuntimeError if the object pointer is not of type
	 * httptMessageEventDatagram.
	 */
	virtual void handleSignal(cComponent * source, simsignal_t signalID, cMessageEventDatagram * datagram);

	//@}


	/** @name Overridden from cMessageEventObserver */
	//@{

	/**
	 * Returns the default signal name for all httptMessageEventObservers.
	 */
	virtual const std::string & getDefaultSignalName() const { return __default_signal_name; }

	//@}


	/** @name To be implemented by subclasses */
	//@{

	virtual void handleRequestMessageEvent(cComponent * component, httptRequestEventDatagram * datagram) =0;
	virtual void handleReplyMessageEvent(cComponent * component, httptReplyEventDatagram * datagram) =0;

	//@}
};

class DuplicateHttptMessageEventObserver : public httptMessageEventObserver, public noncopyable
{
	Enforce_Single_Class_Instance_Declarations(DuplicateHttptMessageEventObserver);

private:

	DuplicateRecordMap _request_message_records;
	DuplicateRecordMap _reply_message_records;
	uint64 _duplicates;

public:
	/** @name Overridden from cIListener */
	//@{

	virtual void finish(cComponent *component, simsignal_t signalID);

	//@}

protected:
	/** @name Implemented from httptMessageEventObserver */
	//@{

	virtual void handleRequestMessageEvent(cComponent * source, httptRequestEventDatagram * datagram);
	virtual void handleReplyMessageEvent(cComponent * source, httptReplyEventDatagram * datagram);

	//@}

	/** @name httptDuplicateMessageEventListener functions */
	//@{

	virtual void updateValue(DuplicateRecordMap & records, DuplicateRecordKey & key);

	virtual void printReport(std::ostream & out_stream) const;

	virtual void printDuplicateRecords(std::ostream & out_stream, DuplicateRecordMap & records,
			const std::string & prefix) const;

	//@}
};


class httptMessageEventDatagram : public cMessageEventDatagram
{
protected:
	httptMessageEventDatagram() : cMessageEventDatagram()/*, _interface_id(-1)*/ {}
	virtual ~httptMessageEventDatagram() {}

public:

	/** @name Overridden from cMessageEventDatagram */
	//@{

	/*
	 * Subclasses should ensure that the message pointer is of the appropriate type,
	 * and if not throw a cRuntimeError.
	 */
	virtual void setMessage(const cMessage * msg) =0;

	//@}
};


class httptRequestEventDatagram : public httptMessageEventDatagram
{
private:
	void copy(const httptRequestEventDatagram & other);

public:
	httptRequestEventDatagram() : httptMessageEventDatagram() {}
	httptRequestEventDatagram(const httptRequestEventDatagram & other) : httptMessageEventDatagram() { copy(other); }
	virtual ~httptRequestEventDatagram() {}

	httptRequestEventDatagram & operator = (const httptRequestEventDatagram & other);

	/** @name Overridden from httptMessageEventDatagram */
	//@{

	/*
	 * Ensures that the message pointer is of type httptRequestMessage.
	 *
	 * @throws a cRuntimeError if the message pointer is not of type httptRequestMessage.
	 */
	virtual void setMessage(const cMessage * msg);

	//@}

	virtual void setRequestMessage (const httptRequestMessage * msg);
	const httptRequestMessage * getRequestMessage() const;
};


class httptReplyEventDatagram : public httptMessageEventDatagram
{
private:
	void copy(const httptReplyEventDatagram & other);

public:
	httptReplyEventDatagram() : httptMessageEventDatagram() {}
	httptReplyEventDatagram(const httptReplyEventDatagram & other) : httptMessageEventDatagram() { copy(other); }
	virtual ~httptReplyEventDatagram() {}

	httptReplyEventDatagram & operator = (const httptReplyEventDatagram & other);
	/** @name Overridden from httptMessageEventDatagram */
	//@{

	/*
	 * Ensures that the message pointer is of type httptReplyMessage.
	 *
	 * @throws a cRuntimeError if the message pointer is not of type httptReplyMessage.
	 */
	virtual void setMessage(const cMessage * msg);

	//@}

	virtual void setReplyMessage(const httptReplyMessage * msg);
	const httptReplyMessage * getReplyMessage() const;
};

#endif /* HTTPTMESSAGEEVENTLISTENER_H_ */
