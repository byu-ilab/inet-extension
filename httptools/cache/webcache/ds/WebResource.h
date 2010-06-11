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

#ifndef WEBRESOURCE_H_
#define WEBRESOURCE_H_

#include "Resource.h"

class WebResource: public Resource {
private:
	string uri;
	uint64 size;
	int type;
	string content;
public:
	WebResource(string uri, uint64 size, int type=-1, string content="");
	WebResource(const WebResource &);
	virtual ~WebResource();
protected:
	virtual string getID();
	virtual uint64 getSize();
	virtual int    getType();
	virtual string getContent();
};

#endif /* WEBRESOURCE_H_ */
