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

#include "httptServerBase.h"

void httptServerBase::initialize()
{
	badRequests = 0;
	EV_DEBUG << "Initializing server base component\n";

	wwwName = par("www").stdstringValue();
	if ( wwwName.size() == 0 )
	{
		wwwName += getParentModule()->getFullName();
		wwwName += ".omnet.net";
	}
	EV_DEBUG << "Initializing HTTP server. Using WWW name " << wwwName << endl;
	port = par("port");
		ASSERT(0 <= port && port < 65536);

	httpProtocol = par("httpProtocol");
		ASSERT(httpProtocol == HTTP_10 || httpProtocol == HTTP_11);

	// find and register with controller
	controller = check_and_cast<httptController *>(simulation.getSystemModule()->getSubmodule(par("controller")));
	EV_DEBUG << "registerWithController: parent module's full name: "<<getParentModule()->getFullName()<<endl;
	controller->registerWWWserver(getParentModule()->getFullName(),wwwName.c_str(),port,INSERT_END,activationTime);

	// logging parameters
	ll = par("logLevel");
	logFileName = par("logFile").stdstringValue();
	enableLogging = !logFileName.empty();
	outputFormat = lf_short;

	activationTime = par("activationTime");

	/*// In the event that we want to use a random object instead of omnet's random functions
	cXMLElement *rootelement = par("config").xmlValue();
	if ( rootelement==NULL ) error("Configuration file is not defined");

	// Initialize the distribution objects for random browsing
 	// @todo Skip initialization of random objects for scripted servers?
	cXMLAttributeMap attributes;
	cXMLElement *element;
	rdObjectFactory rdFactory;

	// Error message size
	element = rootelement->getFirstChildWithTag("errorMessageSize");
	if ( element==NULL ) error("Error message size parameter undefined in XML configuration");
	attributes = element->getAttributes();
	rdErrorMsgSize = rdFactory.create(attributes);
	if ( rdErrorMsgSize==NULL) error("Error message size random object could not be created");
	*/

	/*// not currently supported
	activationTime = par("activationTime");
	EV_INFO << "Activation time is " << activationTime << endl;
	*/
	updateDisplay();
}

void httptServerBase::finish()
{
	EV_SUMMARY << "Bad requests " << badRequests << "\n";
    recordScalar("bad.requests", badRequests);
}

void httptServerBase::updateDisplay()
{
	if ( ev.isGUI() )
	{
		char buf[1024];
		sprintf(buf, "Bad req: %ld", badRequests);
		getParentModule()->getDisplayString().setTagArg("t",0,buf);
		if ( activationTime<=simTime() )
    	{
			getParentModule()->getDisplayString().setTagArg("i2",0,"status/up");
			getParentModule()->getDisplayString().setTagArg("i2",1,"green");
		}
		else
		{
			getParentModule()->getDisplayString().setTagArg("i2",0,"status/down");
			getParentModule()->getDisplayString().setTagArg("i2",1,"red");
		}
	}
}
/*//OLD CODE
void httptServerBase::registerWithController()
{
	EV_DEBUG << "registerWithController: parent module's full name: "<<getParentModule()->getFullName()<<endl;
	controller->registerWWWserver(getParentModule()->getFullName(),wwwName.c_str(),port,INSERT_END,activationTime);
}
*/

void httptServerBase::handleMessage(cMessage *msg)
{
	// Override in derived classes
	updateDisplay();
}

httptReplyMessage * httptServerBase::handleRequestMessage( cMessage *msg )
{
	httptRequestMessage *request = check_and_cast<httptRequestMessage *>(msg);

	EV_DEBUG << "Handling received message " << msg->getName() << ". Target URL: " << request->targetUrl() << endl;

	logRequest(request);

	// @todo figure out what these do and/or implement them
	//simtime_t processingDelay = 0;
	//bool recipientError=false;

	// This should never happen but lets check
	if ( extractServerName(request->targetUrl()) != wwwName )
	{
		// TODO: KVJ left a "DEBUG HERE" notice
		error("Received message intended for '%s'", request->targetUrl());
	}

	httptReplyMessage* replymsg;

	int method = request->method();
	string uri = request->uri();

	if (method == RM_NONE || uri.empty())
	{
		// Verify that the header string has the correct number of parameters
			// Parse the request string on spaces
		cStringTokenizer tokenizer = cStringTokenizer(request->heading()," ");
		std::vector<string> res = tokenizer.asVector();
		if ( res.size() != 3 )
		{
			EV_ERROR << "Invalid request string: " << request->heading() << endl;
			replymsg = generateErrorReply(request, "", 400);
			logResponse(replymsg);
			return replymsg;
		}

		method = httpMethodFromString(res[0]);
		uri = res[1];
	}

	// Send a bad request response if requested
	if ( request->badRequest() )
	{
		// Bad requests get a 404 reply.
		EV_ERROR << "Bad request - bad flag set. Message: " << request->getName() << endl;
		replymsg = generateErrorReply(request, uri, 404);
	}
	// handle the request method
	else if ( method == RM_GET )
	{
		EV_DEBUG << "Handling GET request " << request->getName() << " resource: " << uri << endl;
		replymsg = handleGetRequest(request,uri); // Pass in the resource string part
	}
	else
	{
		EV_ERROR << "Unsupported request type " << httpMethodAsString(method) <<
			" for " << request->heading() << endl;
		replymsg = generateErrorReply(request, uri, 400);
	}

	if ( replymsg!=NULL )
		logResponse(replymsg);

	return replymsg;
}

