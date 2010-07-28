/*
 * ByteBufferNode.cc
 *
 *  Created on: Jul 26, 2010
 *      Author: black
 */

#include "ByteBufferNode.h"

void setMode(ByteBufferNodeMode & mode, std::string param)
{
	if (param == BBN_MODE_NAME_WHOLE)
	{
		mode = BBN_MODE_WHOLE;
	}
	else if (param == BBN_MODE_NAME_FRAGMENTS)
	{
		mode = BBN_MODE_FRAGMENTS;
	}
	else
	{
		throw cRuntimeError("The parameter is not a known byte buffer node mode.");
	}
}
