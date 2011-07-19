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

#ifndef MEANVERTICALPOLICY_H_
#define MEANVERTICALPOLICY_H_

#include "IPolicy.h"

class MeanVerticalPolicy: public IPolicy {
public:
	MeanVerticalPolicy();
	virtual ~MeanVerticalPolicy();
	virtual int selectSegment(ActiveRegion *, VideoPlayback *, double rate);
};

#endif /* MEANVERTICALPOLICY_H_ */
