/*
 * CacheRequestMgr.h
 *
 *  Created on: Jun 3, 2010
 *      Author: travis
 */

#ifndef CACHEREQUESTMGR_H_
#define CACHEREQUESTMGR_H_
#include <list>
#include <map>
#include <string>
#include <iostream>
#include "httptMessages_m.h"
using namespace std;

//typedef pair<int,string> RequestRecord;

struct RequestRecord
{
	int interface_id;
	string resource_id;
	httptRequestMessage * request_msg_ptr;

	RequestRecord(int interface, string resouce, httptRequestMessage * request)
	{
		interface_id = interface;
		resource_id = resouce;
		request_msg_ptr = request;
	}
};

class uri_named
{
private:
	string uri;
public:
	uri_named(string uri): uri(uri){}
  bool operator() (const RequestRecord & value) { return (value.resource_id == uri); }
  //{return (value.second == uri);}
};

class CacheRequestMgr {
private:
	list<RequestRecord> requests;

public:
	CacheRequestMgr();
	virtual ~CacheRequestMgr();
	list<int> clientsAskingForResource(string);
	int numberOfClientsAskingForResource(string);
	list<RequestRecord> getRequestsForResource(string);
	void removeRequestsForResource(string);
	void removeAndDeleteRequestsForResource(string);
	bool addRequest(int,string, httptRequestMessage *);
	void removeAndDeleteAllRequestsOnInterface(int);
};

#endif /* CACHEREQUESTMGR_H_ */
