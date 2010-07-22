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

#ifndef TCPCONNINFODATAGRAM_H_
#define TCPCONNINFODATAGRAM_H_

#include <omnetppextension.h>
#include <cobserver.h>

class TCPConnInfoDatagram : public cObserverDatagram
{
private:
	int _conn_id;
	int _local_port;
	int _remote_port;
	std::string _local_address;
	std::string _remote_address;

	void initialize(const int & conn_id, const int & local_port, const int & remote_port,
			const std::string & local_address, const std::string & remote_address);

public:
	TCPConnInfoDatagram();
	TCPConnInfoDatagram(int conn_id, int local_port, int remote_port,
			const std::string & local_address, const std::string & remote_address);
	// Keep default copy constructor and assignment operator
	virtual ~TCPConnInfoDatagram() {}

	bool operator ==(const TCPConnInfoDatagram & other) const;

	/*
	 * Compares this TCPConnInfoDatagram with other.
	 *
	 * @return An integer < 0 if this < other, 0 if this == other, and
	 * an integer > 0 if this > other.
	 */
	//int compare(const TCPConnInfoDatagram & other);

	/** @name Getters */
	//@{

	int getConnId() const {	return _conn_id; }
	int getLocalPort() const { return _local_port;	}
	int getRemotePort() const { return _remote_port; }
	const std::string & getLocalAddress() const { return _local_address; }
	const std::string & getRemoteAddress() const {	return _remote_address;	}

	//@}

	/** @name Setters */
	//@{

	virtual void setConnId(int conn_id) { this->_conn_id = conn_id; }
	virtual void setLocalPort(int local_port) { this->_local_port = local_port; }
	virtual void setRemotePort(int remote_port) { this->_remote_port = remote_port; }
	virtual void setLocalAddress(const std::string & local_address) { this->_local_address = local_address; }
	virtual void setRemoteAddress(const std::string & remote_address) { this->_remote_address = remote_address; }

	//@}
};

#endif /* TCPCONNINFODATAGRAM_H_ */
