
// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
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
// ***************************************************************************

#include "httptNodeBase.h"

httptNodeBase::httptNodeBase()
{
	m_bDisplayMessage=false;
	m_bDisplayResponseContent=false;
}

const char* httptNodeBase::getWWW()
{
	Enter_Method_Silent();
	return wwwName.c_str();
}

void httptNodeBase::sendDirectToModule( httptNodeBase *receiver, cMessage *message, simtime_t constdelay, rdObject *rdDelay )
{
	cPacket *pckt = check_and_cast<cPacket *>(message);  // MIGRATE40: kvj
	if ( message==NULL ) return;
	simtime_t delay=constdelay+transmissionDelay(message);
	if ( rdDelay!=NULL ) delay+=rdDelay->get();
	EV_DEBUG << "Sending " << message->getName() << " direct to " << receiver->getParentModule()->getName() << " with a delay of " << delay << " s\n";
	sendDirect(message,receiver,"tcpIn");
}

double httptNodeBase::transmissionDelay( cMessage *msg )
{
	cPacket *pckt = check_and_cast<cPacket *>(msg);
	if ( linkSpeed==0 ) return 0.0; // No delay if linkspeed unspecified
	return pckt->getByteLength()/((double)linkSpeed/8);  // The linkspeed is in bits/s
}

void httptNodeBase::logRequest( const httptRequestMessage* httpRequest )
{
	if (!enableLogging) return;
	if ( outputFormat == lf_short )
		logEntry(formatHttpRequestShort(httpRequest));
	else
		logEntry(formatHttpRequestLong(httpRequest));
	if ( m_bDisplayMessage )
		EV_INFO << "Request:\n" << formatHttpRequestLong(httpRequest);
}

void httptNodeBase::logResponse( const httptReplyMessage* httpResponse )
{
	if (!enableLogging) return;
	if ( outputFormat == lf_short )
		logEntry(formatHttpResponseShort(httpResponse));
	else
		logEntry(formatHttpResponseLong(httpResponse));
	if ( m_bDisplayMessage )
		EV_INFO << "Response:\n" << formatHttpResponseLong(httpResponse);
}

void httptNodeBase::logEntry( string line )
{
	if (!enableLogging) return;
	if (logFileName.size() == 0) return;

	ofstream outfile;
	time_t curtime;
	time(&curtime);

	bool exists = fileExists(logFileName.c_str()); // Check if the file exists. If not, add the field names at top.

	outfile.open(logFileName.c_str(),ios::app);
	if ( !exists )
		outfile << "time;simtime;logging-node;sending-node;type;originator-url;target-url;protocol;keep-alive;serial;heading;bad-req;result-code;content-type;content-length" << endl;
	outfile << curtime << ";" << simTime() << ";" << getParentModule()->getName();
	if( outputFormat == lf_short )
		outfile << ";";
	else
		outfile << endl;
 	outfile << line << endl;
	outfile.close();
}

string httptNodeBase::formatHttpRequestShort( const httptRequestMessage* httpRequest )
{
	ostringstream str;

	string originatorStr = "";
	cModule *originator = httpRequest->getSenderModule();
	if ( originator!=NULL && originator->getParentModule()!=NULL )
		originatorStr = originator->getParentModule()->getFullName();

	str << originatorStr << ";";
	str << "REQ;" << httpRequest->originatorUrl() << ";" << httpRequest->targetUrl() << ";";
	str << httpRequest->protocol() << ";" << httpRequest->keepAlive() << ";" << httpRequest->serial() << ";";
	str << httpRequest->heading() << ";" << httpRequest->badRequest() << ";;"; // Skip the response specific fields
	str << ";" << httpRequest->getByteLength();

	return str.str();
}

string httptNodeBase::formatHttpResponseShort( const httptReplyMessage* httpResponse )
{
	ostringstream str;

	string originatorStr = "";
	cModule *originator = httpResponse->getSenderModule();
	if ( originator!=NULL && originator->getParentModule()!=NULL )
		originatorStr = originator->getParentModule()->getFullName();

	str << originatorStr << ";";
	str << "RESP;" << httpResponse->originatorUrl() << ";" << httpResponse->targetUrl() << ";";
	str << httpResponse->protocol() << ";" << httpResponse->keepAlive() << ";" << httpResponse->serial() << ";";
	str << httpResponse->heading() << ";;"; // Skip the request specific fields
	str << httpResponse->result() << ";" << httpResponse->contentType();
	str << ";" << httpResponse->getByteLength();
	str << "\npayload\n" << httpResponse->payload();


	return str.str();
}

string httptNodeBase::formatHttpRequestLong( const httptRequestMessage* httpRequest )
{
	ostringstream str;

	str << "REQUEST: " << httpRequest->getName() << " -- " << httpRequest->getByteLength() << " bytes\n";
	str << "Target URL:" << httpRequest->targetUrl() << "  Originator URL:" << httpRequest->originatorUrl() << endl;

	str << "PROTOCOL:";
	switch( httpRequest->protocol() )  // MIGRATE40: kvj
	{
		case 10: str << "HTTP/1.0"; break;
		case 11: str << "HTTP/1.1"; break;
		default: str << "UNKNOWN"; break;
	}
	str << "  ";

	str << "KEEP-ALIVE:" << httpRequest->keepAlive() << "  ";
	str << "BAD-REQ:" << httpRequest->badRequest() << "  ";
	str << "SERIAL:" << httpRequest->serial() << "  " << endl;

	str << "REQUEST:" << httpRequest->heading() << endl;

	return str.str();
}

string httptNodeBase::formatHttpResponseLong( const httptReplyMessage* httpResponse )
{
	ostringstream str;

	str << "RESPONSE: " << httpResponse->getName() << " -- " << httpResponse->getByteLength() << " bytes\n";

	str << "Target URL:" << httpResponse->targetUrl() << "  Originator URL:" << httpResponse->originatorUrl() << endl;

	str << "PROTOCOL:";
	switch( httpResponse->protocol() )
	{
		case 10: str << "HTTP/1.0"; break;
		case 11: str << "HTTP/1.1"; break;
		default: str << "UNKNOWN"; break;
	}
	str << "  ";

	str << "RESULT:" << httpResponse->result() << "  ";
	str << "KEEP-ALIVE:" << httpResponse->keepAlive() << "  ";
	str << "SERIAL:" << httpResponse->serial() << "  " << endl;

	str << "RESPONSE: " << httpResponse->heading() << endl;

	str << "CONTENT-TYPE:";
	switch( httpResponse->contentType() )
	{
		case rt_html_page: str << "HTML DOC"; break;
		case rt_text: str << "Text/HTML RES"; break;
		case rt_image: str << "IMG RES"; break;
		default: str << "UNKNOWN"; break;
	}
	str << endl;

	if ( m_bDisplayResponseContent )
	{
		str << "CONTENT:" << endl;
		str << httpResponse->payload() << endl;
	}

	return str.str();
}






