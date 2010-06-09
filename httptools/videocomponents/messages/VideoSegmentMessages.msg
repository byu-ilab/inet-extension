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

// these fields make it more convenient to treat a video
// request than it would be to parse the uri every single time,
// yet a valid uri should still be included in the request

message VideoSegmentRequestMessage extends httptRequestMessage
{
    string title = "";
    string type = "";
    int qualityLevel = 0;
    int segmentNumber = -1;
}

message VideoSegmentReplyMessage extends httptReplyMessage 
{
    string title = "";
    string type  = "";
    int qualityLevel = 0;
    int segmentNumber = -1;
}   
