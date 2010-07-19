/*
 * VideoTitleMetaData.h
 *
 *  Created on: Jun 4, 2010
 *      Author: Kevin Black
 */

#ifndef VIDEOTITLEMETADATA_H_
#define VIDEOTITLEMETADATA_H_

#define MIN_QUALITY_LEVEL 1
#define MIN_SEGMENT_NUMBER 0
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

#endif /* VIDEOTITLEMETADATA_H_ */
