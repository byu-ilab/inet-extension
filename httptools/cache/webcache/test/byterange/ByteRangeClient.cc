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

#define DEBUG_CLASS true

void ByteRangeClient::initialize()
{
	wwwName = par("address").stdstringValue();
	if ( wwwName.size() == 0 )
	{
		wwwName += getParentModule()->getFullName();
		wwwName += ".omnet.net";
	}
	LOG_DEBUG_LN("Initializing HTTP Byte Range Client. Using WWW name " << wwwName);
	port = par("port");
		ASSERT(0 <= port && port < 65536);

	httpProtocol = par("httpProtocol");
		ASSERT(httpProtocol == 10 || httpProtocol == 11);

	// logging parameters
	ll = par("logLevel");
	logFileName = (const char*)par("logFile");
	enableLogging = logFileName!="";
	outputFormat = lf_short;


	_socketapi = findTCPSocketMgr(this);

	_controller = check_and_cast<httptController *>(simulation.getSystemModule()->getSubmodule(par("controller")));

	_num_requests_to_make = par("numRequests");
	_num_requests_made = 0;
	_request_round = 0;

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
			createSocket(_num_requests_made, i);
		}
	}
	delete msg;
}

void ByteRangeClient::connectCallback(int socket_id, int ret_status, void * myPtr)
{
	RangeRequestInfo * rri_ptr = static_cast<RangeRequestInfo *>(myPtr);
	if (!rri_ptr)
	{
		throw cRuntimeError(this, "NULL value for connect callback");
	}

//	int * rid_ptr = static_cast<int *>(myPtr);
//	if (!rid_ptr)
//	{
//		throw cRuntimeError(this, "NULL value for connect callback");
//	}
//
//	int range_id = *rid_ptr;

	if (TCPSocketAPI_Inet::isCallbackError(ret_status))
	{
		LOG_DEBUG_LN("Error connecting on request "<<rri_ptr->request_id<<
			" to request byte range "<<rri_ptr->range_id);
		delete rri_ptr;
		_socketapi->close(socket_id);
		//createSocket(range_id);
		return;
	}

	sendBRRequest(socket_id, rri_ptr->request_id, rri_ptr->range_id);
//
//	int range_id = rri_ptr->range_id;//*rid_ptr;
//
//	httptByteRangeRequestMessage * request = NULL;
//
//	if (range_id == _num_ranges -1)
//	{
//		request = generateBRRequest("somefile", range_id * _range_size, -1);
//	}
//	else
//	{
//		request = generateBRRequest("somefile", range_id * _range_size, (range_id+1)*_range_size - 1);
//	}
//
//	_socketapi->send(socket_id, request);
	_socketapi->recv(socket_id, TCPSocketAPI_Base::RECV_MODE_WHOLE, myPtr);
}

void ByteRangeClient::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	RangeRequestInfo * rri_ptr = static_cast<RangeRequestInfo *>(myPtr);
	if (!rri_ptr)
	{
		throw cRuntimeError(this, "NULL value for connect callback");
	}


//	int * rid_ptr = static_cast<int *>(myPtr);
//	if (!rid_ptr)
//	{
//		throw cRuntimeError(this, "NULL value for recv callback");
//	}
//
//	int range_id = *rid_ptr;

	if (TCPSocketAPI_Inet::isCallbackError(ret_status))
	{
		LOG_DEBUG_LN("Error receiving on request "<<rri_ptr->request_id<<
					" to request byte range "<<rri_ptr->range_id);
		delete rri_ptr;//id_ptr;
		_socketapi->close(socket_id);
		return;
	}

	httptReplyMessage * reply = check_and_cast<httptReplyMessage *>(msg);

