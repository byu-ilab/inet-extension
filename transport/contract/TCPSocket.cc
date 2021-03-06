//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "TCPSocket.h"
#include <sstream>
#include <iostream>
TCPSocket::TCPSocket()
{
    // don't allow user-specified connIds because they may conflict with
    // automatically assigned ones.
    connId = ev.getUniqueNumber();
    sockstate = NOT_BOUND;

    localPrt = remotePrt = -1;
    cb = NULL;
    yourPtr = NULL;

    gateToTcp = NULL;
}

TCPSocket::TCPSocket(cMessage *msg)
{
    TCPCommand *ind = dynamic_cast<TCPCommand *>(msg->getControlInfo());
    if (!ind)
        opp_error("TCPSocket::TCPSocket(cMessage *): no TCPCommand control info in message (not from TCP?)");

    connId = ind->getConnId();
    sockstate = CONNECTED;

    localPrt = remotePrt = -1;
    cb = NULL;
    yourPtr = NULL;

    gateToTcp = NULL;

    if (msg->getKind()==TCP_I_ESTABLISHED)
    {
        // management of stockstate is left to processMessage() so we always
        // set it to CONNECTED in the ctor, whatever TCP_I_xxx arrives.
        // However, for convenience we extract TCPConnectInfo already here, so that
        // remote address/port can be read already after the ctor call.

        TCPConnectInfo *connectInfo = dynamic_cast<TCPConnectInfo *>(msg->getControlInfo());
        localAddr = connectInfo->getLocalAddr();
        remoteAddr = connectInfo->getRemoteAddr();
        localPrt = connectInfo->getLocalPort();
        remotePrt = connectInfo->getRemotePort();
    }
}

const char *TCPSocket::stateName(int state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state)
    {
        CASE(NOT_BOUND);
        CASE(BOUND);
        CASE(LISTENING);
        /* +++> */ CASE(ACCEPTING); /* <+++ */
        CASE(CONNECTING);
        CASE(CONNECTED);
        /* +++> */ CASE(RECEIVING); /* <+++ */
        CASE(PEER_CLOSED);
        CASE(LOCALLY_CLOSED);
        CASE(CLOSED);
        CASE(SOCKERROR);
    }
    return s;
#undef CASE
}

