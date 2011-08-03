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
#include "NetworkMonitor.h"
#include "VideoPlayback.h"

ActiveRegion::ActiveRegion(double segmentDuration,int maxBufferSizeKB, double epsilon):
	segmentDuration(segmentDuration), epsilon(epsilon), nextSegment(0), offset(0) {
	maxBuffer_bytes = 1000 * maxBufferSizeKB;
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
	int ns = nextSegment - offset; // get true 'nextSegment'
	ASSERT(segment >= ns);
	if (expectedRegion.size() == 0) {
		return 0;
	} else if (segment >= ns + expectedRegion.size()) {
		return 0;
	} else {
		return expectedRegion[segment - ns];
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
int64_t ActiveRegion::getSize(bool inBytes, Codec * codec) {
	ASSERT(!inBytes || codec); // no null codec if inbytes is true.
	int64_t total = 0;
	vector<int>::iterator it = region.begin();
	for(int i=nextSegment; it != region.end(); it++,i++) {
		int ql = *it;
		total += (inBytes?codec->getBlockSize(i,ql):ql);
	}
	return total;
}
int64_t ActiveRegion::getExpectedSize(bool inBytes, Codec * codec) {
	ASSERT(!inBytes || codec); // no null codec if inbytes is true.
	int64_t total = 0;
	vector<int>::iterator it = expectedRegion.begin();
	for(int i=nextSegment; it != expectedRegion.end(); it++,i++) {
		int ql = *it;
		total += (inBytes?codec->getBlockSize(i,ql):ql);
	}
	return total;
}
bool ActiveRegion::isFull(Codec * codec) {
	return getExpectedSize(true, codec) >=
			maxBuffer_bytes - codec->getBlockSize(nextSegment,1);
	// Todo: is this block size the max of possible choices?
}
int ActiveRegion::advance(Codec * codec, VideoPlayback * playback, NetworkMonitor * monitor) {
	int offset_l = 0;
	for (int i=nextSegment; i < playback->getNumSegments(); i++,offset_l++) {
		if (expectedQualityAt(i) == 0) {
			break;
		}
		int quality = expectedQualityAt(i) + 1;
		int blockSizeBytes = codec->getBlockSize(i, quality);
		double completionTime =(monitor->getRTT() + ( 8.0 * blockSizeBytes / monitor->getRate()));
		double completionSegmentOffset = completionTime / segmentDuration + epsilon;
		double head = playback->getExactHeadPosition();
		bool offsetInsufficient = floor(head + completionSegmentOffset) - floor(head) > offset_l;
		if (!offsetInsufficient) {
			break;
		}

	}
	if(offset_l + nextSegment == playback->getNumSegments() && offset_l > 0) {
		offset_l--; // don't want offset to push us past video.
	}
	/*if (expectedQualityAt(nextSegment) > 0) {
		double completionTime =(monitor->getRTT() + ( 8.0 * blockSize / monitor->getRate()));
		//cout<<"At "<<simTime()<<", completionTime is "<<completionTime<<endl;
		double completionSegmentOffset = completionTime / segmentDuration;
		double head = playback->getExactHeadPosition();
		offset_l = floor(head + completionSegmentOffset) - floor(head);

		// move the offset back so that it's not after any zero segments.
		for (; offset_l>0; offset_l-- ){
			if (expectedQualityAt(nextSegment + offset_l - 1) > 0) {
				break;
			}
		}
	}*/
	nextSegment += offset_l;
	offset = offset_l;
	return offset_l;
}
void ActiveRegion::retreat() {
	nextSegment -= offset;
	offset = 0;
}
int ActiveRegion::getNextExpectedZeroSegment() {
	int i=nextSegment;
	for (;expectedQualityAt(i)>0;i++);
	return i;
}
