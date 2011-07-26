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

#include "NonAdditivePolicy.h"
#include <math.h>
NonAdditivePolicy::NonAdditivePolicy(double completionPos, Codec * codec) {
	targetCompletionPosition = completionPos;
	this->codec = codec;

}

NonAdditivePolicy::~NonAdditivePolicy() {
	// TODO Auto-generated destructor stub
}

int NonAdditivePolicy::_selectSegment(ActiveRegion *buffer, VideoPlayback *playback, NetworkMonitor * monitor) {
	double rate = monitor->getRate();
	int segment = buffer->getNextExpectedZeroSegment();
	double deadline = segment - 1 + targetCompletionPosition;
	double segmentsUntilDeadline = deadline - playback->getExactHeadPosition();
	// if less than 1, hurry.
	// if 1, that's a desired equilibrium.
	// if greater than 1, set expectations high and download a lot.
	// quality-induced rate: (bits per second to blocks per segmentDuration)
	double secondsUntilDeadline = playback->getSegmentDuration() * segmentsUntilDeadline;
	double downloadTime = max(0.0,secondsUntilDeadline - monitor->getRTT());
	int bytesAchievable = (int)((rate * downloadTime) / 8.0);
	int quality = 1;
	buffer->requestedBlock(segment);

	for (quality = 2;codec->getBlockSize(segment, quality) <= bytesAchievable;quality++) {
		buffer->requestedBlock(segment);
	}
	//cout<<"Seg: "<<segment<<"; hp: "<<playback->getExactHeadPosition()<<"; DL: "<<deadline<<"; segUntilDL: "<<segmentsUntilDeadline<<"; sec: "<<secondsUntilDeadline<<"; bytes: "<<bytesAchievable<<"; q: "<<buffer->expectedQualityAt(segment)<<endl;
	return segment;
}

int NonAdditivePolicy::selectSegment(Codec *codec,ActiveRegion *buffer, VideoPlayback *playback, NetworkMonitor *monitor) {
	int segment = this->_selectSegment(buffer, playback, monitor);

}
