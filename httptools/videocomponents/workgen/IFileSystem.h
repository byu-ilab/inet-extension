/*
 * IFileSystem.h
 *
 *  Created on: Sep 15, 2010
 *      Author: tester
 */

#ifndef IFILESYSTEM_H_
#define IFILESYSTEM_H_

#include <string>

using namespace std;

class IFileSystem {

public:

	virtual bool hasResource(string resource) = 0;
	virtual int getResourceSize(string resource) = 0;

};

#endif /* IFILESYSTEM_H_ */
