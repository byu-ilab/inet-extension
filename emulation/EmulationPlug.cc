/*
 * EmulationPlug.cc
 *
 *  Created on: May 13, 2010
 *      Author: Kevin Black
 */

#include "EmulationPlug.h"

EmulationPlug::EmulationPlug() : _callback_map(), _callback_counter(0) {

}

EmulationPlug::~EmulationPlug() {
	std::map<int, EmulationPlugMsg *>::iterator i = _callback_map.begin();

	while (i != _callback_map.end()){
		cancelAndDelete(i->second);
	}
}

void EmulationPlug::initialize() {
	// initialize scalars
}

void EmulationPlug::handleMessage(cMessage * msg) {
	if (msg->isSelfMessage()){
		EmulationPlugMsg * cbmsg = check_and_cast<EmulationPlugMsg *>(msg);

		EmulationPlugCallback callback = cbmsg->getCallback();

		callback.plugin->handleCallback(callback.data);
	}
	delete msg;
}

void EmulationPlug::finish() {
	// record scalars
}


int EmulationPlug::scheduleCallback(simtime_t t_from_now,
		EmulationPluginInterface * plugin, void * yourPtr) {
	EmulationPlugCallback cb;
	cb.plugin = plugin;
	cb.data = yourPtr;
	EmulationPlugMsg * msg = new EmulationPlugMsg();
	msg->setCallback(cb);

	scheduleAt(simTime()+t_from_now, msg);

	_callback_map[_callback_counter] = msg;
	int id = _callback_counter;
	_callback_counter++;// assume that will not exceed Int32.MaxValue
	return id;
}

void EmulationPlug::cancelCallback(int callback_id) {
	std::map<int, EmulationPlugMsg *>::iterator i = _callback_map.find(callback_id);

	if (i != _callback_map.end()) {
		cancelAndDelete (i->second);
	}
}


simtime_t EmulationPlug::getSimTime() {
	return simTime();
}
