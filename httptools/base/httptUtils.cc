

// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson LDSS kristjanvj04@ru.is
//
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

#include "httptUtils.h"

#include <sstream>

string trimLeft( string str )
{
	string::iterator i;
	for (i = str.begin(); i != str.end(); i++) {
		  if (!isspace(*i)) {
		      break;
		  }
	}
	if (i == str.end()) {
		  str.clear();
	} else {
		  str.erase(str.begin(), i);
	}
	return str;
}

string trimLeft( string str, string delim )
{
	int pos = str.find(delim);
	if ( pos==-1 ) return str;
	else return str.substr(pos+1,str.size()-pos-1);
}

string trimRight( string str )
{
	string::iterator i;
	for (i = str.end() - 1; ;i--) {
		  if (!isspace(*i)) {
		      str.erase(i + 1, str.end());
		      break;
		  }
		  if (i == str.begin()) {
		      str.clear();
		      break;
		  }
	}
	return str;
}

string trimRight( string str, string delim )
{
	int pos = str.rfind(delim);
	if ( pos==-1 ) return str;
	else return str.substr(0,pos-1);
}

string trim( string str )
{
	str = trimLeft(str);
	str = trimRight(str);
	return str;
}

string extractServerName( const char* path )
{
	std::string www(path);
	int position = www.find("http://");
	if ( position != -1 )
	{
		www = www.erase(0, position);
	}
	else
	{
		position = www.find("https://");
		if ( position != -1 )
		{
			www = www.erase(0, position);
		}
	}

	position = www.find("/");
	if ( position != -1 )
	{
		www = www.substr(0,position);
	}

	return www;
}

string extractResourceName( const char* path )
{
	std::string www(path);
	int position = www.find("http://");
	if ( position != -1 )
	{
		www = www.erase(0, position);
	}
	else
	{
		position = www.find("https://");
		if ( position != -1 )
		{
			www = www.erase(0, position);
		}
	}

	position = www.find("/");
	if ( position != -1 )
		return www.substr(position+1,www.size()-position);
	else
		return "";
}

std::vector<std::string> parseResourceName(string resource)
{
	string path="";
	string resourceName="";
	string extension="";

	int slashpos = resource.rfind("/");
	if ( slashpos!=-1 )
		path=resource.substr(0,slashpos);
	int dotpos = resource.rfind(".");
	if ( dotpos!=-1 )
	{
		resourceName = resource.substr(slashpos+1,dotpos-slashpos-1);
		extension = resource.substr(dotpos+1,resource.size()-dotpos-1);
	}
	else
	{
		resourceName = resource.substr(slashpos+1,resource.size()-slashpos);
	}

	std::vector<std::string> res;
	res.push_back(path);
	res.push_back(resourceName);
	res.push_back(extension);
	return res;
}

std::string getDelimited(string str, string ldelim, string rdelim)
{
	int lpos = str.find(ldelim);
	int rpos;
	if ( rdelim=="" )
		rpos = str.rfind(ldelim);
	else
		rpos = str.rfind(rdelim);
	if ( lpos==-1 || rpos==-1 || lpos==rpos ) return ""; // Not found
	else return str.substr(lpos+1,rpos-lpos-1);
}

WEB_CONTENT_TYPE getResourceCategory(vector<std::string> res)
{
	if ( res.size()==2 )
		return rt_html_page;
	else if (res.size()>2)
		return getResourceCategory(res[2]); // get the category from the extension
	return rt_unknown;
}

WEB_CONTENT_TYPE getResourceCategory(string resourceExt)
{
	if (resourceExt=="" || resourceExt=="htm" || resourceExt=="html")
		return rt_html_page;
	else if (resourceExt=="jpg" || resourceExt=="gif" || resourceExt=="bmp")
		return rt_image;
	else if (resourceExt=="css" || resourceExt=="txt" || resourceExt=="js")
		return rt_text;
	else if (resourceExt=="vtmd")
		return rt_vtmd;
	else if (resourceExt=="vid" || resourceExt=="vidseg" || resourceExt=="vs")
		return rt_vidseg;

	return rt_unknown;
}

string httpVersionAsString(int version)
{
	if (version == HTTP_10)
	{
		return "HTTP/1.0";
	}
	// else
	if (version == HTTP_11)
	{
		return "HTTP/1.1";
	}
	// else
	return "unknown";
}

string makeRequestHeader(RequestMethod method, string uri, HTTPProtocol version)
{
	string header = httpMethodAsString(method) + " " + uri + " " + httpVersionAsString(version);
	return header;
}


string makeResponseHeader(HTTPProtocol version, int code, string reason_phrase)
{
	string header = httpVersionAsString(version) + " " + httpCodeAsString(code) + " ";
	if (reason_phrase.empty())
	{
		header += httpPhraseFromCode(code);
	}
	else
	{
		header += reason_phrase;
	}
	return header;
}

