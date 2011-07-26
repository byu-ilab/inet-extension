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

#include "MeanVerticalPolicy.h"
#include <math.h>

MeanVerticalPolicy::MeanVerticalPolicy(int blockSize):blockSize(blockSize) {

}

MeanVerticalPolicy::~MeanVerticalPolicy() {
	// TODO Auto-generated destructor stub
}
int MeanVerticalPolicy::_selectSegment(ActiveRegion *buffer, VideoPlayback *playback,  NetworkMonitor * monitor) {
	double rate = monitor->getMeanRate();
	// quality-induced rate: (bits per second to blocks per segmentDuration)
	rate = (playback->getSegmentDuration() *rate) / (8.0 * blockSize);
	//cout<<"Mean Rate (blocks per second): "<<rate<<endl;
	int N = playback->getNumSegments();
	double index = N * (1.0 - (rate - (int)rate));
	int nextIndex = buffer->getNextSegment();
	if (rate < 1.0) {
		return nextIndex;
	}
	for (int i=nextIndex; i < N; i++) {
		if (buffer->expectedQualityAt(i) < (i < index ? floor(rate): ceil(rate))) {
			return i;
		}
	}
	return nextIndex;
}
