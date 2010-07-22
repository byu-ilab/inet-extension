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

#include "TCPConnInfoDatagram.h"

void TCPConnInfoDatagram::initialize(const int & conn_id, const int & local_port, const int & remote_port,
			const std::string & local_address, const std::string & remote_address)
{
	setConnId(conn_id);
	setLocalPort(local_port);
	setRemotePort(remote_port);
	setLocalAddress(local_address);
	setRemoteAddress(remote_address);
}

TCPConnInfoDatagram::TCPConnInfoDatagram()
	: cObserverDatagram()
{
	initialize(-1,-1,-1,"","");
}

TCPConnInfoDatagram::TCPConnInfoDatagram(int conn_id, int local_port, int remote_port,
			const std::string & local_address, const std::string & remote_address)

	: cObserverDatagram()
{
	initialize(conn_id, local_port, remote_port, local_address, remote_address);
}

bool TCPConnInfoDatagram::operator ==(const TCPConnInfoDatagram & other) const
{
	if (this == &other)
	{
		return true;
	}

	if (this->_conn_id != other._conn_id ||
		this->_local_port != other._local_port ||
		this->_remote_port != other._remote_port ||
		this->_local_address != other._local_address ||
		this->_remote_address != other._remote_address)
	{
		return false;
	}

	return true;
}
