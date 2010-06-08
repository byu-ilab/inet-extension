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

#include "ByteRangeClient.h"

Define_Module(ByteRangeClient);

void ByteRangeClient::initialize()
{
	wwwName = par("address").stdstringValue();
	if ( wwwName.size() == 0 )
	{
		wwwName += getParentModule()->getFullName();
		wwwName += ".omnet.net";
	}
	EV_DEBUG << "Initializing HTTP server. Using WWW name " << wwwName << endl;
	port = par("port");
		ASSERT(0 <= port && port < 65536);

	httpProtocol = par("httpProtocol");
		ASSERT(httpProtocol == 10 || httpProtocol == 11);

	// logging parameters
	ll = par("logLevel");
	logFileName = (const char*)par("logFile");
	enableLogging = logFileName!="";
	outputFormat = lf_short;


	_socketapi = findTCPSocketAPI(this);

	_controller = check_and_cast<httptController *>(simulation.getSystemModule()->getSubmodule(par("controller")));

	_file_size = par("requestFileSize");
	_file_size *= 1024;
	_num_ranges = par("numRanges");
	ASSERT(_file_size > 0);
	ASSERT(_num_ranges > 0);
	ASSERT(_file_size > _num_ranges);
	_range_size = _file_size / _num_ranges;

	cMessage * msg = new cMessage("start");
	scheduleAt(0, msg);
}

void ByteRangeClient::handleMessage(cMessage *msg)
{
	if (msg->isSelfMessage())
	{
		// create one socket for each range to be requested
		for (int i = 0; i < _num_ranges; i++)
		{
			createSocket(i);
		}
	}
	delete msg;
}

void ByteRangeClient::connectCallback(int socket_id, int ret_status, void * myPtr)
{
	int * rid_ptr = static_cast<int *>(myPtr);
	if (!rid_ptr)
	{
		throw cRuntimeError(this, "NULL value for connect callback");
	}

	int range_id = *rid_ptr;

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		EV_DEBUG << "Error connecting to request byte range "<<range_id<<endl;
		_socketapi->close(socket_id);
		//createSocket(range_id);
		return;
	}

	httptByteRangeRequestMessage * request = NULL;

	if (range_id == _num_ranges -1)
	{
		request = generateBRRequest("somefile", range_id * _range_size, -1);
	}
	else
	{
		request = generateBRRequest("somefile", range_id * _range_size, (range_id+1)*_range_size - 1);
	}

	_socketapi->send(socket_id, request);
	_socketapi->recv(socket_id, myPtr);
}

void ByteRangeClient::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	int * rid_ptr = static_cast<int *>(myPtr);
	if (!rid_ptr)
	{
		throw cRuntimeError(this, "NULL value for recv callback");
	}

	int range_id = *rid_ptr;

	if (TCPSocketAPI::isCallbackError(ret_status))
	{
		EV_DEBUG << "Error receiving byte range "<<range_id<<endl;
		delete rid_ptr;
		_socketapi->close(socket_id);
		return;
	}

	httptByteRangeReplyMessage * br_reply = dynamic_cast<httptByteRangeReplyMessage *>(msg);
	if (!br_reply)
	{
		throw cRuntimeError(this, "received message for byte range %d is not a byte range reply message.", range_id);
	}

	if (br_reply->result() != 206)
	{
		EV_DEBUG << "result code: "<<br_reply->result()<<"\nexpected: 206\n";
	}

	if (br_reply->firstBytePos() != range_id * _range_size)
	{
		EV_DEBUG << "first byte of returned message is: "<<br_reply->firstBytePos()<<
			"\nexpected: "<<range_id*_range_size<<endl;
	}

	if (range_id == _num_ranges - 1)
	{
		if (br_reply->lastBytePos() != (range_id+1) * _range_size -1 )
		{
			EV_DEBUG << "last byte of returned message is: "<<br_reply->lastBytePos()<<
						"\nexpected: "<<(range_id+1)*_range_size-1<<endl;
		}
	}
	else if (br_reply->lastBytePos() != _file_size -1 )
	{
		EV_DEBUG << "last byte of returned message is: "<<br_reply->lastBytePos()<<
								"\nexpected: "<<_file_size-1<<endl;
	}

	if (br_reply->instanceLength() != _file_size)
	{
		EV_DEBUG << "instance length is: "<<br_reply->instanceLength()<<
			"\nexpected: "<<_file_size<<endl;
	}

	delete rid_ptr;
	_socketapi->close(socket_id);
}

int ByteRangeClient::createSocket(int range_id)
{
	int * rid = new int(range_id);
	//char * serverhostname = new char[512];
	char serverhostname[512];
	int serverport = 0;
	_controller->getServerInfo(par("serverwww").stringValue(), serverhostname, serverport);

	//std::string servername = _controller->getServerModule(par("serverwww"))->getName();
	EV_DEBUG << "extracted server name: "<<serverhostname<<endl;
	return _socketapi->makeActiveSocket(this, "", -1,
			serverhostname, serverport /*par("serverport")*/, (void *) rid);
}

httptByteRangeRequestMessage * ByteRangeClient::generateBRRequest(const std::string & uri, int fbp, int lbp)
{
	std::string header = "GET "+uri+" HTTP/1.";
	switch(httpProtocol)
	{
	case 10: header = header + "0"; break;
	case 11: header = header + "1"; break;
	defualt:
		error("Unknown HTTP protocol");
	}

	httptByteRangeRequestMessage * request = new httptByteRangeRequestMessage(header.c_str());

	request->setHeading(header.c_str());
	request->setOriginatorUrl("");
	request->setTargetUrl(par("serverwww"));
	request->setProtocol(httpProtocol);
	request->setByteLength(100);
	request->setKind(HTTPT_REQUEST_MESSAGE);
	request->setFirstBytePos(fbp);
	request->setLastBytePos(lbp);

	return request;
}