void TCPSocket::sendToTCP(cMessage *msg)
{
    if (!gateToTcp)
        opp_error("TCPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToTcp->getOwnerModule())->send(msg, gateToTcp);
}

void TCPSocket::bind(int lPort)
{
    if (sockstate!=NOT_BOUND)
        opp_error("TCPSocket::bind(): socket already bound");
    if (lPort<0 || lPort>65535)
        opp_error("TCPSocket::bind(): invalid port number %d", lPort);

    localPrt = lPort;
    sockstate = BOUND;
}

void TCPSocket::bind(IPvXAddress lAddr, int lPort)
{
    if (sockstate!=NOT_BOUND)
        opp_error("TCPSocket::bind(): socket already bound");
    // allow -1 here, to make it possible to specify address only
    if ((lPort<0 || lPort>65535) && lPort!=-1)
        opp_error("TCPSocket::bind(): invalid port number %d", lPort);

    localAddr = lAddr;
    localPrt = lPort;
    sockstate = BOUND;
}

void TCPSocket::listen(bool fork)
{
    if (sockstate!=BOUND)
        opp_error(sockstate==NOT_BOUND ? "TCPSocket: must call bind() before listen()"
                                       : "TCPSocket::listen(): connect() or listen() already called");

    cMessage *msg = new cMessage("PassiveOPEN", TCP_C_OPEN_PASSIVE);

    TCPOpenCommand *openCmd = new TCPOpenCommand();
    openCmd->setLocalAddr(localAddr);
    openCmd->setLocalPort(localPrt);
    openCmd->setConnId(connId);
    openCmd->setFork(fork);
    openCmd->setSendQueueClass(sendQueueClass.c_str());
    openCmd->setReceiveQueueClass(receiveQueueClass.c_str());
    openCmd->setTcpAlgorithmClass(tcpAlgorithmClass.c_str());

    msg->setControlInfo(openCmd);
    sendToTCP(msg);
    sockstate = LISTENING;
}

void TCPSocket::connect(IPvXAddress remoteAddress, int remotePort)
{
    if (sockstate!=NOT_BOUND && sockstate!=BOUND)
        opp_error( "TCPSocket::connect(): connect() or listen() already called (need renewSocket()?)");
    if (remotePort<0 || remotePort>65535)
        opp_error("TCPSocket::connect(): invalid remote port number %d", remotePort);

    cMessage *msg = new cMessage("ActiveOPEN", TCP_C_OPEN_ACTIVE);

    remoteAddr = remoteAddress;
    remotePrt = remotePort;

    TCPOpenCommand *openCmd = new TCPOpenCommand();
    openCmd->setConnId(connId);
    openCmd->setLocalAddr(localAddr);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setSendQueueClass(sendQueueClass.c_str());
    openCmd->setReceiveQueueClass(receiveQueueClass.c_str());
    openCmd->setTcpAlgorithmClass(tcpAlgorithmClass.c_str());

    msg->setControlInfo(openCmd);
    sendToTCP(msg);
    sockstate = CONNECTING;
}

void TCPSocket::send(cMessage *msg)
{
    if (sockstate!=CONNECTED && /* +++> */ sockstate != RECEIVING && /* <+++ */
    		sockstate!=CONNECTING && sockstate!=PEER_CLOSED)
        opp_error("TCPSocket::send(): not connected or connecting");

    msg->setKind(TCP_C_SEND);
    TCPSendCommand *cmd = new TCPSendCommand();
    cmd->setConnId(connId);
    msg->setControlInfo(cmd);
    sendToTCP(msg);
}

void TCPSocket::close()
{
    if (sockstate!=CONNECTED /* +++> */ && sockstate != RECEIVING && sockstate != ACCEPTING /* <+++ */
    		&& sockstate!=PEER_CLOSED && sockstate!=CONNECTING && sockstate!=LISTENING)
        opp_error("TCPSocket::close(): not connected or close() already called");
    cMessage *msg = new cMessage("CLOSE", TCP_C_CLOSE);
    TCPCommand *cmd = new TCPCommand();
    cmd->setConnId(connId);
    msg->setControlInfo(cmd);
    sendToTCP(msg);
    sockstate = (sockstate==CONNECTED /* +++> */ || RECEIVING /* <+++ */) ? LOCALLY_CLOSED : CLOSED;
}

void TCPSocket::abort()
{
    if (sockstate!=NOT_BOUND && sockstate!=BOUND && sockstate!=CLOSED && sockstate!=SOCKERROR)
    {
        cMessage *msg = new cMessage("ABORT", TCP_C_ABORT);
        TCPCommand *cmd = new TCPCommand();
        cmd->setConnId(connId);
        msg->setControlInfo(cmd);
        sendToTCP(msg);
    }
    sockstate = CLOSED;
}

void TCPSocket::requestStatus()
{
    cMessage *msg = new cMessage("STATUS", TCP_C_STATUS);
    TCPCommand *cmd = new TCPCommand();
    cmd->setConnId(connId);
    msg->setControlInfo(cmd);
    sendToTCP(msg);
}

void TCPSocket::renewSocket()
{
    connId = ev.getUniqueNumber();
    remoteAddr = localAddr = IPvXAddress();
    remotePrt = localPrt = -1;

    sockstate = NOT_BOUND;
}

bool TCPSocket::belongsToSocket(cMessage *msg)
{
    return dynamic_cast<TCPCommand *>(msg->getControlInfo()) &&
           ((TCPCommand *)(msg->getControlInfo()))->getConnId()==connId;
}

bool TCPSocket::belongsToAnyTCPSocket(cMessage *msg)
{
    return dynamic_cast<TCPCommand *>(msg->getControlInfo());
}

void TCPSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void TCPSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    TCPStatusInfo *status;
    TCPConnectInfo *connectInfo;
    switch (msg->getKind())
    {
        case TCP_I_DATA:
             if (cb)
                 cb->socketDataArrived(connId, yourPtr, PK(msg), false);
             else
                 delete msg;
             break;
        case TCP_I_URGENT_DATA:
             if (cb)
                 cb->socketDataArrived(connId, yourPtr, PK(msg), true);
             else
                 delete msg;
             break;
        case TCP_I_ESTABLISHED:
             // Note: this code is only for sockets doing active open, and nonforking
             // listening sockets. For a forking listening sockets, TCP_I_ESTABLISHED
             // carries a new connId which won't match the connId of this TCPSocket,
             // so you won't get here. Rather, when you see TCP_I_ESTABLISHED, you'll
             // want to create a new TCPSocket object via new TCPSocket(msg).
             sockstate = CONNECTED;
             connectInfo = dynamic_cast<TCPConnectInfo *>(msg->getControlInfo());
             localAddr = connectInfo->getLocalAddr();
             remoteAddr = connectInfo->getRemoteAddr();
             localPrt = connectInfo->getLocalPort();
             remotePrt = connectInfo->getRemotePort();
             delete msg;
             if (cb)
                 cb->socketEstablished(connId, yourPtr);
             break;
        case TCP_I_PEER_CLOSED:
             sockstate = sockstate==CONNECTED ? PEER_CLOSED : CLOSED;
             delete msg;
             if (cb)
                 cb->socketPeerClosed(connId, yourPtr);
             break;
        case TCP_I_CLOSED:
             sockstate = CLOSED;
             delete msg;
             if (cb)
                 cb->socketClosed(connId, yourPtr);
             break;
        case TCP_I_CONNECTION_REFUSED:
        case TCP_I_CONNECTION_RESET:
        case TCP_I_TIMED_OUT:
             sockstate = SOCKERROR;
             if (cb)
                 cb->socketFailure(connId, yourPtr, msg->getKind());
             delete msg;
             break;
        case TCP_I_STATUS:
             status = check_and_cast<TCPStatusInfo *>(msg->removeControlInfo());
             delete msg;
             if (cb)
                 cb->socketStatusArrived(connId, yourPtr, status);
             break;
        default:
             opp_error("TCPSocket: invalid msg kind %d, one of the TCP_I_xxx constants expected", msg->getKind());
    }
}

std::string TCPSocket::toString(bool full_address) const
{
	std::stringstream info;
	info << "id: "<< connId << " state: "<<stateName(sockstate)<<" address: ";

	switch((full_address) ? -1 : sockstate ) // trigger default if full_address is true
	{
	case NOT_BOUND:
		info << " not specified";
		break;
	case BOUND:
	case LISTENING:
		info << localAddr.str() << ":" << localPrt;
		break;
	default:
		// includes CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED
		// CLOSED, SOCKERROR
		info << localAddr.str() << ":" << localPrt << " to "<<
		remoteAddr.str() << ":"<<remotePrt;
	}

	return info.str();
}
