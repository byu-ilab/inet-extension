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

#include "VideoPlayback.h"

VideoPlayback::VideoPlayback(int numSegments, cSimpleModule * host) {
	this->numSegments = numSegments;
	this->host = host;
	this->currSegment = -1;
	this->state = IDLE;
}

VideoPlayback::~VideoPlayback() {
}
bool VideoPlayback::videoComplete() {
	return state == END;
}
bool VideoPlayback::canDownload() {
	return state != END && state != WATCH_LAST;
}
bool VideoPlayback::videoBuffering() {
	return state == BUFFER;
}
void VideoPlayback::networkReady(ActiveRegion * buffer) {
	if (state == IDLE || state == BUFFER) {
		advanceHead(buffer);
	}
}

int VideoPlayback::advanceHead(ActiveRegion * buffer) {
	ASSERT(state != END);
	int quality = -1;
	if (!buffer->hasAvailableVideo() && currSegment < numSegments - 1) {
		state = BUFFER;
	} else {
		state = WATCH;
		if (currSegment < numSegments - 1) {
			quality = buffer->shift();
		}
		++currSegment;
		if (currSegment == numSegments - 1) {
			state = WATCH_LAST;
		} else if (currSegment == numSegments) {
			state = END;
		}
	}
	return quality;
}

