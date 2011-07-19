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

#ifndef HORIZONTALPOLICY_H_
#define HORIZONTALPOLICY_H_

#include "IPolicy.h"

class HorizontalPolicy: public IPolicy {
public:
	HorizontalPolicy();
	virtual ~HorizontalPolicy();
	virtual int selectSegment(ActiveRegion *, VideoPlayback *, double rate);
private:
	int findMinQuality(ActiveRegion * buffer, int N); // finds min quality in future segments of video
};

#endif /* HORIZONTALPOLICY_H_ */
