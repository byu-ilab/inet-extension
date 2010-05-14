// Author: Kevin Black

#ifndef EMULATIONPLUG_H_
#define EMULATIONPLUG_H_

#include <omnetpp.h>
#include <simtime.h>

#include <map>

#include "EmulationPluginInterface.h"
#include "EmulationPlugMsg_m.h"
#include "EmulationPlugCallback.h"

class EmulationPlug: public cSimpleModule {
protected:
	// callback id -> callback msg
	std::map<int, EmulationPlugMsg *> _callback_map;
	int _callback_counter;

public:
	// SECTION: EmulationPlugin methods

	EmulationPlug ();
	virtual ~EmulationPlug ();

	// @return the callback id as an int
	virtual int scheduleCallback(simtime_t t_from_now,
			EmulationPluginInterface * plugin, void * yourPtr);

	virtual void cancelCallback(int callback_id);

	// @return the current simulation time
	virtual simtime_t getSimTime();

	// SECTION: overwritten cSimpleModule methods

	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	virtual void finish();
};

#endif /* EMULATIONPLUG_H_ */
