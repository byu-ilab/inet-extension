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

#include "TCPConnInfoMappingObserver.h"

#define DEBUG_CLASS false

Enforce_Single_Class_Instance_Definitions(TCPConnInfoMappingObserver);

std::string TCPConnInfoMappingObserver::__default_signal_name = "tcpconninfo";

TCPConnInfoMappingObserver::TCPConnInfoMappingObserver()
	: cLoggableObserver()
{
	LOG_DEBUG_FUN_BEGIN("");

	setLogFilename("tcp_connection_mapping.txt");
	LOG_DEBUG_LN("filename: "<<getLogFilename());

	LOG_DEBUG_FUN_END("");
}

TCPConnInfoMappingObserver::~TCPConnInfoMappingObserver()
{
	LOG_DEBUG_LN("destroyed");
}

void TCPConnInfoMappingObserver::handleSignal(cComponent * source, simsignal_t signalID,
			cObserverDatagram * datagram)
{
	LOG_DEBUG_FUN_BEGIN("");
	TCPConnInfoDatagram * conn_info = dynamic_cast<TCPConnInfoDatagram *>(datagram);

	if (conn_info == NULL)
	{
		throw cRuntimeError("Datagram pointer is not of type TCPConnIdDatagram.");
	}

	bool pair_found = false;
	TCPConnInfoList::iterator uc_itr = _unmapped_connections.begin();
	while(uc_itr != _unmapped_connections.end())
	{
		if (arePair(*conn_info, *uc_itr))
		{
			_mapped_connection_pairs.push_back(TCPConnInfoPair(*conn_info, *uc_itr));
			_unmapped_connections.erase(uc_itr);
			pair_found = true;
			break;
		}
		uc_itr++;
	}

	if (!pair_found)
	{
		_unmapped_connections.push_back(*conn_info);
	}
	LOG_DEBUG_FUN_END("");
}

bool TCPConnInfoMappingObserver::arePair(const TCPConnInfoEntry & first, const TCPConnInfoEntry & second) const
{
	if (first.getLocalPort() != second.getRemotePort() ||
			first.getRemotePort() != second.getLocalPort() ||
			first.getLocalAddress() != second.getRemoteAddress() ||
			first.getRemoteAddress() != second.getLocalAddress())
	{
		return false;
	}

	return true;
}

void TCPConnInfoMappingObserver::finish(cComponent *component, simsignal_t signalID)
{
	LOG_DEBUG_FUN_BEGIN("");

	if (_getNewDataSinceFinishCalled())
	{
		bool print_to_ev = getLogFilename().empty();

		if (!print_to_ev)
		{
			// try opening output file
			LOG_DEBUG_LN("printing out to: "<<getLogFilename());
			std::ofstream logfile(getLogFilename().c_str());
			if (!logfile.fail() && !logfile.bad())
			{
				printMapping(logfile);
			}
			else
			{
				print_to_ev = true;
			}
		}

		if (print_to_ev)
		{
			LOG_DEBUG_LN("printing out to the simulation environment");
			// Print out to the environment
			printMapping(ev.getOStream());
		}
	}

	cLoggableObserver::finish(component, signalID); // sets state to indicate finish has been called

	LOG_DEBUG_FUN_END("");
}

void TCPConnInfoMappingObserver::printMapping(std::ostream & out_stream)
{
	LOG_DEBUG_FUN_BEGIN("");
	out_stream << "# Mapped connection pairs: "<<_mapped_connection_pairs.size()<<endl;
	out_stream << "# Unmapped connections: "<<_unmapped_connections.size()<<endl;
	out_stream << "Mapped connection pairs: "<<endl;
	out_stream << "local address:local port, conn_id <=> conn id, local address:local port"<<endl;

	for (TCPConnInfoPairList::iterator mc_itr = _mapped_connection_pairs.begin();
			mc_itr != _mapped_connection_pairs.end(); mc_itr++)
	{
		const TCPConnInfoEntry & first = (*mc_itr).first;
		const TCPConnInfoEntry & second = (*mc_itr).second;
		out_stream << first.getLocalAddress()<<":"<<first.getLocalPort()<<", "<<first.getConnId()<<
			" <=> "<<second.getConnId()<<", "<<second.getLocalAddress()<<":"<<second.getLocalPort()<<endl;
	}
	LOG_DEBUG_FUN_END("");
}

