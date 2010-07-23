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
	map<string, list<RequestRecord> * >::iterator oit;
	for (oit = requests.begin(); oit != requests.end(); oit++) {
		list<RequestRecord> * rptr = (*oit).second;
		list<RequestRecord>::iterator it;
		for (it = rptr->begin(); it != rptr->end(); it++)
		{
			deleteSafe((*it).request_msg_ptr);
		}
		//oit = requests.erase(oit); //delete rptr;
	}
	requests.clear();
}

list<int> CacheRequestMgr::clientsAskingForResource(string uri) {
	list<int> clients;
	list<RequestRecord> * reqs =getRequestsForResource(uri);
	if (reqs) {
		list<RequestRecord>::iterator it;
		for(it == reqs->begin(); it != reqs->end(); it++) {
			clients.push_back((*it).interface_id);
		}
	}
	return clients;
//	for (it = requests.begin();it != requests.end(); it++) {
//		if ((*it).resource_id == uri)
//		{
//			int s = (*it).interface_id;
//			clients.push_back(s);
//		}
//	}
//	return clients;
}

int CacheRequestMgr::numberOfClientsAskingForResource(string uri)
{
	list<RequestRecord> * lptr = getRequestsForResource(uri);
	if (lptr) {
		return lptr->size();
	}
	return 0;
}

list<RequestRecord> * CacheRequestMgr::getRequestsForResource(string uri)
{
	list<RequestRecord> *  reqs = NULL;
	map<string, list<RequestRecord> * >::iterator res = requests.find(uri);
	if (res != requests.end()) {
		reqs = res->second;
	}
	return reqs;

//	list<RequestRecord> resource_requests;
//	list<RequestRecord>::iterator it;
//	for (it = requests.begin(); it != requests.end(); it++)
//	{
//		if ((*it).resource_id == uri)
//		{
//			resource_requests.push_back(*it);
//		}
//	}
//	return resource_requests;
}

void CacheRequestMgr::removeRequestsForResource(string uri) {
	// remove them from requests
	list<RequestRecord> * reqs = NULL;
	map<string, list<RequestRecord>* >::iterator res = requests.find(uri);
	if (res == requests.end()) {
		return;
	}
	reqs = (*res).second;
	delete reqs;
	requests.erase(res);
	//requests.remove_if(uri_named(uri));
}

void CacheRequestMgr::removeAndDeleteRequestsForResource(string uri)
{
	list<RequestRecord> * reqs;
	map<string, list<RequestRecord>* >::iterator res = requests.find(uri);
	if (res == requests.end()) {
		return;
	}
	reqs = res->second;
	list<RequestRecord>::iterator it;
	for (it = reqs->begin(); it != reqs->end(); it++)
	{
		if ((*it).resource_id == uri)
		{
			if ((*it).request_msg_ptr)
			{
				delete (*it).request_msg_ptr;
			}
			//it = requests.erase(it);
		}
	}
	reqs->clear();
	delete reqs;
	requests.erase(res);
}

bool CacheRequestMgr::addRequest(int socket_id, string uri, httptRequestMessage * request) {
	// first, search for an occurrence of the uri
	bool found = false;
	map<string, list<RequestRecord>* >::iterator res = requests.find(uri);
	if (res != requests.end()) {
		found = true;
		list<RequestRecord> * reqs = res->second;
		reqs->push_back(RequestRecord(socket_id, uri, request));
	} else { // first time ever with this request
		list<RequestRecord> * reqs = new list<RequestRecord>;
		reqs->push_back(RequestRecord(socket_id, uri, request));
		requests.insert(pair<string,list<RequestRecord>* >(uri, reqs));
	}
//	requests.push_back(RequestRecord(socket_id, uri, request));
	return !found;
}

void CacheRequestMgr::removeAndDeleteAllRequestsOnInterface(int interface_id)
{
	map<string, list<RequestRecord>* >::iterator oit;
	for (oit = requests.begin(); oit != requests.end(); oit++) {
		list<RequestRecord>::iterator it;
		for (it = (oit->second)->begin(); it != (oit->second)->end(); it++)
		{
			if ((*it).interface_id == interface_id)
			{
				if ((*it).request_msg_ptr)
				{
					delete (*it).request_msg_ptr;
				}
				it = (oit->second)->erase(it);
			}
		}
//		if (oit->second->size() == 0) {
//			oit = requests.erase(oit);
//		}
	}
}
