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
using namespace std;

typedef pair<int,string> Request;

class uri_named
{
private:
	string uri;
public:
	uri_named(string uri): uri(uri){}
  bool operator() (const Request & value) {return (value.second == uri);}
};

class CacheRequestMgr {
private:
	list<Request> requests;

public:
	CacheRequestMgr();
	virtual ~CacheRequestMgr();
	list<int> clientsAskingForResource(string);
	void removeRequestsForResource(string);
	bool addRequest(int,string);
};

#endif /* CACHEREQUESTMGR_H_ */
