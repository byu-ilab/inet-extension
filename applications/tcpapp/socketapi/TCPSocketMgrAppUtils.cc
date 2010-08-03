/**
 * \file TCPSocketMgrAppUtils.cc
 *
 * Created: June 7, 2010
 * Author: Kevin Black
 */

#include "TCPSocketMgrAppUtils.h"

TCPSocketMgr * findTCPSocketMgr(cSimpleModule * app_module)
{
    std::string api_obj_name = app_module->par("socketapi").stringValue();
    if (api_obj_name.empty())
    {
    	throw cRuntimeError(app_module, "the parameter socketapi is not specified!");
    }
    return check_and_cast<TCPSocketMgr *>(app_module->getParentModule()->getSubmodule(api_obj_name.c_str()));
}
