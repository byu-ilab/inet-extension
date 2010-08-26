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

// from inet
#include "InternetTypeDefsWithStandardTypes.h"

// From standard C++ libraries
#include <string>

struct VideoTitleMetaData
{
public:
	std::string video_title;
	std::string video_type;
	int num_segments;
	double segment_duration;
	int num_quality_levels;
	int quality_interval;

	VideoTitleMetaData();
	VideoTitleMetaData(uri_t uri);
	VideoTitleMetaData(str_cref_t title, str_cref_t type, int num_segments,
			double segment_duration, int num_quality_levels, int quality_interval);
	virtual ~VideoTitleMetaData();

	virtual uri_t getURI();

	static bool isValid(const struct VideoTitleMetaData vtmd);
	static bool isValid(uri_cref_t uri);
};

struct VideoSegmentMetaData
{
public:
	std::string video_title;
	std::string video_type;
	int quality_level;
	int segment_number;
	int first_byte_position;
	int last_byte_position;

	VideoSegmentMetaData();
	VideoSegmentMetaData(uri_t uri);
	VideoSegmentMetaData(str_cref_t title, str_cref_t type, int quality_level,
			int segment_number, int fbp = -1, int lbp = -1);
	virtual ~VideoSegmentMetaData();

	virtual uri_t getURI() const;

	static bool isValid(const struct VideoSegmentMetaData & vsmd);
	static bool isValid(uri_cref_t uri);
};

#endif /* _VIDEOMETADATA_H_ */
