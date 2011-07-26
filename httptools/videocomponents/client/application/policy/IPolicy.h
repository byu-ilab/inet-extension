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

#ifndef IPOLICY_H_
#define IPOLICY_H_
#include "ActiveRegion.h"
#include "VideoPlayback.h"
#include "NetworkMonitor.h"

class IPolicy {
	virtual int _selectSegment(ActiveRegion *, VideoPlayback *, NetworkMonitor *) = 0;
public:
	IPolicy();
	virtual ~IPolicy();
	virtual int selectSegment(Codec *,ActiveRegion *, VideoPlayback *, NetworkMonitor *);
};

#endif /* IPOLICY_H_ */
