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

#include "DuplicateHttpMessageNameObserver.h"
#include <stdstringutils.h>
#include <fstream>

#define DEBUG_CLASS false

Enforce_Single_Class_Instance_Definitions(DuplicateHttpMessageNameObserver)

DuplicateHttpMessageNameObserver::DuplicateHttpMessageNameObserver()
	: cMessageEventObserver()
{
	_duplicates = 0;
	setLogFilename("duplicate_http_message_names_report.txt");
}

DuplicateHttpMessageNameObserver::~DuplicateHttpMessageNameObserver()
{
}

void DuplicateHttpMessageNameObserver::handleSignal(cComponent * source, simsignal_t signalID, cMessageEventDatagram * datagram)
{
	std::string msg_name = datagram->getMessage()->getName();
	LOG_DEBUG("datagram message name: \'"<<datagram->getMessage()->getName()<<"\'");
	LOG_DEBUG("datagram message path: \'"<<datagram->getMessage()->getFullName()<<"\'");
	LOG_DEBUG("datagram message full path: \'"<<datagram->getMessage()->getFullPath()<<"\'");
	LOG_DEBUG("datagram interface id: "<<datagram->getInterfaceID());
	LOG_DEBUG("message name: "<<msg_name);
	if ( stdstring_contains(msg_name, "http", false) )
	{
		DuplicateRecordKey key(source->getFullPath(), msg_name, datagram->getInterfaceID());
		updateRecord(key, simTime()); //source->getFullPath(), simTime());
	}
}

void DuplicateHttpMessageNameObserver::finish(cComponent *component, simsignal_t signalID)
{
	if (!getLogFilename().empty())
	{
		std::ofstream logfile(getLogFilename().c_str());
		if (!logfile.fail() && !logfile.bad())
		{
			printDuplicateReport(logfile);
			return;
		}
	}

	// Print out to the environment
	printDuplicateReport(ev.getOStream());
}

void DuplicateHttpMessageNameObserver::printDuplicateReport(std::ostream & out_stream) const
{
	out_stream << "Detected duplicates: "<<_duplicates<<endl; \
	out_stream << "# message name, interface id pairs: "<<_message_records.size()<<endl; \
	for (DuplicateRecordMap::const_iterator r_itr = _message_records.begin();
			r_itr != _message_records.end(); r_itr++)
	{
		if (1 < r_itr->second.size())
		{
			out_stream << ": source: "<<r_itr->first.getSource()<<
					" interface: "<<r_itr->first.getInterface()<<
					" name: "<<r_itr->first.getName()<<
					" duplicates: "<<(r_itr->second.size() - 1)<<
					"\n\ttimes: ";
			DuplicateMessageEvents::const_iterator et_end_itr = r_itr->second.end();
			for (DuplicateMessageEvents::const_iterator et_itr = r_itr->second.begin();
					et_itr != et_end_itr; et_itr++)
			{
				out_stream << (*et_itr) << " ";
			}
			out_stream << std::endl;
		}
	}
}

void DuplicateHttpMessageNameObserver::updateRecord(DuplicateRecordKey & key, simtime_t_cref time)
{
	DuplicateRecordMap::iterator r_itr = _message_records.find(key);
	if (r_itr != _message_records.end())
	{
//		r_itr->second.event_times.push_back(time);
		r_itr->second.push_back(time);
		_duplicates++;
	}
	else
	{
		DuplicateMessageEvents entry;//(source_name);
		//entry.event_times.push_back(time);
		entry.push_back(time);
		_message_records[key] = entry;
	}
}
