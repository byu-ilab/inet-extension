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

#ifndef CACHE_H_
#define CACHE_H_

#include <omnetpp.h>
#include "httptLogdefs.h"
#include "Resource.h"

class Cache {
public:
	/**
	 * Indicates whether the cache holds a resource.
	 * Returns the resource if it exists, and NULL if it does not.
	 */
	virtual Resource * has(Resource *)=0;
	/**
	 * Adds resource to the cache.
	 * @pre cache must not have resource before adding.
	 */
	virtual void add(Resource *)=0;
	/**
	 * Clears the cache.
	 */
	virtual void empty() = 0;

	virtual unsigned long getCapacity() = 0;
	virtual unsigned long getRemainingCapacity() = 0;
	virtual void renew(Resource *) = 0;
};

#endif /* CACHE_H_ */
