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

	wwwName = (const char*)par("www");
	if ( wwwName.size() == 0 )
	{
		wwwName = "www.";
		wwwName += getParentModule()->getFullName();
		wwwName += ".com";
	}
	EV_DEBUG << "Initializing HTTP server. Using WWW name " << wwwName << endl;
	port = par("port");
		ASSERT(0 <= port && port < 65536);

	httpProtocol = par("httpProtocol");
		ASSERT(httpProtocol == 10 || httpProtocol == 11);

	// logging parameters
	ll = par("logLevel");
	logFileName = (const char*)par("logFile");
	enableLogging = logFileName!="";
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

    // Register the server with the controller object
   	registerWithController();

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
//		sprintf( buf, "%ld", htmlDocsServed + imgResourcesServed + textResourcesServed ); // TODO move this to httptHTMLServerBase
//		sprintf( buf, "Req: %ld", requestsReceived );
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
//		if ( activationTime>simTime() )
//		{
//			getParentModule()->getDisplayString().setTagArg("i2",0,"status/down");
//			getParentModule()->getDisplayString().setTagArg("i2",1,"red");
//		}
	}
}

void httptServerBase::registerWithController()
{
	// Find controller object and register
	cModule * controller = simulation.getSystemModule()->getSubmodule(par("controller"));
	if ( controller == NULL )
		error("Controller module not found");
	EV_DEBUG << "registerWithController: parent module's full name: "<<getParentModule()->getFullName()<<endl;
	((httptController*)controller)->registerWWWserver(getParentModule()->getFullName(),wwwName.c_str(),port,INSERT_END,activationTime);
}

void httptServerBase::handleMessage(cMessage *msg)
{
	// Override in derived classes
	updateDisplay();
}

httptReplyMessage * httptServerBase::handleRequestMessage( cMessage *msg ) //, int reply_interface_num)
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

	// @todo use the new request fields instead of parsing
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

	// Send a bad request response if requested
	if ( request->badRequest() )
	{
		// Bad requests get a 404 reply.
		EV_ERROR << "Bad request - bad flag set. Message: " << request->getName() << endl;
		replymsg = generateErrorReply(request, res[1], 404);
	}
	// handle the request method
	else if ( res[0] == "GET" )
	{
		EV_DEBUG << "Handling GET request " << request->getName() << " resource: " << res[1] << endl;
		replymsg = handleGetRequest(request,res[1]); // Pass in the resource string part
	}
	else
	{
		EV_ERROR << "Unsupported request type " << res[0] << " for " << request->heading() << endl;
		replymsg = generateErrorReply(request, res[1], 400);
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

httptReplyMessage* httptServerBase::generateErrorReply( httptRequestMessage *request, string resource_url, int code )
{
	badRequests++;
	return generateStandardReply(request, resource_url, code, par("errorReplySize"), rt_none);
//
//	httptReplyMessage * reply = new httptReplyMessage();
//	fillinReplyMessage(reply, request, resource_url, code, par("errorReplySize"), rt_none);
//	badRequests++;
//	return reply;
}

//void httptServerBase::fillinReplyMessage(httptReplyMessage * reply, httptRequestMessage * request,
//		string resource, int code, int size, int content_type)
//{
//	ASSERT(reply && request);
//	ASSERT(100 <= code && code <= 505);// TODO replace with constants or #define
//	ASSERT(0 <= size);
//
//	string header = "HTTP/1.";
//	switch(httpProtocol)
//	{
//	case HTTP_10: header = header + "0"; break;
//	case HTTP_11: header = header + "1"; break;
//	default:
//		error("fillinReplyMessage(): Unknown HTTP protocol");
//	}
//	header = header + " " + httpCodeAsString(code) + " " + httpPhraseFromCode(code);
//
//	reply->setHeading(header.c_str());
//
//	if (!resource.empty())
//	{
//		header = header + " (" + resource + ")"; // to identify the resource in question
//		reply->setRelatedUri(resouce.c_str());
//	}
//	reply->setName(header.c_str());
//
//	reply->setProtocol(request->protocol());  // MIGRATE40: kvj
//	reply->setResult(code);
//	reply->setPhrase(httpPhraseFromCode(code).c_str());
//
//	reply->setOriginatorUrl(wwwName.c_str());
//	reply->setTargetUrl(request->originatorUrl());
//	reply->setSerial(request->serial());
//	reply->setContentType(content_type);
//
//	reply->setByteLength(size);
//	reply->setKind(HTTPT_RESPONSE_MESSAGE);
//}

httptReplyMessage * httptServerBase::generateStandardReply(httptRequestMessage * request,
		string resource_uri, int code, int size, int content_type)
{
	ASSERT(request);
	ASSERT(100 <= code && code <= 505);// TODO replace with constants or #define
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

	httptReplyMessage * reply = new httptReplyMessage();

	reply->setHeading(header.c_str());

	if (!resource_uri.empty())
	{
		header = header + " (" + resource_uri + ")"; // to identify the resource in question
		reply->setRelatedUri(resource_uri.c_str());
	}
	reply->setName(header.c_str());

	reply->setProtocol(request->protocol());  // MIGRATE40: kvj
	reply->setResult(code);
	reply->setPhrase(httpPhraseFromCode(code).c_str());

	reply->setOriginatorUrl(wwwName.c_str());
	reply->setTargetUrl(request->originatorUrl());
	reply->setSerial(request->serial());
	reply->setContentType(content_type);

	reply->setByteLength(size);
	reply->setKind(HTTPT_RESPONSE_MESSAGE);

	return reply;
}

/// @see RFC 2616, sections 10.2.7, 10.4.17, 14.16, 14.35.1
httptReplyMessage * httptServerBase::generateByteRangeReply(
		httptRequestMessage * request, string resource_uri, int resource_size, int content_type)
{
	ASSERT(request);
	ASSERT(resource_size > 0);

	int fbp = request->firstBytePos();
	int lbp = request->lastBytePos();

	// check for invalid syntax and/or that the range header is not set
	if ( fbp < 0 || (0 <= lbp && lbp < fbp) )
	{
		// ignore the error and just return the whole entity with a 200 OK
		return generateStandardReply(request, resource_uri, 200, resource_size, content_type);//TODO replace 200 with constant or #define
	}

	httptReplyMessage * reply = NULL;

	// check if is unsatisfiable
	if ( resource_size < fbp )
	{
		// send a 416 error
		reply = generateErrorReply(request, resource_uri, 416);//, par("errorReplySize"), content_type);// TODO replace 416 with constant or #define
		reply->setFirstBytePos(BRS_ASTERISK);
		reply->setInstanceLength(resource_size);
		return reply;
	}

	// else is satisfiable, send back a 206 response
	if ( lbp < 0 || resource_size < lbp )
	{
		lbp = resource_size - 1;
	}
	reply = generateStandardReply(request, resource_uri, 206, lbp - fbp, content_type); // TODO replace 206 with constant or #define
	reply->setFirstBytePos(fbp);
	reply->setLastBytePos(lbp);
	reply->setInstanceLength(resource_size);
	return reply;
}
