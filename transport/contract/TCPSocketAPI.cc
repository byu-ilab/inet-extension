// Author: Kevin Black
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

#include "TCPSocketAPI.h"

Define_Module(TCPSocketAPI);

void TCPSocketAPI::initialize()
{
	write_count = 0;
}

void TCPSocketAPI::handleMessage(cMessage *msg)
{
	// TODO - Generated method body
}

void TCPSocketAPI::write(std::string msg)
{
	write_count++;
	EV << "Message written from socket api: "<<msg << endl;
	EV << "Total messages written: "<<write_count << endl;
}

