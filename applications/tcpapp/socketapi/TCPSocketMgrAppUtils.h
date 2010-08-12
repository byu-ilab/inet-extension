/**
 * \file TCPSocketMgrAppUtils.h
 *
 * Created: June 7, 2010
 * Author: Kevin Black
 */

#ifndef __INET__TCPSOCKETMGRAPPUTILS_H_
#define __INET__TCPSOCKETMGRAPPUTILS_H_

#include <omnetpp.h>
#include "TCPSocketMgr.h"

/**
 * Finds the TCPSocketMgr module given the parameter socketapi's
 * value in the given module.
 *
 * @param app_module -- A pointer to the module to get the TCPSocketMgr's
 * 		module name from.
 *
 * @throw Throws a cRuntimeError if the module cannot be found.
 */
TCPSocketMgr * findTCPSocketMgr(cSimpleModule * app_module);

#endif /* __INET__TCPSOCKETMGRAPPUTILS_H_ */
