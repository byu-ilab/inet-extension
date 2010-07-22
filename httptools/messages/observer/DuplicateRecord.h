/*
 * DuplicateRecord.h
 *
 *  Created on: Jul 16, 2010
 *      Author: Kevin Black
 */

#ifndef DUPLICATERECORD_H_
#define DUPLICATERECORD_H_

#include <omnetpp.h>
#include <string>
#include <list>
#include <map>

//#include <StringIntKey.h>

//struct DuplicateMessageEvents // TODO rename to DuplicateRecordEntry
//{
//	//std::string source_name;
//	std::list<SimTime> event_times;
//
//	DuplicateMessageEvents() {}
//	//DuplicateMessageEvents(const std::string & sourcename) { source_name = sourcename; }
//	virtual ~DuplicateMessageEvents() {}
//};

//typedef StringIntKey DuplicateRecordKey;

class DuplicateRecordKey
{
private:
	std::string _source;
	std::string _name;
	int _interface;

	void initialize(const std::string & source, const std::string & name, const int & interface);

public:
	DuplicateRecordKey() { initialize("", "", -1); }

	DuplicateRecordKey(const std::string & source, const std::string & name, const int & interface) { initialize(source, name, interface); }

	virtual ~DuplicateRecordKey() {}

	bool operator < (const DuplicateRecordKey & other) const;

	virtual void setSource(const std::string & source) { _source = source; }
	virtual void setName(const std::string & name) { _name = name; }
	virtual void setInterface(const int & interface) { _interface = interface; }

	std::string getSource() const { return _source; }
	std::string getName() const { return _name; }
	int getInterface() const { return _interface; }
};

typedef std::list<SimTime> DuplicateMessageEvents;

typedef std::map<DuplicateRecordKey, DuplicateMessageEvents> DuplicateRecordMap;

#endif /* DUPLICATERECORD_H_ */
