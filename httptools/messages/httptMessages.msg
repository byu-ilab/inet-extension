
// ----------------------------------------------------------------------------
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable 
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ----------------------------------------------------------------------------
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// @file httptMessages.msg
//
// HTTP message definitions for the HttpTools components.
//
// NEW: The module has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @version 1.0 
// @author  Kristjan V. Jonsson
// @contributor Kevin Black
//
// ----------------------------------------------------------------------------

cplusplus
{{
    #include "HTTPMsgDefs.h"
}}
enum HTTPProtocol;
enum RequestMethod;
enum ByteRangeState;

enum HTTPMessageKind
{
    HTTPT_REQUEST_MESSAGE 			= 10000;
    HTTPT_DELAYED_REQUEST_MESSAGE 	= 10001;
    HTTPT_RESPONSE_MESSAGE 			= 10010;
    HTTPT_DELAYED_RESPONSE_MESSAGE 	= 10011;
};

//
// Base class for HTTP messages
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptBaseMessage
{
    @omitGetVerb(true);
    string targetUrl; 				// The target URL - request set for browsers, echoed by servers.
    string originatorUrl = "";		// The originator URL. Only applicable for servers.
    int protocol enum(HTTPProtocol) = HTTP_11; // 10 for http/1.0, 11 for http/1.1.
    bool keepAlive = true;			// The keep-alive header
    int serial = 0;					// Convenience field which allows resource requests to be serially tagged for ease of analysis.
    string heading = "";			// The message heading - request string for requests, response for replies
    string payload = "";			// The payload field
    								// These two values should add together to match the total byte length of the packet:
    int headerLength = 0;			// The length of the header block in bytes
    int contentLength = 0;			// The length of the entity body in bytes and the value of the Content-Length headerp
}

//
// Message class for HTTP requests
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptRequestMessage extends httptBaseMessage
{
    @omitGetVerb(true);			// Dont append get to generated getter methods.
    bool badRequest = false;	// Set to true to simulate a bad request. Server answers w. 404.
    int method enum(RequestMethod) = RM_NONE;
    string uri = "";
    
    // Range header values
    
    // provides for the simulation of regular byte range requests, i.e. with a 
    // single Range header and single byte-set, specified by 
    // the first and (optional) last byte, doesn't provide for suffix range requests
    // see section 14.35.1 of RFC 2616
    
    // use of int gives index capacity up to 2GB (just the positive integers)
    
    // if is BRS_UNSPECIFIED then the Range header is not set, other negative values are
    // not defined
    int firstBytePos = BRS_UNSPECIFIED;
    // if is BRS_UNSPECIFIED then the last byte position is not set and/or the header is
    // not set (check firstBytePos)
    int lastBytePos = BRS_UNSPECIFIED;
}

//
// Message class for HTTP replies
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptReplyMessage extends httptBaseMessage
{
    @omitGetVerb(true);
    int result = 0; 				// e.g. 200 for OK, 404 for NOT FOUND.
    int contentType = 0;			// The content type: HTML pages, text or image resources.
    string phrase = "";				// The description phrase associated with the status code
    								// provided here to allow for use of status codes other
    								// than the ones known to httptutils
    string relatedUri = "";			// the uri of the resouce that this response pertains to
    								// provided merely as a convenience tag for tracking
    
    // Content-Range header values
    
    // provides for the simulation of regular byte range responses, i.e.
    // that include a Content-Range header, specified by
    // the first and last byte and the instance length
    // see section 14.16 of RFC 2616
    
    // use of int gives index capacity up to 2GB (just the positive integers)
    
    // if value is BRS_UNSPECIFIED then the Content-Range header is not set
    // if value is BRS_ASTERISK can be interpreted as an "*"
    int firstBytePos = BRS_UNSPECIFIED;
    // only has meaning if firstBytePos is positive
    int lastBytePos = BRS_UNSPECIFIED;
    // if value is BRS_ASTERISK can be interpreted as an "*"
    int instanceLength = BRS_UNSPECIFIED; 
}
