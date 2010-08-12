/**
 * \file HTTPMsgDefs.h
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

#endif /* _HTTPMSGDEFS_H_ */
