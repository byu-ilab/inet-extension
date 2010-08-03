/**
 * \file VideoTitleMetaData.h
 *
 * Created: June 4, 2010
 * Author: Kevin Black
 *
 * @todo Add GPL notice.
 */

#ifndef __INET__VIDEOMETADATA_H_
#define __INET__VIDEOMETADATA_H_

#define MIN_QUALITY_LEVEL 1
#define MIN_SEGMENT_NUMBER 0

// From emulation
#include "INETDefs.h"

// From standard C++ libraries
#include <string>

struct INET_API VideoTitleMetaData
{
	std::string video_title;
	std::string video_type;
	int num_segments;
	int num_quality_levels;
	int quality_interval;
};

struct INET_API VideoSegmentMetaData
{
	std::string video_title;
	std::string video_type;
	int quality_level;
	int segment_number;
	int first_byte_position;
	int last_byte_position;
};

#endif /* __EMULATION__VIDEOMETADATA_H_ */
