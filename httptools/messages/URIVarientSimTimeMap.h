/*
 * URIVarientSimTimeMap.h
 *
 *  Created on: Jun 24, 2010
 *      Author: Kevin Black
 */

#ifndef URIVARIENTSIMTIMEMAP_H_
#define URIVARIENTSIMTIMEMAP_H_

#include <string>
#include <StringIntKey.h>

class URIVarientKey : public StringIntKey
{
//protected:
//	std::string _uri;
//	int _varient;


public:
	URIVarientKey(const std::string & uri_value, int varient_value) : StringIntKey(uri_value, varient_value)
	{
//		setStringPart(uri_value);
//		setIntPart(varient_value);
//		_uri = uri_value;
//		_varient = varient_value;
	}
	virtual ~URIVarientKey() {}

	bool operator < (const URIVarientKey & other) const
	{
		// Remember it should behave like < (less than)
		const StringIntKey * translated = &other;

		return StringIntKey::operator<(*translated);
//
//		if (this->_uri < other._uri)
//		{
//			return true;
//		}
//		if (this->_uri == other._uri)
//		{
//			if (this->_varient < other._varient)
//			{
//				return true;
//			}
//			return false;
//		}
//		return false;
	}

	std::string getURI() const { return _get_string_part(); /*_uri; */ }
	int getVarient() const { return _get_int_part(); /*_varient;*/ }
};

struct MsgIdTimestamp
{
	int message_id;
	simtime_t time;

public:
	MsgIdTimestamp() { message_id = -1; time = 0; }
	MsgIdTimestamp(int id, simtime_t t) { message_id = id; time = t; }
	virtual ~MsgIdTimestamp() {}
};

typedef std::map<URIVarientKey, MsgIdTimestamp> URIVarientSimTimeMap;

#endif /* URIVARIENTSIMTIMEMAP_H_ */
