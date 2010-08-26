/**
 * @file VideoTitleMetaData.h
 *
 * Created: June 4, 2010
 *
 * @todo Add GPL notice.
 */

#ifndef _VIDEOMETADATA_H_
#define _VIDEOMETADATA_H_

#define MIN_QUALITY_LEVEL 1
#define MIN_SEGMENT_NUMBER 0

// From standard C++ libraries
#include <string>

struct VideoTitleMetaData
{
	std::string video_title;
	std::string video_type;
	int num_segments;
	int num_quality_levels;
	int quality_interval;
};

struct VideoSegmentMetaData
{
	std::string video_title;
	std::string video_type;
	int quality_level;
	int segment_number;
	int first_byte_position;
	int last_byte_position;
};

#endif /* _VIDEOMETADATA_H_ */
