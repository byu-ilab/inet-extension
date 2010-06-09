
// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
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

#ifndef __httpUtils_H_
#define __httpUtils_H_

#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "httptMessages_m.h"

#if WIN32
# include <io.h>
# include <stdio.h>
#else
# include <unistd.h>
#endif

// currently identified HTTP request methods
#define HTTP_METHOD_OPTIONS "OPTIONS"
#define HTTP_METHOD_GET 	"GET"
#define HTTP_METHOD_HEAD	"HEAD"
#define HTTP_METHOD_POST	"POST"
#define HTTP_METHOD_PUT		"PUT"
#define HTTP_METHOD_DELETE	"DELETE"
#define HTTP_METHOD_TRACE	"TRACE"
#define HTTP_METHOD_CONNECT	"CONNECT"

// currently supported HTTP codes and phrases
#define HTTP_CODE_MIN	100
#define HTTP_CODE_MAX	505

#define HTTP_CODE_200 	200
#define HTTP_PHRASE_200 "OK"
#define HTTP_CODE_206 	206
#define HTTP_PHRASE_206 "PARTIAL CONTENT"

#define HTTP_CODE_400 	400
#define HTTP_PHRASE_400 "ERROR"
#define HTTP_CODE_404 	404
#define HTTP_PHRASE_404 "NOT FOUND"
#define HTTP_CODE_416 	416
#define HTTP_PHRASE_416 "REQUESTED RANGE NOT SATISFIABLE"

/** @brief type of returned content. Used in http responses. */
enum CONTENT_TYPE_ENUM {rt_unknown,rt_none,rt_html_page,rt_image,rt_text,rt_vtmd, rt_vidseg};

using namespace std;

string trimLeft( string str );
string trimRight( string str );
string trimLeft( string str, string delim );
string trimRight( string str, string delim );
string trim( string str );
string extractServerName( const char *path );
string extractResourceName( const char *path );
std::string getDelimited(string str, string ldelim, string rdelim="");
std::vector<std::string> parseResourceName(string resource);
CONTENT_TYPE_ENUM getResourceCategory(vector<std::string> res);
CONTENT_TYPE_ENUM getResourceCategory(string resourceExt);

// returns RM_NONE if the methodstr doesn't contain a known http method
RequestMethod httpMethodFromString(const string & methodstr);

// returns an empty string if the method is unknown
string httpMethodAsString(int method);

// returns an emtpy string if the code is out of the defined status code range (currently 100 - 505)
string httpCodeAsString(int code);

// returns an empty string if the code is not recognized
string httpPhraseFromCode(int code);

string getReverseString(const string & toreverse);

double safeatof(const char* strval, double defaultVal=0.0);
int safeatoi(const char* strval, int defaultVal=0);
int safeatobool(const char* strval, bool defaultVal=false);

std::vector<std::string> splitFile(string fileName);
bool fileExists( const char *file );

#endif /* __httpUtils_H_ */
