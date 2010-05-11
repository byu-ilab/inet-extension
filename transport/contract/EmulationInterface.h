// Author: Kevin Black

#include <simtime.h>

class EmulationInterface
{
	virtual void scheduleCallback () =0;
	virtual simtime_t getSimTime () =0;
};
