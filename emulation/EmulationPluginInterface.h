/*
 * EmulationPluginInterface.h
 *
 *  Created on: May 13, 2010
 *      Author: Kevin Black
 */

#ifndef EMULATIONPLUGININTERFACE_H_
#define EMULATIONPLUGININTERFACE_H_

class EmulationPluginInterface {
public:
	virtual void handleCallback(void * yourPtr) =0;
};

#endif /* EMULATIONPLUGININTERFACE_H_ */
