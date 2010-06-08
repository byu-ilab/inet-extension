// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ***************************************************************************

#include "videoServer.h"

Define_Module(videoServer);

void videoServer::initialize()
{
	httptServerBase::initialize();

    controller = dynamic_cast<httptController*>(getParentModule()->getParentModule()->getSubmodule("controller"));
	if (controller == NULL) {
		error("Controller module not found");
	}
	workload_generator = dynamic_cast<VideoTitleWorkloadGenerator*>(getParentModule()->getParentModule()->getSubmodule("workloadGenerator"));
	if (!workload_generator) {
		error("workload generator module not found");
	}
	updateDisplay();
	socketsBroken=0;
	socketsOpened=0;

	// get socket api
	std::string api_obj_name = par("socketapi").stringValue();
	if (api_obj_name.empty()) {
		opp_error("videoServer::initialize(): no tcp socket api specified!");
	}
	tcp_api = check_and_cast<TCPSocketAPI *>(getParentModule()->getSubmodule(api_obj_name.c_str()));

    cMessage * start = new cMessage("START",START);
    scheduleAt(simTime(),start);
}

void videoServer::finish()
{
	httptServerBase::finish();

	EV_SUMMARY << "Sockets opened: " << socketsOpened << endl;
	EV_SUMMARY << "Broken connections: " << numBroken << endl;

	recordScalar("sock.opened", socketsOpened);
	recordScalar("sock.broken", numBroken);
}

void httptServer::handleMessage(cMessage *msg)
{
	// setup listener socket.
	if (msg->getKind() == START) {
		int port = par("port");
	    int fd = tcp_api->socket(this);
	    tcp_api->bind(fd,"",port);
	    tcp_api->listen(fd);

	    tcp_api->accept (fd);
	}
	if (msg->isSelfMessage()){
    	//opp_error("WebCacheNewAPI::handleMessage(): received a non self message");
    }
	httptServerBase::handleMessage(msg);
	delete msg;
}


bool videoServer::hasCallback(TCPSocketAPI::CALLBACK_TYPE type){
	return (type == TCPSocketAPI::CB_T_RECV ||
			type == TCPSocketAPI::CB_T_ACCEPT);
}

/// Handles the acceptance of a new socket.
/// @param socket_id -- the descriptor for the listening socket
/// @param ret_status -- the status of the previously invoked accept method or
///		the descriptor for the accepted socket
/// @param yourPtr -- the pointer to the data/struct/object previously passed
///  	to accept
/// @details
/// On success: ret_status will be the descriptor of the accepted socket
/// On error: ret_status will be TCPSocketAPI::CB_E_UNKNOWN from the
/// 	TCPSocketAPI::CALLBACK_ERROR enumeration
void videoServer::acceptCallback(int socket_id, int ret_status, void * yourPtr) {

	// signal next accept
	tcp_api->accept(socket_id);

	switch(ret_status) {
		case TCPSocketAPI::CB_E_UNKNOWN:
			opp_error("videoServer::acceptCallback: unknown error.");
			break;
		default: // read data from new socket
			tcp_api->recv(ret_status, NULL);
			break;
	}
}

// @param socket_id -- the id of the accepted socket
// @param ret_status -- the status of the previously invoked recv method or
//						the number of bytes received
// @param msg -- a pointer to the received message
// @param yourPtr -- the pointer to the data passed to the accept method
void videoServer::recvCallback(int socket_id, int ret_status,
	cPacket * msg, void * myPtr){


	switch(ret_status) {
	case TCPSocketAPI::CB_E_TIMEOUT:
		handleTimeout(socket_id);
		break;
	case TCPSocketAPI::CB_E_UNKNOWN:
		// do nothing special
		break;
	case TCPSocketAPI::CB_E_CLOSED:
		closeSocket(socket_id);
		break;
	default: // positive # of bytes received
		processDownstreamRequest(socket_id, msg);
		break;
	}
}
// A client (or cache) requests a file from me.
// If I have it, send it along.  Otherwise, initiate a request from an upstream host.
void videoServer::processDownstreamRequest(int socket_id, cPacket * msg) {

	httptRequestMessage * request = dynamic_cast<httptRequestMessage *>(msg);
	if (!request) {
		opp_error("videoServer::processDownstreamRequest: msg not a httptRequest.");
	}
	requestsReceived++;

	// call the message handler to process the message.
	cMessage *reply = handleReceivedMessage(msg);
	if (reply)
	{
		tcp_api->send(socket_id, reply);
	} else {
		opp_error("videoServer::processDownstreamRequest: handleReceivedMessage returns NULL");
	}

	// ask client for anything else it might send:
	tcp_api->recv(socket_id,NULL);
	updateDisplay(); // draw.
	delete request;
	delete wr;
}

