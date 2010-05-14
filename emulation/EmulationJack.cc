/*
 * EmulationJack.cc
 *
 *  Created on: May 13, 2010
 *      Author: Kevin Black
 */

#include "EmulationJack.h"

EmulationJack::EmulationJack() : _callback_map(), _callback_counter(0) {

}

EmulationJack::~EmulationJack() {
	std::map<int, EmulationJackMsg *>::iterator i = _callback_map.begin();

	while (i != _callback_map.end()){
		cancelAndDelete(i->second);
	}
}

void EmulationJack::initialize() {
	// initialize scalars
}

void EmulationJack::handleMessage(cMessage * msg) {
	if (msg->isSelfMessage()){
		EmulationJackMsg * cbmsg = check_and_cast<EmulationJackMsg *>(msg);

		EmulationJackCallback callback = cbmsg->getCallback();

		callback.plugin->handleCallback(callback.data);
	}
	delete msg;
}

void EmulationJack::finish() {
	// record scalars
}


int EmulationJack::scheduleCallback(simtime_t t_from_now,
		EmulationPluginInterface * plugin, void * yourPtr) {
	EmulationJackCallback cb;
	cb.plugin = plugin;
	cb.data = yourPtr;
	EmulationJackMsg * msg = new EmulationJackMsg();
	msg->setCallback(cb);

	scheduleAt(simTime()+t_from_now, msg);

	_callback_map[_callback_counter] = msg;
	int id = _callback_counter;
	_callback_counter++;// assume that will not exceed Int32.MaxValue
	return id;
}

void EmulationJack::cancelCallback(int callback_id) {
	std::map<int, EmulationJackMsg *>::iterator i = _callback_map.find(callback_id);

	if (i != _callback_map.end()) {
		cancelAndDelete (i->second);
	}
}


simtime_t EmulationJack::getSimTime() {
	return simTime();
}
