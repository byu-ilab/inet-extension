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

// from inet
#include "AccessTCPSocketMgrApp.h"
#include "TCPSocketMgr.h"
#include "TCPSocketMgrAppUtils.h"

// from omnetpp extension
#include "DebugDef.h"

#define DEBUG_CLASS true

Define_Module(AccessTCPSocketMgrApp);

void AccessTCPSocketMgrApp::initialize()
{
	LOG_DEBUG_LN("module name: "<<getName());
	LOG_DEBUG_LN("module full name: "<<getFullName());
	LOG_DEBUG_LN("module full path: "<<getFullPath());
	_socketmgr = findTCPSocketMgr(this);
}

