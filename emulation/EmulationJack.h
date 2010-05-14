// Author: Kevin Black

#ifndef EMULATIONJACK_H_
#define EMULATIONJACK_H_

#include <omnetpp.h>
#include <simtime.h>

#include <map>

#include "EmulationPluginInterface.h"
#include "EmulationJackMsg_m.h"
#include "EmulationJackCallback.h"

class EmulationJack: public cSimpleModule {
protected:
	// callback id -> callback msg
	std::map<int, EmulationJackMsg *> _callback_map;
	int _callback_counter;

public:
	// SECTION: EmulationPlugin methods

	EmulationJack ();
	virtual ~EmulationJack ();

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

#endif /* EMULATIONJACK_H_ */
