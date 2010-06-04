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

#ifndef WEBCACHENEWAPI_H_
#define WEBCACHENEWAPI_H_
#include <stdlib.h>
#include <map>
#include <set>
#include <vector>
#include <list>
#include "TCPSocketAPI.h"
#include "httptServerBase.h"
#include "LRUCache.h"
#include "IPAddressResolver.h"
#include "WebResource.h"
#include "CacheRequestMgr.h"

enum CacheMessageType { START = 1 };
enum SockType{SERVER,CLIENT,LISTENER};
struct ConnInfo
{
	SockType sockType; // type of socket (server means socket where cache acts as a server, client means socket where cache acts as client)
	httptRequestMessage * ds_request; // a ptr to the request message made by a client, useful when connecting for the purpose of requesting a file.
};

class WebCacheNewAPI:
	public httptServerBase, TCPSocketAPI::CallbackInterface {

public:
	WebCacheNewAPI();
	virtual ~WebCacheNewAPI();

protected:
	// internals
	httptController *controller; // Reference to central controller object.
	Cache * resourceCache;
	TCPSocketAPI * tcp_api;
	string upstream_cache;
	int request_timeout;
	CacheRequestMgr pendingRequests;

	// stats
	unsigned long numBroken;
	unsigned long socketsOpened;
	unsigned long serverSocketsBroken;
	unsigned long serverSocketsOpened;
	unsigned long clientSocketsBroken;
	unsigned long clientSocketsOpened;
	int hits;
	int misses;

	virtual void updateDisplay(); //> Update the display string if running in GUI mode

	string extractURLFromRequest(httptRequestMessage * request);
	string extractURLFromResponse(httptReplyMessage * response);

	// cModule methods
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);

	// TCPSocketAPI::CallbackInterface functions
	virtual bool hasCallback(TCPSocketAPI::CALLBACK_TYPE type); // is this in CB interface?
	virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr); // this happens after each call  to accept.
	virtual void connectCallback(int socket_id, int ret_status, void * myPtr); // this happens after each call to connect
	virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr); // this happens after each call to recv

	// cache's methods
	virtual void makeUpstreamRequest(int socket_id, ConnInfo * data); // cache send req. to upper-level cache or server.
	virtual void processUpstreamResponse(int socket_id, cPacket * msg, ConnInfo * data); // get response from upper-level server.
	virtual void processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data); // handle request from client (or cache)
	virtual int openUpstreamSocket(ConnInfo *data);
	virtual void handleTimeout(int socket_id);
	virtual void closeSocket(int socket_id);

	bool isErrorMessage(httptReplyMessage *msg);

};

#endif /* WEBCACHENEWAPI_H_ */