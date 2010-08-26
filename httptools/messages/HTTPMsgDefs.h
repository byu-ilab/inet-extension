/**
 * @file HTTPMsgDefs.h
 *
 * Created: Aug 4, 2010
 * Author: Kevin Black
 *
 * @todo Add GPL notice.
 */

#ifndef _HTTPMSGDEFS_H_
#define _HTTPMSGDEFS_H_

/**
 * Enumerates known HTTP protocol versions used in the simulator.
 */
enum HTTPProtocol
{
    HTTP_10 = 10,
    HTTP_11 = 11
};

/**
 * Enumerates standard HTTP request methods.
 * see http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html
 */
enum RequestMethod
{
    RM_NONE = -1, 	/**< Signals that no request method has been set. */
    RM_OPTIONS = 0,	/**< OPTIONS request method. */
    RM_GET = 1,		/**< GET request method. */
    RM_HEAD = 2,	/**< HEAD request method. */
    RM_POST = 3,	/**< POST request method. */
    RM_PUT = 4,		/**< PUT request method. */
    RM_DELETE = 5,	/**< DELETE request method. */
    RM_TRACE = 6,	/**< TRACE request method. */
    RM_CONNECT = 7	/**< CONNECT request method. */
};

/**
 * Enumerates special states for byte count variables used to simulatate
 * HTTP "Range" and "Content-Range" headers.
 */
enum ByteRangeState
{
    BRS_UNSPECIFIED = -1,
    BRS_ASTERISK = -2
};


// currently identified HTTP request method names
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

#endif /* _HTTPMSGDEFS_H_ */
