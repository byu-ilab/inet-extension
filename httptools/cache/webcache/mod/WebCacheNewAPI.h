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
#include "simunits.h"
#include "WebContentExtensionFilter.h"
#include "DebugDef.h"
#include "DeleteSafeDefs.h"
#include "ActiveTCPSocketPool.h"

#define ANY_US_SOCKET -1
#define US_SOCK_NONE -1
#define US_SOCK_CONNECTING -2
// TODO or CONTINUE don't let requests be sent on the socket unless it is connected.
// perhaps the best thing to do would be to make a pending upstream request queue
// then when an upstream socket is connected it can get the first request from the
// queue and make the request, that changes what ConnInfo has to store

enum CacheMessageType { START = 1 };

enum WebCacheSockType{
	WCST_SERVER, 	//< the web cache uses the socket to serve content to downstream clients
	WCST_CLIENT, 	//< the web cache uses the socket to request content from upstream server
	WCST_LISTENER	//< the web cache uses the socket to listen on
};

struct ConnInfo
{
	WebCacheSockType sockType; //< indicates how the server uses the socket

	/// For a sockType of WebCacheSockType::WCST_SERVER the number of requests that have been
	/// sent whose response is still pending.
	int numPendingResponses;
};

class WebCacheNewAPI:
	public httptServerBase, TCPSocketAPI::CallbackInterface {

public:
	WebCacheNewAPI();
	virtual ~WebCacheNewAPI();

protected:
	// internals
	Cache * resourceCache;
	TCPSocketAPI * tcp_api;
	string upstream_server;
	int request_timeout;
	CacheRequestMgr pendingDownstreamRequests;

	ActiveTCPSocketPool * upstreamSocketPool;
	int resend_request_threshold;

//	cQueue pendingUpstreamRequests;
//	int socket_cap;
//	int cur_socket;
//	// maps socket descriptors to connection info
//	std::multiset<int, ConnInfo *> upstreamSocketDescriptors;
//	int target_load_for_us_socket;

	// stats
	uint64 requestsReceived;

	uint64 serverSocketsBroken;
	uint64 serverSocketsOpened;

	uint64 clientSocketsBroken;
	uint64 clientSocketsOpened;
	uint64 currentSocketsOpenToServer;

	uint64 hits;
	uint64 misses;

	WebContentExtensionFilter contentFilter;
	bool shouldFilter;

	map<int, ConnInfo *> socketConnInfoMap;

	// Overridden from cSimpleModule
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg);

	// TCPSocketAPI::CallbackInterface functions
	virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr); // this happens after each call  to accept.
//	virtual void connectCallback(int socket_id, int ret_status, void * myPtr); // this happens after each call to connect
	virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr); // this happens after each call to recv

	// overriden from httptServerBase
	virtual httptReplyMessage * handleGetRequest(httptRequestMessage * msg, string resource_url);
	virtual void updateDisplay(); //> Update the display string if running in GUI mode

	// cache's methods
	// socket_id is only a suggestion as to which upstream socket to request on
	// manages the load on the upstream sockets
	virtual void makeUpstreamRequest(/*int socket_id,*/ httptRequestMessage * ds_request_template=NULL); //ConnInfo * data); // cache send req. to upper-level cache or server.
	virtual void processUpstreamResponse(int socket_id, cPacket * msg, ConnInfo * data); // get response from upper-level server.
	virtual void processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data); // handle request from client (or cache)
//	virtual int  openUpstreamSocket(ConnInfo *data);
	//virtual void handleTimeout(int socket_id);
	virtual void closeSocket(int socket_id);
	virtual void respondToClientRequest(int socket_id, httptRequestMessage * request, Resource * resouce);

	bool isErrorMessage(httptReplyMessage *msg);
	string extractURLFromRequest(httptRequestMessage * request);
	string extractURLFromResponse(httptReplyMessage * response);

};

#endif /* WEBCACHENEWAPI_H_ */
