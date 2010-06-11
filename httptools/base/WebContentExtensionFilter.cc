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

#include "WebContentExtensionFilter.h"

WebContentExtensionFilter::WebContentExtensionFilter() : _ext_type_map()
{

}

WebContentExtensionFilter::~WebContentExtensionFilter()
{

}

void WebContentExtensionFilter::configureFromXML(const std::string & input_filename)
{
	if (input_filename.empty())
	{
		return;
	}

	XMLScanner scanner;
	scanner.Scan(input_filename, true);

	const XMLTag * tag = (const XMLTag *) scanner.NextToken();

	if (tag->GetTagName() != XML_WCEF_ROOT)
	{
		throw cRuntimeError("Invalid XML.  Expected %s as the root tag.", XML_WCEF_ROOT);
	}

	std::string ext;
	std::string type_str;
	WEB_CONTENT_TYPE type;

	while (!scanner.IsTokenStreamAtEnd())
	{
		tag = (const XMLTag *) scanner.NextToken();
		if (tag->GetTagName() == XML_WCEF_ROOT)
		{
			break;
		}
		else if (tag->GetTagName() != XML_WCEF_EXT_TAG)
		{
			throw cRuntimeError("Invalid XML.  Expected %s tag got %s tag.", XML_WCEF_EXT_TAG, tag->GetTagName().c_str());
		}

		ext = tag->GetAttributeValue(XML_WCEF_EXT_VALUE_ATTRIB);
		if (ext.empty())
		{
			throw cRuntimeError("Invalid XML.  Extension doesn't have a value.");
		}

		type_str = tag->GetAttributeValue(XML_WCEF_EXT_TYPE_ATTRIB);
		type = rt_unknown;

		if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_HTML)
		{
			type = rt_html_page;
		}
		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_IMAGE)
		{
			type = rt_image;
		}
		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_TEXT)
		{
			type = rt_text;
		}
		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_VTMD)
		{
			type = rt_vtmd;
		}
		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_VIDSEG)
		{
			type = rt_vidseg;
		}

		addExtension(ext, type);
	}
}

//void WebContentExtensionFilter::configureFromXML(cXMLElement * root)
//{
//	std::string root_tag_name = root->getTagName();
//	if (root_tag_name != XML_WCEF_ROOT)
//	{
//		// try to find it
//		root = root->getFirstChildWithTag(XML_WCEF_ROOT);
//		if (!root)
//		{
//			throw cRuntimeError("Invalid XML.  Expected %s as the root tag.", XML_WCEF_ROOT);
//		}
//	}
//
//	cXMLElementList ext_elements = root->getChildrenByTagName(XML_WCEF_EXT_TAG);
//
//	cXMLElementList::iterator ee_itr;
//	std::string ext;
//	std::string type_str;
//	WEB_CONTENT_TYPE type;
//
//	for (ee_itr = ext_elements.begin(); ee_itr < ext_elements.end(); ee_itr++)
//	{
//		ext = (*ee_itr)->getAttribute(XML_WCEF_EXT_VALUE_ATTRIB);
//		if (ext.empty())
//		{
//			throw cRuntimeError("Invalid XML.  Extension doesn't have a name.");
//		}
//		type_str = (*ee_itr)->getAttribute(XML_WCEF_EXT_TYPE_ATTRIB);
//		type = rt_unknown;
//
//		if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_HTML)
//		{
//			type = rt_html_page;
//		}
//		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_IMAGE)
//		{
//			type = rt_image;
//		}
//		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_TEXT)
//		{
//			type = rt_text;
//		}
//		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_VTMD)
//		{
//			type = rt_vtmd;
//		}
//		else if (type_str == XML_WCEF_EXT_TYPE_ATTRIB_VIDSEG)
//		{
//			type = rt_vidseg;
//		}
//
//		addExtension(ext, type);
//	}
//}

//bool WebContentExtensionFilter::writeXML(const std::string & output_filename)
//{
//	std::ofstream outfile(output_filename.c_str());
//
//	if (outfile.bad() || outfile.fail())
//	{
//		return false;
//	}
//
//	outfile << XML_TAG_BEGIN << XML_WCEF_ROOT << XML_TAG_CLOSE << endl;
//
//
//}

/*
 * Returns true if the extension is added and/or it related content type
 * is changed.  Returns false if the extension is in the map and already
 * mapped to the indicated type.
 */
bool WebContentExtensionFilter::addExtension(const std::string & ext, WEB_CONTENT_TYPE type)
{
	_et_itr = _ext_type_map.find(ext);
	if (_et_itr != _ext_type_map.end())
	{
		if (_et_itr->second == type)
		{
			return false;
		}
		_et_itr->second = type;
		return true;
	}
	_ext_type_map[ext] = type;
	return true;
}
/*
 * Removes the extension.  Returns true if the extension was in the map
 * and false if it was not.
 */
bool WebContentExtensionFilter::removeExtension(const std::string & ext)
{
	_et_itr = _ext_type_map.find(ext);
	if (_et_itr != _ext_type_map.end())
	{
		_ext_type_map.erase(_et_itr);
		return true;
	}
	return false;
}
/*
 * Removes all extensions related with the specified type.  Returns true
 * if there were any extensions related with the indicated type.
 */
bool WebContentExtensionFilter::removeType(WEB_CONTENT_TYPE type)
{
	ExtSet extensions = getExtensions(type);
	if (extensions.empty())
	{
		return false;
	}
	ExtSet::iterator ext_itr = extensions.begin();
	for (ext_itr = extensions.begin(); ext_itr != extensions.end(); ext_itr++)
	{
		_ext_type_map.erase(*ext_itr);
	}
	return true;
//
//	bool type_existed = false;
//	for (_et_itr = _ext_type_map.begin(); _et_itr != _ext_type_map.end(); _et_itr++)
//	{
//		if (_et_itr->second == type)
//		{
//			type_existed = true;
//			_et_itr = _ext_type_map.erase(_et_itr);
//		}
//	}
//	return type_existed;
}

WEB_CONTENT_TYPE WebContentExtensionFilter::getType(const std::string & ext)// const
{
	_et_itr = _ext_type_map.find(ext);
	if (_et_itr == _ext_type_map.end())
	{
		throw cRuntimeError("getType: the extension is not in the filter");
	}

	return (WEB_CONTENT_TYPE) _et_itr->second;
}
ExtSet WebContentExtensionFilter::getExtensions(WEB_CONTENT_TYPE type)// const
{
	ExtSet extensions;
	for (_et_itr = _ext_type_map.begin(); _et_itr != _ext_type_map.end(); _et_itr++)
	{
		if (_et_itr->second == type)
		{
			extensions.insert(_et_itr->first);
		}
	}
	return extensions;
}

bool WebContentExtensionFilter::containsExtension(const std::string & ext)// const
{
	_et_itr = _ext_type_map.find(ext);
	return (_et_itr != _ext_type_map.end()) ? true : false;
}

bool WebContentExtensionFilter::containsType(WEB_CONTENT_TYPE type)// const
{
	_et_itr = _ext_type_map.begin();
	while (_et_itr != _ext_type_map.end())
	{
		if (_et_itr->second == type)
		{
			return true;
		}
		_et_itr++;
	}
	return false;
}

int  WebContentExtensionFilter::numberOfExtensions()// const
{
	return _ext_type_map.size();
}


