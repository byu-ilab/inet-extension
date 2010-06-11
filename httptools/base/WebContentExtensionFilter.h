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

#ifndef WEBCONTENTEXTENSIONFILTER_H_
#define WEBCONTENTEXTENSIONFILTER_H_

#include <omnetpp.h>
#include <map>
#include <set>
#include "httptUtils.h"
#include <fstream>
#include "XMLDefs.h"
#include "XMLIO.h"

#define XML_WCEF_ROOT 					"wcef-config"
#define XML_WCEF_EXT_TAG 				"extension"
#define XML_WCEF_EXT_VALUE_ATTRIB 		"value"
#define XML_WCEF_EXT_TYPE_ATTRIB 		"type"
#define XML_WCEF_EXT_TYPE_ATTRIB_HTML	"html"
#define XML_WCEF_EXT_TYPE_ATTRIB_IMAGE	"image"
#define XML_WCEF_EXT_TYPE_ATTRIB_TEXT	"text"
#define XML_WCEF_EXT_TYPE_ATTRIB_VTMD	"vtmd"
#define XML_WCEF_EXT_TYPE_ATTRIB_VIDSEG	"vidseg"

typedef std::map<std::string, int> ExtTypeMap;
typedef std::set<std::string> ExtSet;

class WebContentExtensionFilter
{
protected:
	// maps extension strings to web content types
	ExtTypeMap _ext_type_map;
	ExtTypeMap::iterator _et_itr;
	ExtTypeMap::const_iterator _cet_itr;

public:
	WebContentExtensionFilter();
	virtual ~WebContentExtensionFilter();

	virtual void configureFromXML(const std::string & input_filename);

	/*
	 * Returns true if the file was written successfully and false if it wasn't.
	 */
	//virtual bool writeXML(const std::string & output_filename);

	/*
	 * Returns true if the extension is added and/or it related content type
	 * is changed.  Returns false if the extension is in the map and already
	 * mapped to the indicated type.
	 */
	virtual bool addExtension(const std::string & ext, WEB_CONTENT_TYPE type);
	/*
	 * Removes the extension.  Returns true if the extension was in the map
	 * and false if it was not.
	 */
	virtual bool removeExtension(const std::string & ext);
	/*
	 * Removes all extensions related with the specified type.  Returns true
	 * if there were any extensions related with the indicated type.
	 */
	virtual bool removeType(WEB_CONTENT_TYPE type);

	/*
	 * Throws a cRuntimeError if the ext does not exist.  Use containsExtension
	 * to check before calling this function.
	 */
	virtual WEB_CONTENT_TYPE getType(const std::string & ext);// const;
	/*
	 * Returns an empty set if there are no extension related with the indicated type.
	 */
	virtual ExtSet getExtensions(WEB_CONTENT_TYPE type);// const;

	virtual bool containsExtension(const std::string & ext);// const;
	virtual bool containsType(WEB_CONTENT_TYPE type);// const;
	virtual int  numberOfExtensions();// const;
};

#endif /* WEBCONTENTEXTENSIONFILTER_H_ */