httptReplyMessage * httptServerBase::handleGetRequest( httptRequestMessage *request, string resource_url )
{
	EV_ERROR << "Unknown or unsupported resource requested in " << request->heading() << endl;
	return generateErrorReply(request, resource_url, 404);
}

httptReplyMessage* httptServerBase::generateErrorReply( httptRequestMessage *request, string resource_uri, int code )
{
	badRequests++;
	return generateStandardReply(request, resource_uri, code, par("errorReplySize"), rt_unknown);
}

void httptServerBase::fillinErrorReply(httptReplyMessage * reply, httptRequestMessage *request, string resource_uri, int code)
{
	badRequests++;
	fillinStandardReply(reply, request, resource_uri, code, par("errorReplySize"), rt_unknown);
}

httptReplyMessage * httptServerBase::generateStandardReply(httptRequestMessage * request,
		string resource_uri, int code, int size, int content_type)
{
	httptReplyMessage * reply = new httptReplyMessage();
	fillinStandardReply(reply, request, resource_uri, code, size, content_type);
	return reply;
}

void httptServerBase::fillinStandardReply(httptReplyMessage * reply, httptRequestMessage * request,
		string resource_uri, int code, int size, int content_type)
{
	ASSERT(reply && request);
	ASSERT(HTTP_CODE_MIN <= code && code <= HTTP_CODE_MAX);
	ASSERT(0 <= size);

	string header = "HTTP/1.";
	switch(httpProtocol)
	{
	case HTTP_10: header = header + "0"; break;
	case HTTP_11: header = header + "1"; break;
	default:
		error("generateStandardReply(): Unknown HTTP protocol");
	}
	header = header + " " + httpCodeAsString(code) + " " + httpPhraseFromCode(code);

	reply->setHeading(header.c_str());

	if (!resource_uri.empty())
	{
		header = header + " (" + resource_uri + ")"; // to identify the resource in question
		reply->setRelatedUri(resource_uri.c_str());
	}
	reply->setName(header.c_str());

	reply->setProtocol(request->protocol());
	reply->setResult(code);
	reply->setPhrase(httpPhraseFromCode(code).c_str());

	reply->setOriginatorUrl(wwwName.c_str());
	reply->setTargetUrl(request->originatorUrl());
	reply->setSerial(request->serial());
	reply->setContentType(content_type);

	reply->setByteLength(size);
	reply->setKind(HTTPT_RESPONSE_MESSAGE);
}

httptReplyMessage * httptServerBase::generateByteRangeReply(
		httptRequestMessage * request, string resource_uri, int resource_size, int content_type)
{
	httptReplyMessage * reply = new httptReplyMessage();
	fillinByteRangeReply(reply, request, resource_uri, resource_size, content_type);
	return reply;
}

/// @see RFC 2616, sections 10.2.7, 10.4.17, 14.16, 14.35.1
void httptServerBase::fillinByteRangeReply(httptReplyMessage * reply, httptRequestMessage * request,
		string resource_uri, int resource_size, int content_type)
{
	ASSERT(reply && request);
	ASSERT(0 < resource_size);

	int fbp = request->firstBytePos();
	int lbp = request->lastBytePos();

	// check for invalid syntax and/or that the range header is not set
	if ( fbp < 0 || (0 <= lbp && lbp < fbp) )
	{
		// ignore the error and just return the whole entity with a 200 OK
		fillinStandardReply(reply, request, resource_uri, HTTP_CODE_200, resource_size, content_type);
		return;
	}

	// check if is unsatisfiable
	if ( resource_size < fbp )
	{
		// send a 416 error
		fillinErrorReply(reply, request, resource_uri, HTTP_CODE_416);
		reply->setFirstBytePos(BRS_ASTERISK);
		reply->setInstanceLength(resource_size);
		return;
	}

	// else is satisfiable, send back a 206 response
	if ( lbp < 0 || resource_size < lbp )
	{
		lbp = resource_size - 1;
	}
	fillinStandardReply(reply, request, resource_uri, HTTP_CODE_206, lbp - fbp, content_type);
	reply->setFirstBytePos(fbp);
	reply->setLastBytePos(lbp);
	reply->setInstanceLength(resource_size);
}
