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

#include "VideoServer.h"

Define_Module(VideoServer);

void VideoServer::initialize()
{
	httptServerBase::initialize();

	workload_generator = dynamic_cast<VideoTitleWorkloadGenerator*>(simulation.getSystemModule()->getSubmodule("vtmdWorkloadGenerator"));
	//workload_generator = dynamic_cast<VMDWorkloadGenerator*>(simulation.getSystemModule()->getSubmodule(par("workloadGenerator")));
		// alternate: getParentModule()->getParentModule()->getSubmodule("workloadGenerator"));
	if (!workload_generator) {
		error("workload generator module not found");
	}
	socketsBroken=0;
	socketsOpened=0;
	requestsReceived = 0;
	responsesSent=0;
	updateDisplay();

	// get socket api
	tcp_api = findTCPSocketMgr(this);

	shouldTrackDupHttpMsgNames = par("shouldTrackDuplicateMessageNames");
	if (shouldTrackDupHttpMsgNames)
	{
		DuplicateHttpMessageNameObserver::getInstance()->subscribeOnDefaultSignal(this);
	}

    cMessage * start = new cMessage("START",START);
    scheduleAt(simTime(),start);
}

void VideoServer::finish()
{
	httptServerBase::finish();

	EV_SUMMARY << "Sockets opened: " << socketsOpened << endl;
	EV_SUMMARY << "Broken connections: " << socketsBroken << endl;

	recordScalar("sock.opened", socketsOpened);
	recordScalar("sock.broken", socketsBroken);
}

void VideoServer::handleMessage(cMessage *msg)
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
void VideoServer::acceptCallback(int socket_id, int ret_status, void * yourPtr) {

	// signal next accept
	tcp_api->accept(socket_id);

	switch(ret_status) {
		case TCPSocketAPI_Inet::CB_E_UNKNOWN:
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
void VideoServer::recvCallback(int socket_id, int ret_status,
	cPacket * msg, void * myPtr){

	Enter_Method_Silent();

	if (TCPSocketAPI_Inet::isCallbackError(ret_status))
	{
		// msg is NULL so don't call delete
		closeSocket(socket_id);
		return;
	}

	// handleReceivedMessage will return an error reply if there is a problem with the
	// message, otherwise control will get passed to handleGetRequest which will return
	// NULL

	// log before deleting the message
	emitMessageEvent(msg, socket_id);

	httptReplyMessage * response = handleRequestMessage(msg);

	// log before losing ownership of the response
	emitMessageEvent(response, socket_id);

	tcp_api->send(socket_id, response);
	delete msg;
	tcp_api->recv(socket_id);

	requestsReceived++;
	updateDisplay();
}

void VideoServer::closeSocket(int socket_id) {
	tcp_api->close(socket_id);
}
httptReplyMessage* VideoServer::handleGetRequest( httptRequestMessage *request, string resource_uri ) {
	// error if this resource is not a video title in workload generator
//	VideoSegmentRequestMessage * vrequest = static_cast<VideoSegmentRequestMessage *>(request);
//	if (!vrequest) {
//		opp_error("VideoServer::handleGetRequest: did not receive VideoSegmentRequestMessage");
//	}

	VideoSegmentMetaData vsmd(resource_uri);

	VideoTitleMetaData vtmd =  workload_generator->getVTMD(vsmd.video_title);

	if (!vsmd.pertainsTo(vtmd)) //workload_generator->isVideoSegmentDataValid(vsmd))
	{
		return generateErrorReply(request, resource_uri, 404);
	}

	int res_size = vtmd.quality_interval * vsmd.quality_level;

//	int res_type = rt_vidseg;

	// if it is a byte range request, service it.
//	VideoSegmentReplyMessage * reply = new VideoSegmentReplyMessage();
//
//	reply->setTitle(vrequest->getTitle());
//	reply->setType(vrequest->getType());
//	reply->setSegmentNumber(vrequest->getSegmentNumber());
//	reply->setQualityLevel(vrequest->getQualityLevel());
	responsesSent++;
	httptReplyMessage * rep = generateByteRangeReply(request, resource_uri, res_size, rt_vidseg);
	//cout<<"BR Rep Size: "<<rep->getByteLength()<<endl;
	return rep;

//	if (vrequest->firstBytePos() != BRS_UNSPECIFIED) {
//		fillinByteRangeReply(reply, vrequest, resource, res_size, res_type);
//	}
//	else { // service normal request
//		fillinStandardReply(reply, vrequest, resource, HTTP_CODE_200, res_size, res_type);
//	}
//	return reply;
}

void VideoServer::updateDisplay() {
	httptServerBase::updateDisplay();
	if ( ev.isGUI() )
	{
		char buf[1024];
		sprintf( buf, "R/S: %ld/%ld",requestsReceived,responsesSent );
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
	}
}

void VideoServer::emitMessageEvent(const cMessage * msg, const int & id)
{
	if (shouldTrackDupHttpMsgNames)
	{
		cMessageEventDatagram d(msg, id);
		emit(DuplicateHttpMessageNameObserver::getInstance()->getDefaultSignalID(), &d);
	}
}

