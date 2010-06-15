/*
 * DebugDef.h
 *
 *  Created on: Jun 14, 2010
 *      Author: Kevin Black
 */

#ifndef DEBUGDEF_H_
#define DEBUGDEF_H_

//#define DEBUG
#ifdef DEBUG
	#include <iostream>
	#define LOG_DEBUG(STREAM_SEQ) { std::string file = __FILE__; int slash_pos = file.rfind("/") + 1; int dot_pos = file.find("."); \
	std::string classname = file.substr(slash_pos, dot_pos - slash_pos); \
	std::cout << classname << "::" << __FUNCTION__ << " " << STREAM_SEQ << endl; }
	// may optionally print to the simulation EV
#else
	#define LOG_DEBUG(STREAM_SEQ)
#endif /* DEBUG */

#endif /* DEBUGDEF_H_ */
