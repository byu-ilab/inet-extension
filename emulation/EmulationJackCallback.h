/*
 * EmulationPlugCallback.h
 *
 *  Created on: May 13, 2010
 *      Author: Kevin Black
 */

#ifndef EMULATIONJACKCALLBACK_H_
#define EMULATIONJACKCALLBACK_H_

#include "EmulationPluginInterface.h"

struct EmulationJackCallback {
	EmulationPluginInterface * plugin;
	void * data;
};

#endif /* EMULATIONJACKCALLBACK_H_ */
