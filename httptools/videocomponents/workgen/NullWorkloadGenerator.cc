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

#include "NullWorkloadGenerator.h"

Define_Module(NullWorkloadGenerator);

void NullWorkloadGenerator::initialize()
{
    // TODO - Generated method body
}

void NullWorkloadGenerator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
/** @name Overridden from IFileSystem */
//@{
bool NullWorkloadGenerator::hasResource(string resource) {
	return true;
}
int NullWorkloadGenerator::getResourceSize(httptRequestMessage * message, string resource) {
	return message->entitySize();
}
