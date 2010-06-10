//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "LRUCache.h"


LRUCache::LRUCache(uint64 capacity):capacity(capacity), remaining(capacity){
	return;
}

LRUCache::~LRUCache() {
	LCM::iterator it;
	for(it = resources.begin(); it != resources.end(); it++) {
		delete ((*it).first);
	}
}
/**
 * indicates whether the cache contains a resource
 */
Resource * LRUCache::has(Resource * r) {
	LCM::iterator it;
	it = resources.find(r);
	if (it == resources.end()) {
		//cout<<"DontHave: "<<r->getID()<<endl;
		return NULL;
	}
	//cout<<"Have: "<<r->getID()<<endl;
	return (*it).first;
}
/**
 * Adds resource to cache, returns true if cache did not contain resource.
 */
void LRUCache::add(Resource * r) {
	if (has(r)) {
		//cout<<"RM(REPL): " <<r->getID()<<endl;
		resources.erase(r);
		remaining += r->getSize();
	}
	pair<LCM::iterator,bool> ret =
		resources.insert(pair<Resource *,simtime_t>(r,simulation.getSimTime()));
		//cout<<"Adding: "<<r->getID()<<endl;
	while (r->getSize() > remaining) { // cache is full. free up space.
		Resource * old = getOldestResource();
		remaining += old->getSize();
		resources.erase(old);
		//cout<<"Removing: "<<old->getID()<<endl;
	}
	remaining -= r->getSize();
}
void LRUCache::empty() {
	resources.clear();
	remaining = capacity;
}
Resource * LRUCache::getOldestResource() {
	Resource * oldest = NULL;
	simtime_t oldestTime = -1;
	LCM::iterator it;
	for (it = resources.begin(); it !=resources.end(); it++) {
			if( (*it).second < oldestTime || oldest == NULL ) {
				oldest = ((*it).first);
				oldestTime = (*it).second;
			}
	}
	return oldest;
}

uint64 LRUCache::getRemainingCapacity() {
	return remaining;
}
uint64 LRUCache::getCapacity() {
	return capacity;
}
void LRUCache::renew(Resource * res) {
	Resource * existing = has(res);
	if (!existing) {
		opp_error("LRUCache::renew: resource not already in cache.");
	}
	add(existing);
}
