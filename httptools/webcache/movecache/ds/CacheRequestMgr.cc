/*
 * RequestManager.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: travis
 */

#include "CacheRequestMgr.h"
CacheRequestMgr::CacheRequestMgr() : requests() {
}

CacheRequestMgr::~CacheRequestMgr() {
}

list<int> CacheRequestMgr::clientsAskingForResource(string uri) {
	list<int> clients;
	list<RequestRecord>::iterator it;
	for (it = requests.begin();it != requests.end(); it++) {
//		if ((*it).second == uri) {
//			int s = (*it).first;
//			clients.push_back(s);
//		}
		if ((*it).resource_id == uri)
		{
			int s = (*it).interface_id;
			clients.push_back(s);
		}
	}
	return clients;
}

list<RequestRecord> CacheRequestMgr::getRequestsForResource(string uri)
{
	list<RequestRecord> resource_requests;
	list<RequestRecord>::iterator it;
	for (it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it).resource_id == uri)
		{
			resource_requests.push_back(*it);
		}
	}
	return resource_requests;
}

void CacheRequestMgr::removeRequestsForResource(string uri) {
	// remove them from requests
	requests.remove_if(uri_named(uri));
}

void CacheRequestMgr::removeAndDeleteRequestsForResource(string uri)
{
	list<RequestRecord>::iterator it;
	for (it = requests.begin(); it != requests.end(); it++)
	{
		if ((*it).resource_id == uri)
		{
			if ((*it).request_msg_ptr)
			{
				delete (*it).request_msg_ptr;
			}
			it = requests.erase(it);
		}
	}
}

bool CacheRequestMgr::addRequest(int socket_id, string uri, httptRequestMessage * request) {
	// first, search for an occurrence of the uri
	bool found = false;
	list<RequestRecord>::iterator it;
		for (it = requests.begin();it != requests.end(); it++) {
//			if ((*it).second == uri) {
//				found = true;
//				break;
//			}
			if ((*it).resource_id == uri)
			{
				found = true;
				break;
			}
		}
	requests.push_back(RequestRecord(socket_id, uri, request));//pair<int,string>(socket_id,uri));
	return !found;
}
