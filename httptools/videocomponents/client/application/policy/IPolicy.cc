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

#include "IPolicy.h"
#include <omnetpp.h>
IPolicy::IPolicy() {
	// TODO Auto-generated constructor stub

}

IPolicy::~IPolicy() {
	// TODO Auto-generated destructor stub
}

int IPolicy::selectSegment(Codec * codec, ActiveRegion *buffer, VideoPlayback *playback, NetworkMonitor * monitor) {
	int offset = buffer->advance(codec, playback, monitor);
	int segment = this->_selectSegment(buffer, playback, monitor);
	buffer->retreat();
	//cout<<"Selecting "<<segment<<", offset of "<<offset;
	buffer->requestedBlock(segment);
	//cout<<", Running total for segment: "<<buffer->expectedQualityAt(segment)<<endl;
	cout<<"With nextSegment "<<buffer->getNextSegment()<<" Chose segment "<<segment<<endl;
	return segment;
}
