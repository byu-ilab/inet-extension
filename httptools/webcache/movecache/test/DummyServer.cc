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

#include "DummyServer.h"

Define_Module(DummyServer);

void DummyServer::initialize()
{
	httptServerBase::initialize();

	_socketapi = findTCPSocketAPI(this);

	int _listening_fd = _socketapi->makePassiveSocket(this, "", port, NULL);
	_socketapi->accept(_listening_fd, NULL);
}

void DummyServer::handleMessage(cMessage *msg)
{
	delete msg;
}

void DummyServer::acceptCallback(int socket_id, int ret_status, void * myPtr)
{
	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		_socketapi->close(socket_id);
		return;
	}
	_socketapi->recv(ret_status, NULL);
	_socketapi->accept(socket_id, NULL);
}

void DummyServer::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		_socketapi->close(socket_id);
		return;
	}
	httptRequestMessage * request = check_and_cast<httptRequestMessage *>(msg);
	cStringTokenizer tokenizer = cStringTokenizer(request->heading()," ");
	vector<string> res = tokenizer.asVector();
	int filesize = par("fileSize");
	httptReplyMessage * reply = generateStandardReply(request, res[1], 200, 1024*filesize, rt_unknown);
	_socketapi->send(socket_id, reply);
	_socketapi->recv(socket_id, NULL);
	delete msg;
}
