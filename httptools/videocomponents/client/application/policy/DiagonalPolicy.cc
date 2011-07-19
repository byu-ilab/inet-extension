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

#include "DiagonalPolicy.h"
#include <limits>
DiagonalPolicy::DiagonalPolicy() {
	// TODO Auto-generated constructor stub

}

DiagonalPolicy::~DiagonalPolicy() {
	// TODO Auto-generated destructor stub
}
int DiagonalPolicy::selectSegment(ActiveRegion *buffer, VideoPlayback *playback, double rate) {
	double slope = -1;

	int nextIndex = buffer->getNextSegment();
	int last = findFirstZeroSegment(buffer, buffer->getNextSegment(), playback->getNumSegments());

	int index = nextIndex;
	double minVal = numeric_limits<double>::max();
	for (int i = nextIndex; i <= last; i++) {
		double val = buffer->expectedQualityAt(i) - slope * i;
		if (val < minVal) {
			minVal = val;
			index = i;
		}
	}
	return index;
}
int DiagonalPolicy::findFirstZeroSegment(ActiveRegion *buffer, int nextIndex, int N) {
	for (int i=nextIndex; i < N; i++) {
		if (buffer->expectedQualityAt(i) == 0) {
			return i;
		}
	}
	return N - 1;
}