//	if (!reply)
//	{
//		delete rri_ptr;//id_ptr;
//		throw cRuntimeError(this, "received message on request %d for byte range %d is not a byte range reply message.",
//				rri_ptr->request_id, rri_ptr->range_id);
//	}

	if (reply->result() != 206)
	{
		LOG_DEBUG_LN("result code: "<<reply->result()<<"\nexpected: 206");
	}

	int range_id = rri_ptr->range_id;

	if (reply->firstBytePos() != range_id * _range_size)
	{
		LOG_DEBUG_LN("first byte of returned message is: "<<reply->firstBytePos()<<
			"\nexpected: "<<range_id*_range_size);
	}

	if (range_id == _num_ranges - 1)
	{
		if (reply->lastBytePos() != _file_size -1 )
		{
			LOG_DEBUG_LN("last byte of returned message is: "<<reply->lastBytePos()<<
				"\nexpected: "<<_file_size-1);
		}
	}
	else if (reply->lastBytePos() !=  (range_id+1) * _range_size - 1 )
	{
		LOG_DEBUG_LN("last byte of returned message is: "<<reply->lastBytePos()<<
			"\nexpected: "<<(range_id+1)*_range_size-1);

	}

	if (reply->instanceLength() != _file_size)
	{
		LOG_DEBUG_LN("instance length is: "<<reply->instanceLength()<<
			"\nexpected: "<<_file_size);
	}

	delete reply;

	if (rri_ptr->request_id < _num_requests_to_make -1)
	{
		sendBRRequest(socket_id, rri_ptr->request_id+1, rri_ptr->range_id);
		rri_ptr->request_id = rri_ptr->request_id+1;
		_socketapi->recv(socket_id, TCPSocketAPI_Base::RECV_MODE_WHOLE, (void *) rri_ptr);
	}
	else if (_request_round == 0)
	{
		sendBRRequest(socket_id, 0, rri_ptr->range_id);
		rri_ptr->request_id = 0;
		_socketapi->recv(socket_id, TCPSocketAPI_Base::RECV_MODE_WHOLE, (void *) rri_ptr);

		if (_num_requests_made - (_num_requests_to_make*_num_ranges) == _num_ranges)
		{
			_request_round++;
		}
	}
	else
	{
		delete rri_ptr;//id_ptr;
		_socketapi->close(socket_id);
	}
}

int ByteRangeClient::createSocket(int request_id, int range_id)
{
	RangeRequestInfo * rri = new RangeRequestInfo(request_id, range_id);

	//char * serverhostname = new char[512];
	char serverhostname[512];
	int serverport = 0;
	_controller->getServerInfo(par("serverwww").stringValue(), serverhostname, serverport);

	//std::string servername = _controller->getServerModule(par("serverwww"))->getName();
	LOG_DEBUG_LN("extracted server name: "<<serverhostname);
	return _socketapi->makeActiveSocket(this, "", -1,
			serverhostname, serverport /*par("serverport")*/, (void *) rri);
}

httptRequestMessage * ByteRangeClient::generateBRRequest(const std::string & uri, int fbp, int lbp)
{
	std::string header = "GET "+uri+" HTTP/1.";
	switch(httpProtocol)
	{
	case 10: header = header + "0"; break;
	case 11: header = header + "1"; break;
	default:
		error("Unknown HTTP protocol");
	}

	httptRequestMessage * request = new httptRequestMessage(header.c_str());

	request->setHeading(header.c_str());
	request->setOriginatorUrl("");
	request->setTargetUrl(par("serverwww"));
	request->setMethod(RM_GET);
	request->setUri(uri.c_str());
	request->setProtocol(httpProtocol);
	request->setByteLength(100);
	request->setKind(HTTPT_REQUEST_MESSAGE);
	request->setFirstBytePos(fbp);
	request->setLastBytePos(lbp);

	return request;
}

void ByteRangeClient::sendBRRequest(int socket_id, int request_id, int range_id)
{
	httptRequestMessage * request = NULL;

	std::stringstream filename;
	filename << "somefile" << request_id;

	if (range_id == _num_ranges - 1)
	{
		request = generateBRRequest(filename.str(), range_id * _range_size, -1);
	}
	else
	{
		request = generateBRRequest(filename.str(), range_id * _range_size, (range_id+1)*_range_size - 1);
	}

	_num_requests_made++;
	LOG_DEBUG_LN("sending request "<<_num_requests_made);
	_socketapi->send(socket_id, request);
}



