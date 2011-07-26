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

#ifndef DIAGONALPOLICY_H_
#define DIAGONALPOLICY_H_

#include "IPolicy.h"

class DiagonalPolicy: public IPolicy {
public:
	DiagonalPolicy(double slope);
	virtual ~DiagonalPolicy();
private:
	double slope;
	virtual int _selectSegment(ActiveRegion *, VideoPlayback *,  NetworkMonitor * monitor);
	int findFirstZeroSegment(ActiveRegion *buffer, int nextIndex, int N);
};

#endif /* DIAGONALPOLICY_H_ */
