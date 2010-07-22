/*
 * DuplicateRecord.cc
 *
 *  Created on: Jul 19, 2010
 *      Author: Kevin Black
 */

#include "DuplicateRecord.h"

void DuplicateRecordKey::initialize(const std::string & source, const std::string & name, const int & interface)
{
	setSource(source);
	setName(name);
	setInterface(interface);
}

bool DuplicateRecordKey::operator < (const DuplicateRecordKey & other) const
{
	// Remember it should behave like < (less than)

	if (this->_source < other._source)
	{
		return true;
	}
	if (this->_source == other._source)
	{
		if (this->_interface < other._interface)
		{
			return true;
		}
		if (this->_interface == other._interface)
		{
			if (this->_name < other._name)
			{
				return true;
			}
		}
	}
	return false;
}
