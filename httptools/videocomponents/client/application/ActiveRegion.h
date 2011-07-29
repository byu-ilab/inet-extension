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

#ifndef ACTIVEREGION_H_
#define ACTIVEREGION_H_
#include <vector>
#include <omnetpp.h>
#include "Codec.h"
using namespace std;
class VideoPlayback;
class NetworkMonitor;
class ActiveRegion {
private:
	vector<int> region; // region mapping future segments to qualities

	/* active region accounting for both received and anticipated qualities */
	vector<int> expectedRegion;
	//int blockSize;
	int segmentDuration;
	int nextSegment; // 0-based index of next segment to be played
	int offset; // used to change appearance of active region for the policy.
				// important in expectedQualityAt() method.
	int64_t maxBuffer_bytes;

	int64_t getExpectedSize(bool inBytes, Codec * codec);

public:
	ActiveRegion(double segmentDuration, int maxBufferSizeKB);
	virtual ~ActiveRegion();
	int shift();
	bool hasAvailableVideo();
	bool receivedBlock(int segment); // signal a block received. True iff block arrived on time.
	int getNextSegment() {return nextSegment;}
	int getNextExpectedZeroSegment();
	int qualityAt(int segment); //
	int expectedQualityAt(int segment);
	void requestedBlock(int segment); // requests a segment
	int64_t getSize(bool inBytes, Codec * codec);
	bool isFull(Codec * codec);
	int advance(Codec *,VideoPlayback*, NetworkMonitor*); // advance nextSegment 'artificially' for policy selection
	void retreat();  // move nextSegment back (must be called right after policy selection)

};

#endif /* ACTIVEREGION_H_ */
