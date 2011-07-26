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

#include "HorizontalPolicy.h"

HorizontalPolicy::HorizontalPolicy() {
	// TODO Auto-generated constructor stub

}

HorizontalPolicy::~HorizontalPolicy() {
	// TODO Auto-generated destructor stub
}
int HorizontalPolicy::findMinQuality(ActiveRegion * buffer, int N) {
	int minQuality = buffer->expectedQualityAt(N-1);
	int nextIndex = buffer->getNextSegment();
	for (int i=nextIndex; i < N; i++) {
		if (buffer->expectedQualityAt(i) < minQuality ) {
			minQuality = buffer->expectedQualityAt(i);
		}
	}
	return minQuality;
}
int HorizontalPolicy::_selectSegment(ActiveRegion *buffer, VideoPlayback *playback, NetworkMonitor * monitor) {

	int N = playback->getNumSegments();
	int minQuality = findMinQuality(buffer, N);

	int nextIndex = buffer->getNextSegment();
	for (int i=nextIndex; i < N; i++) {
		if (buffer->expectedQualityAt(i) == minQuality ) {
			return i;
		}
	}
	return -1;
}
