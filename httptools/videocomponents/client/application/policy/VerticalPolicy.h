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

#ifndef VERTICALPOLICY_H_
#define VERTICALPOLICY_H_

#include "IPolicy.h"

class VerticalPolicy: public IPolicy {
public:
	VerticalPolicy();
	virtual ~VerticalPolicy();
	virtual int selectSegment(ActiveRegion *, VideoPlayback *, double rate);
};

#endif /* VERTICALPOLICY_H_ */
