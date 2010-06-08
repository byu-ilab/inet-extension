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

cplusplus {{
#include "httptMessages_m.h"
}}

message httptRequestMessage;
message httptReplyMessage;

// use of int gives index capacity up to 2GB (just the positive integers)

// provides for the simulation of regular byte range requests, i.e. with a 
// single Range header and single byte-set, specified by 
// the first and (optional) last byte, doesn't provide for suffix range requests
// see section 14.35.1 of RFC 2616
packet httptByteRangeRequestMessage extends httptRequestMessage
{
    @omitGetVerb(true);			
    int firstBytePos = 0;
    // a negative value here indicates that it is unspecified, default is -1
    int lastBytePos = -1;
    	
}

// provides for the simulation of regular byte range responses, i.e.
// that include a Content-Range header, specified by
// the first and last byte and the instance length
// see section 14.16 of RFC 2616
packet httptByteRangeReplyMessage extends httptReplyMessage
{
    @omitGetVerb(true);
    // a negative value here can be interpreted as an "*"
    int firstBytePos = -1;
    int lastBytePos = -1;
    // a negative value here can be interpreted as an "*"
    int instanceLength = -1; 
}