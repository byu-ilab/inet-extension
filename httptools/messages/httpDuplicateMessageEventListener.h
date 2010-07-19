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

#ifndef HTTPDUPLICATEMESSAGEEVENTLISTENER_H_
#define HTTPDUPLICATEMESSAGEEVENTLISTENER_H_

#include <omnetppextension.h>
#include <cMessageEventListener.h>
#include "DuplicateRecord.h"

/**
 * Tracks duplicate messages that contain HTTP (case insensitive) as a substring
 * in the message name.  This allows messages from the application layer (including
 * the socket api), and the transport layer to be handled by one listener without
 * casting the messages to an httptRequestMessage or httptReplyMessage, provided that
 * the TCP module sets the first TCP segment's name to the name of the request or
 * reply message encapsulated in it.
 */
class httpDuplicateMessageEventListener : public cMessageEventListener, public noncopyable
{
Enforce_Single_Class_Instance_Declarations(httpDuplicateMessageEventListener);

private:
	DuplicateRecordMap _message_records;
	std::string _logfilename;
	uint64 _duplicates;

public:
	/** @name Overridden from cIListener */
	//@{

	/**
	 * Writes the duplicate summary to the specified log file or to the simulation environment
	 * if not log file is specified.
	 */
	virtual void finish(cComponent *component, simsignal_t signalID);

	//@}

protected:
	/** @name Implemented from cMessageEventListener */
	//@{

	virtual void handleSignal(cComponent * source, simsignal_t signalID, cMessageEventDatagram * datagram);

	//@}

	/** @name httpDuplicateMessageEventListener functions */
	//@{

	virtual void updateRecord(DuplicateRecordKey & key, simtime_t time); //const std::string & source_name, simtime_t time);

	inline const std::string & _get_log_filename() const { return _logfilename; }
	inline const uint64 & _get_duplicate_count()   const { return _duplicates; }

public:

	virtual void setLogFilename(const std::string & filename) { _logfilename = filename; }
	std::string  getLogFilename() const { return _logfilename; }

	//@}
};

#endif /* HTTPDUPLICATEMESSAGEEVENTLISTENER_H_ */
