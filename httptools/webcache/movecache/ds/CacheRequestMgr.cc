/*
 * RequestManager.cpp
 *
 *  Created on: Jun 3, 2010
 *      Author: travis
 */

#include "CacheRequestMgr.h"

CacheRequestMgr::CacheRequestMgr() {
}

CacheRequestMgr::~CacheRequestMgr() {
}

list<int> CacheRequestMgr::clientsAskingForResource(string uri) {
	list<int> clients;
	list<Request>::iterator it;
	for (it = requests.begin();it != requests.end(); it++) {
		if ((*it).second == uri) {
			int s = (*it).first;
			clients.push_back(s);
		}
	}
	return clients;
}
void CacheRequestMgr::removeRequestsForResource(string uri) {
	requests.remove_if(uri_named(uri));
}
bool CacheRequestMgr::addRequest(int socket_id,string uri) {
	// first, search for an occurrence of the uri
	bool found = false;
	list<Request>::iterator it;
		for (it = requests.begin();it != requests.end(); it++) {
			if ((*it).second == uri) {
				found = true;
				break;
			}
		}
	requests.push_back(pair<int,string>(socket_id,uri));
	return !found;
}
