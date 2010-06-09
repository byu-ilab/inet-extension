// Author: Kevin Black
//
// Based off of the original httptServerBase class (which was refactored
// to httptHTMLServerBase and then modified to extend this class).  Provides
// generic HTTP server functions, such as generating error messages, parsing
// request options, validating byte range requests, and generating byte
// range reply messages.
//
//*****************************************************************************
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

#ifndef HTTPTSERVERBASE_H_
#define HTTPTSERVERBASE_H_

#include "httptNodeBase.h"

class httptServerBase: public httptNodeBase
{
protected:
	unsigned long badRequests;
	simtime_t activationTime;
	httptController * controller;

public:
	/** Overridden from cSimpleModule */
	//@{
	virtual void initialize();
	virtual void finish();
	virtual void handleMessage(cMessage *msg)=0;
	//@}

protected:
	/**
	 * Update the display string if running in GUI mode
	 */
	virtual void updateDisplay();

	/**
	 * Register the server object with the controller.
	 * Called at initialization (simulation startup).
	 */
	//void registerWithController();

	/**
	 * Handle a received data message, determine which HTTP option it is, if the bad bit is
	 * set then return an error response immediately.
	 *
	 * @param msg -- the message pointer
	 * @param reply_interface -- the interface to send replies on
	 */
	httptReplyMessage * handleRequestMessage( cMessage *msg );//, int reply_interface_num );

	/**
	 * Handles HTTP GET requests.  By default it just deletes the message, thus acting
	 * as a sink.
	 */
	virtual httptReplyMessage * handleGetRequest( httptRequestMessage *request, string resource_url);//, int reply_interface_num );

	// add other HTTP method handlers here if needed, and modify handleReceivedMessage to
	// correctly redirect to that handler

	/**
	 * Generate an error reply in case of invalid resource requests.
	 * Increments the bad request counter.
	 * @todo perhaps make an enum instead of an int
	 */
	httptReplyMessage* generateErrorReply( httptRequestMessage *request, string resource_uri, int code);

	/**
	 * Fills in fields associated with the standard reply message (not including the byte range fields).
	 * Increments the bad request counter.
	 */
	void fillinErrorReply(httptReplyMessage * reply, httptRequestMessage * request, string resource_uri, int code);

	/**
	 * Generates a standard reply message with the given parameters (i.e. one that doesn't
	 * worry about byte ranges).
	 */
	httptReplyMessage * generateStandardReply(httptRequestMessage * request, string resource_uri,
			int code, int size, int content_type);

	/**
	 * Fills in fields associated with the standard reply message (not including the byte range fields).
	 */
	void fillinStandardReply(httptReplyMessage * reply, httptRequestMessage * request,
			string resource_uri, int code, int resource_size, int content_type);

	/**
	 * Generates a standard reply message according to the indicated byte range request,
	 * validating it against the indicated resource size.
	 */
	httptReplyMessage * generateByteRangeReply(httptRequestMessage * request,
			string resource, int resource_size, int content_type);

	/**
	 * Fills in the standard reply fields as well as the byte range fields.
	 */
	void fillinByteRangeReply(httptReplyMessage * reply, httptRequestMessage * request,
			string resource_uri, int resource_size, int content_type);
};

#endif /* HTTPTSERVERBASE_H_ */



