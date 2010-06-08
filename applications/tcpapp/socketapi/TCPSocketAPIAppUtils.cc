/*
 * TCPSocketAPIAppUtils.cc
 *
 *  Created on: Jun 7, 2010
 *      Author: black
 */

#include "TCPSocketAPIAppUtils.h"

TCPSocketAPI * findTCPSocketAPI(cSimpleModule * app_module)
{
    std::string api_obj_name = app_module->par("socketapi").stringValue();
    if (api_obj_name.empty())
    {
    	throw cRuntimeError(app_module, "the parameter socketapi is not specified!");
    }
    return check_and_cast<TCPSocketAPI *>(app_module->getParentModule()->getSubmodule(api_obj_name.c_str()));
}
