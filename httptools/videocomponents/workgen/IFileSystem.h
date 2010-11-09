/*
 * IFileSystem.h
 *
 *  Created on: Sep 15, 2010
 *      Author: tester
 */

#ifndef IFILESYSTEM_H_
#define IFILESYSTEM_H_

#include <string>
#include "httptMessages_m.h"
using namespace std;
class IFileSystem {

public:

	virtual bool hasResource(string resource) = 0;
	virtual int getResourceSize(httptRequestMessage * request, string resource) = 0;

};

#endif /* IFILESYSTEM_H_ */
