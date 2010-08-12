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

#ifndef TCPCONNIDMAPPINGOBSERVER_H_
#define TCPCONNIDMAPPINGOBSERVER_H_

// From omnetpp extension
#include <omnetppextension.h>
#include <cobserver.h>

// From standard C++ libraries
#include <fstream>
#include <iostream>
#include <list>

// From inet
#include "TCPConnInfoDatagram.h"

//---

typedef TCPConnInfoDatagram TCPConnInfoEntry;
typedef std::list<TCPConnInfoEntry> TCPConnInfoList;

struct TCPConnInfoPair
{
	TCPConnInfoEntry first;
	TCPConnInfoEntry second;

	TCPConnInfoPair(const TCPConnInfoEntry & first, const TCPConnInfoEntry & second)
	{
		this->first = first;
		this->second = second;
	}
};

typedef std::list<TCPConnInfoPair> TCPConnInfoPairList;

class TCPConnInfoMappingObserver : public cLoggableObserver, public noncopyable
{

	Singleton_Declarations(TCPConnInfoMappingObserver);

private:
	static std::string __default_signal_name;

	TCPConnInfoList _unmapped_connections;
	TCPConnInfoPairList _mapped_connection_pairs;

protected:
	virtual bool arePair(const TCPConnInfoEntry & first, const TCPConnInfoEntry & second) const;

	virtual void printMapping(std::ostream & out_stream);

	/** @name Implemented cObserver functions */
	//@{

	/**
	 * Determines which tcp connections, as identified by their connection
	 * id are connected.
	 *
	 * @param source -- A cComponent pointer to the signal source.
	 * @param signalID -- The signal id.
	 * @param obj -- A cObserverDatagram object pointer which should be of
	 * 		type TCPConnIdDatagram.
	 *
	 * @return Nothing.
	 *
	 * @throw Throws a cRuntimeError if the datagram pointer is not of type
	 * TCPConnIdDatagram or is NULL.
	 */
	virtual void handleSignal(cComponent * source, simsignal_t signalID,
			cObserverDatagram * datagram);

public:

	/**
	 * Returns the default signal name of this observer.
	 */
	virtual const std::string & getDefaultSignalName() const { return __default_signal_name; }

	//@}


	/** @name Overridden from cIListener */
	//@{

	virtual void finish(cComponent *component, simsignal_t signalID);

	//@}
};

#endif /* TCPCONNIDMAPPINGOBSERVER_H_ */
