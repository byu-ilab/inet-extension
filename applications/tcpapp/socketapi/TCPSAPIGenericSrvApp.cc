// Based on the TCPGenericSrvApp by Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPSAPIGenericSrvApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"
#include "GenericAppMsg_m.h"

Define_Module(TCPSAPIGenericSrvApp);

void TCPSAPIGenericSrvApp::initialize()
{
	EV << "parameter parsing...";
    const char *address = par("address");
    int port = par("port");
    delay = par("replyDelay");
    maxMsgDelay = 0;

	std::string api_obj_name = par("socketapi").stringValue();
    if (api_obj_name.empty())
    	opp_error("TCPSAPIGenericSrvApp::initialize(): no tcp socket api specified!");

    socketapi = check_and_cast<TCPSocketAPI *>(getParentModule()->getSubmodule(api_obj_name.c_str()));

    EV << "vector/scalar initialization...";
    msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;
    WATCH(msgsRcvd);
    WATCH(msgsSent);
    WATCH(bytesRcvd);
    WATCH(bytesSent);

//    mymsg = new cMessage();
//    scheduleAt(0.0, mymsg);

    EV << "passive socket creation...";
    int s_id = socketapi->socket(this);
    socketapi->bind(s_id, "", /*address[0] ? IPvXAddress(address) : IPvXAddress(),*/ port);
    socketapi->listen(s_id);
    socketapi->accept(s_id, NULL);//, this);//&CallbackInterface::accept_connection);
    /*TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
    */
}

//void TCPSAPIGenericSrvApp::sendOrSchedule(cMessage *msg, simtime_t delay)
//{
//    if (delay==0)
//        sendBack(msg);
//    else
//        scheduleAt(simTime()+delay, msg);
//}
//
//void TCPSAPIGenericSrvApp::sendBack(cMessage *msg)
//{
//	GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg*>(msg);
//
//	if (appmsg)
//	{
//		msgsSent++;
//		bytesSent += appmsg->getByteLength();
//
//		EV << "sending \"" << appmsg->getName() << "\" to TCP, " << appmsg->getByteLength() << " bytes\n";
//	}
//	else
//	{
//		EV << "sending \"" << msg->getName() << "\" to TCP\n";
//	}
//
//    //send(msg, "tcpOut");
//	socketapi->send( , msg);
//}



void TCPSAPIGenericSrvApp::handleMessage(cMessage *msg)
{
//	if (msg->isSelfMessage())
//	{
//		EV << "passive socket creation...";
//		int s_id = socketapi->socket();
//		    socketapi->bind(s_id, "", /*address[0] ? IPvXAddress(address) : IPvXAddress(),*/ port);
//		    socketapi->listen(s_id);
//		    socketapi->accept(s_id, NULL, this);
//	}
	EV << "TCPSAPIGenericSrvApp::handleMessage(): recieved a message: " << msg->getClassName()
		<< ", " << msg->getName();
	delete msg;
}

bool TCPSAPIGenericSrvApp::hasCallback (TCPSocketAPI::CALLBACK_TYPE type) {
	if (type == TCPSocketAPI::CB_T_ACCEPT || type == TCPSocketAPI::CB_T_RECV)
		return true;
	else
		return false;
}

void TCPSAPIGenericSrvApp::acceptCallback(int socket_id, int ret_status, void * myPtr)
{
	socketapi->recv(ret_status, NULL);//, this);// &TCPSAPIGenericSrvApp::recv_data);
}

void TCPSAPIGenericSrvApp::recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr)
{
	if (ret_status == 0) {
		EV << "TCPSAPIGenericSrvApp::recv_data(): socket closed";
		socketapi->close(socket_id);
		return;
	}
	else if (ret_status == -1) {
		EV << "TCPSAPIGenericSrvApp::recv_data(): error occurred";
		return;
	}

	GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
	if (!appmsg)
		error("Message (%s)%s is not a GenericAppMsg -- "
				"probably wrong client app, or wrong setting of TCP's "
				"sendQueueClass/receiveQueueClass parameters "
				"(try \"TCPMsgBasedSendQueue\" and \"TCPMsgBasedRcvQueue\")",
				msg->getClassName(), msg->getName());

	msgsRcvd++;
	bytesRcvd += appmsg->getByteLength();

	long requestedBytes = appmsg->getExpectedReplyLength();

	simtime_t msgDelay = appmsg->getReplyDelay();
	if (msgDelay>maxMsgDelay)
		maxMsgDelay = msgDelay;

	bool doClose = appmsg->getServerClose();

	//int connId = check_and_cast<TCPCommand *>(appmsg->getControlInfo())->getConnId();

	if (requestedBytes==0)
	{
		delete msg;
	}
	else
	{
		//delete appmsg->removeControlInfo();
//		TCPSendCommand *cmd = new TCPSendCommand();
//		cmd->setConnId(connId);
//		appmsg->setControlInfo(cmd);

		// set length and send it back
		//appmsg->setKind(TCP_C_SEND);
		appmsg->setByteLength(requestedBytes);
		//sendOrSchedule(appmsg, delay+msgDelay);
		socketapi->send(socket_id, appmsg);
	}

	if (doClose)
	{
		//cMessage *msg = new cMessage("close");
//		msg->setKind(TCP_C_CLOSE);
//		TCPCommand *cmd = new TCPCommand();
//		cmd->setConnId(connId);
//		msg->setControlInfo(cmd);
		//sendOrSchedule(msg, delay+maxMsgDelay);
		socketapi->close(socket_id);//send(socket_id, msg);
	}
}

void TCPSAPIGenericSrvApp::finish()
{
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";

    recordScalar("packets sent", msgsSent);
    recordScalar("packets rcvd", msgsRcvd);
    recordScalar("bytes sent", bytesSent);
    recordScalar("bytes rcvd", bytesRcvd);
}

