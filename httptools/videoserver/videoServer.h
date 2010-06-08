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

#ifndef __videoServerSock_H_
#define __videoServerSock_H_

#include "httptServerBase.h"
#include "TCPSocketAPI.h"

/**
 *
 * @version 1.0
 * @author  Travis Andelin
 */
class INET_API videoServer :
	public httptServerBase, TCPSocketAPI::CallbackInterface {

	protected:
		// internals
		httptController *controller; // Reference to central controller object.
		TCPSocketAPI * tcp_api;
		VideoTitleWorkloadGenerator * workload_generator;

		// stats
		unsigned long numBroken;
		unsigned long socketsOpened;

		virtual void updateDisplay(); //> Update the display string if running in GUI mode
	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** @brief Initialization of the component and startup of browse event scheduling */
		virtual void initialize();

		/** @brief Report final statistics */
		virtual void finish();

		/** @brief Handle incoming messages */
		virtual void handleMessage(cMessage *msg);
	//@}

		/** @name TCP Socket API callbacks */
		//@ {
		virtual bool hasCallback(TCPSocketAPI::CALLBACK_TYPE type); // is this in CB interface?
		virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr); // this happens after each call  to accept.
		virtual void connectCallback(int socket_id, int ret_status, void * myPtr); // this happens after each call to connect
		virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr); // this happens after each call to recv
		//@ }

		/** @name httptServerBase redefinitions */
		//@ {
		virtual httptReplyMessage* handleGetRequest( httptRequestMessage *request, string resource );
		virtual void updateDisplay(); //> Update the display string if running in GUI mode
		//@ }

		/** @name videoServer methods */
		//@ {
		virtual void processDownstreamRequest(int socket_id, cPacket * msg, ConnInfo * data); // handle request from client (or cache)
		//@ }
};

#endif


