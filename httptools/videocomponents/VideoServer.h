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

#ifndef __VIDEOSERVER_H
#define __VIDEOSERVER_H

#include <omnetppextension.h>

#include "httptServerBase.h"
#include "TCPSocketAPI.h"
#include "VideoTitleWorkloadGenerator.h"
#include "TCPSocketAPIAppUtils.h"
#include "DuplicateHttpMessageNameObserver.h"

/**
 *
 * @version 1.0
 * @author  Travis Andelin
 * @contributor Kevin Black
 */

enum VSMessageType { START = 1 };

class INET_API VideoServer : public httptServerBase, public TCPSocketAPI::CallbackInterface
{

	protected:
		// internals
		httptController *controller; // Reference to central controller object.
		TCPSocketAPI * tcp_api;
		VideoTitleWorkloadGenerator * workload_generator;

		// stats
		unsigned long socketsBroken;
		unsigned long socketsOpened;
		unsigned long requestsReceived;

		bool shouldTrackDupHttpMsgNames;

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
		virtual void acceptCallback  (int socket_id, int ret_status, void * yourPtr); // this happens after each call  to accept.
		//virtual void connectCallback(int socket_id, int ret_status, void * myPtr); // this happens after each call to connect
		virtual void recvCallback(int socket_id, int ret_status, cPacket * msg, void * myPtr); // this happens after each call to recv
		//@ }

		/** @name httptServerBase redefinitions */
		//@ {
		virtual httptReplyMessage* handleGetRequest( httptRequestMessage *request, string resource );
		virtual void updateDisplay();
		//@ }

		virtual void closeSocket(int socket_id);

		virtual void emitMessageEvent(const cMessage * msg, const int & id);
};

#endif /* __VIDEOSERVER_H */


