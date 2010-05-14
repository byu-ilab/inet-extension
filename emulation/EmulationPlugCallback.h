/*
 * EmulationPlugCallback.h
 *
 *  Created on: May 13, 2010
 *      Author: black
 */

#ifndef EMULATIONPLUGCALLBACK_H_
#define EMULATIONPLUGCALLBACK_H_

#include "EmulationPluginInterface.h"

struct EmulationPlugCallback {
	EmulationPluginInterface * plugin;
	void * data;
};

#endif /* EMULATIONPLUGCALLBACK_H_ */