RequestMethod httpMethodFromString(const string & methodstr)
{
	if (methodstr == HTTP_METHOD_OPTIONS)
	{
		return RM_OPTIONS;
	}

	if (methodstr == HTTP_METHOD_GET)
	{
		return RM_GET;
	}

	if (methodstr == HTTP_METHOD_HEAD)
	{
		return RM_HEAD;
	}

	if (methodstr == HTTP_METHOD_POST)
	{
		return RM_POST;
	}

	if (methodstr == HTTP_METHOD_PUT)
	{
		return RM_PUT;
	}

	if (methodstr == HTTP_METHOD_DELETE)
	{
		return RM_DELETE;
	}

	if (methodstr == HTTP_METHOD_TRACE)
	{
		return RM_TRACE;
	}

	if (methodstr == HTTP_METHOD_CONNECT)
	{
		return RM_CONNECT;
	}

	return RM_NONE;
}

string httpMethodAsString(int method)
{
	switch(method)
	{
	case RM_OPTIONS:	return HTTP_METHOD_OPTIONS;
	case RM_GET: 		return HTTP_METHOD_GET;
	case RM_HEAD: 		return HTTP_METHOD_HEAD;
	case RM_POST: 		return HTTP_METHOD_POST;
	case RM_PUT: 		return HTTP_METHOD_PUT;
	case RM_DELETE: 	return HTTP_METHOD_DELETE;
	case RM_TRACE: 		return HTTP_METHOD_TRACE;
	case RM_CONNECT: 	return HTTP_METHOD_CONNECT;

	default: 			return "";
	}
}


string httpCodeAsString(int code)
{
	if (code < 100 || 505 < code)
		return "";

	string decirep = "";
	int remainder = 0;
	int temp = code;

	while (temp != 0)
	{
		remainder = temp % 10;
		temp = (temp - remainder) / 10;

		switch(remainder)
		{
		case 0: decirep = decirep + "0"; break;
		case 1: decirep = decirep + "1"; break;
		case 2: decirep = decirep + "2"; break;
		case 3: decirep = decirep + "3"; break;
		case 4: decirep = decirep + "4"; break;
		case 5: decirep = decirep + "5"; break;
		case 6: decirep = decirep + "6"; break;
		case 7: decirep = decirep + "7"; break;
		case 8: decirep = decirep + "8"; break;
		case 9: decirep = decirep + "9"; break;
		}
	}
	// invert the string and return it
	return getReverseString(decirep);
}

string getReverseString(const string & toreverse)
{
	string retstr = "";
	string::iterator itr = retstr.end();
	//for (int i = toreverse.size()-1; i >= 0; i--)
	for (int i = 0; i < (int) toreverse.size(); i++)
	{
		itr = retstr.insert(itr, toreverse.at(i));
	}
	return retstr;
}

string httpPhraseFromCode(int code)
{
	switch(code)
	{
		case HTTP_CODE_200: return HTTP_PHRASE_200;
		case HTTP_CODE_206: return HTTP_PHRASE_206;
		case HTTP_CODE_400: return HTTP_PHRASE_400;
		case HTTP_CODE_404: return HTTP_PHRASE_404;
		case HTTP_CODE_416: return HTTP_PHRASE_416;

		default: 			return "";
	}
}

double safeatof(const char* strval, double defaultVal)
{
	try
	{
		return atof(strval);
	}
	catch(...)
	{
		return defaultVal;
	}
}

int safeatoi(const char* strval, int defaultVal)
{
	try
	{
		return atoi(strval);
	}
	catch(...)
	{
		return defaultVal;
	}
}

int safeatobool(const char* strval, bool defaultVal)
{
	try
	{
		return ( strcmp(strval,"TRUE")==0 || strcmp(strval,"true")==0 );
	}
	catch(...)
	{
		return defaultVal;
	}
}

std::vector<std::string> splitFile(string fileName)
{
	string path="";
	string file="";
	string ext="";

	int slashpos = fileName.rfind("/");
	if ( slashpos==-1 )
		slashpos = fileName.rfind("\\");
	if ( slashpos!=-1 )
	{
		path = fileName.substr(0,slashpos+1);
		fileName = fileName.substr(slashpos+1,fileName.size()-slashpos-1);
	}

	int dotpos = fileName.find(".");
	if ( dotpos!=-1 )
	{
		ext = fileName.substr(dotpos+1,fileName.size()-dotpos-1);
		file = fileName.substr(0,dotpos);
	}
	else
	{
		file = fileName;
	}

	std::vector<std::string> res;
	res.push_back(path);
	res.push_back(file);
	res.push_back(ext);
	return res;
}

bool fileExists( const char *file )
{
	#if WIN32
	# define CHECKACCESS _access
	# define CHECKRIGHTS 0
	#else
	# define CHECKACCESS access
	# define CHECKRIGHTS F_OK
	#endif
	return CHECKACCESS(file,CHECKRIGHTS) == 0;
}


