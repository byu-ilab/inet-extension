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

#ifndef WEBCACHE_H_
#define WEBCACHE_H_
#include "httptServer.h"
#include "LRUCache.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "IPAddressResolver.h"
#include "WebResource.h"
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
enum SockType{SERVER,CLIENT,LISTENER};
struct CACHE_SOCK_DS
{
	SockType sockType; // type of socket (server means socket where cache acts as a server, client means socket where cache acts as client)
	TCPSocket *socket;					// A reference to the socket object.
	MESSAGE_QUEUE_TYPE messageQueue; 	//> Queue of pending messages.
	int pendingReplies;						// A counter for the number of outstanding replies.
};
//struct setcomp {
//		bool operator() (TCPSocket * lhs, TCPSocket * rhs ) const {
//			return lhs->getConnectionId() < rhs->getConnectionId();
//		}
//};
class WebCache: public httptServer {
protected:
	httptController *controller; // Reference to central controller object.
	TCPSocket * listensocket;
	TCPSocketMap socketCollection;
  typedef std::map<TCPSocket*,cPacket*> ClientMap;
  ClientMap pendingRequests;
	unsigned long serverSocketsBroken;
	unsigned long serverSocketsOpened;
	unsigned long clientSocketsBroken;
	unsigned long clientSocketsOpened;
	Cache * resourceCache;
	int hits;
	int misses;
	typedef std::set<int> TSet;
	TSet serverConnections;  // Distinguish a server socket from a client socket

	virtual void updateDisplay(); //> Update the display string if running in GUI mode
public:
	WebCache();
	virtual ~WebCache();
private:
	string extractURLFromRequest(httptRequestMessage * request);
	string extractURLFromResponse(httptReplyMessage * response);

	// cModule methods
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);

	// socket API methods
	virtual void socketEstablished(int connId, void *yourPtr);
	virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
	virtual void socketPeerClosed(int connId, void *yourPtr);
	virtual void socketClosed(int connId, void *yourPtr);
	virtual void socketFailure(int connId, void *yourPtr, int code);
	virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);
	// server methods
  bool serverHasResource(cPacket * msg);
	bool meantForServer(cMessage * msg);
	// void sendResource(); Not sure on this yet.
	// void receiveRequest(int connId, cPacket * msg); // or on this
	bool isErrorMessage(httptReplyMessage *);
	// client methods
  httptRequestMessage * generateServerRequest(cPacket * msg);
	TCPSocket * sendRequest(httptRequestMessage * request);
	void receiveResource(cPacket * msg);

	// pipelining via client
	TCPSocket * submitToSocket( const char* moduleName, int connectPort, cMessage *msg );
  TCPSocket * submitToSocket( const char* moduleName, int connectPort, MESSAGE_QUEUE_TYPE &queue );

};

#endif /* WEBCACHE_H_ */
