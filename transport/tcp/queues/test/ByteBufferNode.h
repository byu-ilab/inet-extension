/*
 * ByteBufferNode.h
 *
 *  Created on: Jul 26, 2010
 *      Author: Black
 */

#ifndef BYTEBUFFERNODE_H_
#define BYTEBUFFERNODE_H_

#include <omnetpp.h>
#include <string>

#define BBN_MODE_NAME_WHOLE "whole"
#define BBN_MODE_NAME_FRAGMENTS "fragments"

enum ByteBufferNodeMode { BBN_MODE_NOTSET, BBN_MODE_WHOLE, BBN_MODE_FRAGMENTS };

void setMode(ByteBufferNodeMode & mode, std::string param);

#endif /* BYTEBUFFERNODE_H_ */
