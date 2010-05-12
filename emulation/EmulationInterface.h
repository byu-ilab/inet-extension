// Author: Kevin Black

#include <omnetpp.h>
#include <simtime.h>

#define EMUL_CB void (*callback)(void * yourPtr)

class EmulationInterface : public cSimpleModule
{
	// @return the callback id as an int
	virtual int scheduleCallback (simtime_t t_from_now, void * yourPtr, EMUL_CB) =0;

	virtual void cancelCallback (int callback_id) =0;

	// @return the current simulation time
	virtual simtime_t getSimTime () =0;
};