void videoServer::updateDisplay() {
	httptServerBase::updateDisplay();
	if ( ev.isGUI() )
	{
		char buf[1024];
		sprintf( buf, "Req: %ld", requestsReceived );
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	}
}


//void httptServer::socketEstablished(int connId, void *yourPtr)
//{
//    EV_INFO << "connected socket with id=" << connId << endl;
//	socketsOpened++;
//}
//
//void httptServer::socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent)
//{
//	if ( yourPtr==NULL )
//	{
//		EV_ERROR << "Socket establish failure. Null pointer" << endl;
//		return;
//	}
//	TCPSocket *socket = (TCPSocket*)yourPtr;
//
//	// Should be a httptReplyMessage
//	EV_DEBUG << "Socket data arrived on connection " << connId << ". Message=" << msg->getName() << ", kind=" << msg->getKind() << endl;
//
//	// call the message handler to process the message.
//	cMessage *reply = handleReceivedMessage(msg);
//	cPacket *pckt = check_and_cast<cPacket *>(msg);
//	if ( reply!=NULL )
//	{
//		requestsReceived++;
//		socket->send(reply); // Send to socket if the reply is non-zero.
//	}
//	delete msg; // Delete the received message here. Must not be deleted in the handler!
//}
//
//void httptServer::socketPeerClosed(int connId, void *yourPtr)
//{
//	if ( yourPtr==NULL )
//	{
//		EV_ERROR << "Socket establish failure. Null pointer" << endl;
//		return;
//	}
//	TCPSocket *socket = (TCPSocket*)yourPtr;
//
//    // close the connection (if not already closed)
//    if (socket->getState()==TCPSocket::PEER_CLOSED)
//    {
//        EV_INFO << "remote TCP closed, closing here as well. Connection id is " << connId << endl;
//        socket->close();  // Call the close method to properly dispose of the socket.
//    }
//}
//
//void httptServer::socketClosed(int connId, void *yourPtr)
//{
//    EV_INFO << "connection closed. Connection id " << connId << endl;
//
//	if ( yourPtr==NULL )
//	{
//		EV_ERROR << "Socket establish failure. Null pointer" << endl;
//		return;
//	}
//	// Cleanup
//	TCPSocket *socket = (TCPSocket*)yourPtr;
//	sockCollection.removeSocket(socket);
//	delete socket;
//}
//
//void httptServer::socketFailure(int connId, void *yourPtr, int code)
//{
//    EV_WARNING << "connection broken. Conneciton id " << connId << endl;
//    numBroken++;
//
//    EV_INFO << "connection closed. Connection id " << connId << endl;
//
//	if ( yourPtr==NULL )
//	{
//		EV_ERROR << "Socket establish failure. Null pointer" << endl;
//		return;
//	}
//	TCPSocket *socket = (TCPSocket*)yourPtr;
//
//	if (code==TCP_I_CONNECTION_RESET)
//		EV_WARNING << "Connection reset!\\n";
//	else if (code==TCP_I_CONNECTION_REFUSED)
//		EV_WARNING << "Connection refused!\\n";
//
//	// Cleanup
//	sockCollection.removeSocket(socket);
//	delete socket;
//}
