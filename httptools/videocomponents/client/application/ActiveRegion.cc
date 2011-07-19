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

#include "ActiveRegion.h"

ActiveRegion::ActiveRegion(): nextSegment(0) {
}

ActiveRegion::~ActiveRegion() {
}
int ActiveRegion::shift() {
	ASSERT(region.size() > 0);
	int retval = region[0];
	region.erase(region.begin());
	expectedRegion.erase(expectedRegion.begin());
	nextSegment++;
	return retval;
}
bool ActiveRegion::hasAvailableVideo() {
	return region.size() > 0;
}
bool ActiveRegion::receivedBlock(int segment) {
	if (segment < nextSegment) {
		return false;
	}
	ASSERT(segment <= nextSegment + region.size()); // must stay in valid region.
	if (segment == nextSegment + region.size()) {
		region.push_back(1);
	} else {
		region[segment - nextSegment]+= 1;
	}
	return true;
}

int ActiveRegion::qualityAt(int segment) {
	ASSERT(segment >= nextSegment);
	if (region.size() == 0) {
		return 0;
	} else if (segment >= nextSegment + region.size()) {
		return 0;
	} else {
		return region[segment - nextSegment];
	}
}

int ActiveRegion::expectedQualityAt(int segment) {
	ASSERT(segment >= nextSegment);
	if (expectedRegion.size() == 0) {
		return 0;
	} else if (segment >= nextSegment + expectedRegion.size()) {
		return 0;
	} else {
		return expectedRegion[segment - nextSegment];
	}
}
void ActiveRegion::requestedBlock(int segment) {
	ASSERT(segment >= nextSegment);
	ASSERT(segment <= nextSegment + expectedRegion.size()); // must stay in valid region.
	if (segment == nextSegment + expectedRegion.size()) {
		expectedRegion.push_back(1);
	} else {
		expectedRegion[segment - nextSegment]+= 1;
	}
}
