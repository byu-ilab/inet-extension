//===========================================================================80>
/**
 * @file InternetTypeDefsWithStandardTypes.h
 *
 * Created: Aug 5, 2010
 * Author: Kevin Black
 *
 * @todo Add GPL notice.
 */

#ifndef _INTERNETTYPEDEFSWITHSTANDARDTYPES_H_
#define _INTERNETTYPEDEFSWITHSTANDARDTYPES_H_

// From standard C++ libraries
#include <string>

// Standard string defs
typedef std::string		str_t;
typedef const str_t &	str_cref_t;

// Standard count defs
typedef int				counter_t;

#define COUNTER_NULL ((counter_t) -1)
#define COUNTER_MAX  ((counter_t) 2147483647)
#define COUNTER_MIN  ((counter_t) 0)

// Internet type defs
typedef int				bytecount_t;
typedef str_t 			address_t;
typedef str_cref_t		address_cref_t;
typedef int 			port_t;
typedef int 			socket_id_t;

typedef address_t		ip_address_t;
typedef address_cref_t 	ip_address_cref_t;
typedef address_t		domain_name_t;
typedef address_cref_t 	domain_name_cref_t;

typedef str_t			uri_t;
typedef str_cref_t		uri_cref_t;

#define SOCKET_ID_NULL ((socket_id_t) -1)
#define BYTECOUNT_NULL ((bytecount_t) -1)
#define PORT_NULL 	   ((port_t) -1)
#define ADDRESS_NULL   ""
#define URI_NULL       ""

// inclusive
#define TCP_PORT_RANGE_MIN 0
#define TCP_PORT_RANGE_MAX 65535

#endif /* _INTERNETTYPEDEFSWITHSTANDARDTYPES_H_ */
