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

#include "WebResource.h"

WebResource::WebResource(string uri, uint64 size, int type, string content)
	: uri(uri), size(size), type(type), content(content) {
	//return;
}
WebResource::WebResource(const WebResource & other) {
	uri = "" + other.uri;
	size = other.size;

}
WebResource::~WebResource() {
	return;
}
string WebResource::getID() {
	return uri;
}
uint64 WebResource::getSize() {
	return size;
}

int WebResource::getType() {
	return type;
}

string WebResource::getContent() {
	return content;
}
