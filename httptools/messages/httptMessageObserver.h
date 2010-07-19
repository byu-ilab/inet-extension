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

#ifndef __INET_HTTPTMESSAGEOBSERVER_H_
#define __INET_HTTPTMESSAGEOBSERVER_H_

#include <omnetpp.h>
#include <omnetppextension.h>
#include <string>
#include <fstream>
#include "httptMessages_m.h"
#include "URIVarientSimTimeMap.h"

class httptMessageObserverDatagram : public cObject
{
private:
	httptRequestMessage * _req_msg;
	httptReplyMessage * _rep_msg;
	int _interface_id;

	void initialize();
	void copy(const httptMessageObserverDatagram & other);
	void clean();

public:
	httptMessageObserverDatagram();
	httptMessageObserverDatagram(httptBaseMessage * msg, int id);
	httptMessageObserverDatagram(const httptMessageObserverDatagram & other);
	virtual ~httptMessageObserverDatagram();

	httptMessageObserverDatagram & operator = (const httptMessageObserverDatagram & other);

	virtual void setMessage(const httptBaseMessage * msg);
	virtual void setRequestMessage(const httptRequestMessage * msg);
	virtual void setReplyMessage(const httptReplyMessage * msg);

	virtual void setInterfaceId(int id);

	httptBaseMessage * getMessage() const;
	httptRequestMessage * getRequestMessage() const;
	httptReplyMessage * getReplyMessage() const;
	int getInterfaceId() const;
};




class httptMessageObserver : public cSimpleModule, public cListener
{
private:
	/// Maps a URIVarient using the uri and interface id of the
	/// received signal object to a MsgIdTimestamp struct where
	/// the message_id field is used to count duplicates and the
	/// the time field is used to track the most recent duplicate.
	URIVarientSimTimeMap _request_message_records;
	URIVarientSimTimeMap _reply_message_records;
	std::string _logfilename;
	uint64 _duplicates;

protected:
	/** @name Overridden from cSimpleModule */
	//@{
    virtual void initialize();
    //virtual void handleMessage(cMessage *msg);
    virtual void finish();
    //@}

    /** @name Overridden from cListener */
    //@{
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
    //@}

    /** @name httptMessageObserver functions */
    //@{
    virtual void updateValue(URIVarientSimTimeMap & records, URIVarientKey & key);

    URIVarientSimTimeMap & getRequestRecords() { return _request_message_records; }
    URIVarientSimTimeMap & getReplyRecords()   { return _reply_message_records; }
    std::string & getLogFilename() { return _logfilename; }
    uint64 & getDuplicateCount() { return _duplicates; }
    //@}
};

#endif
