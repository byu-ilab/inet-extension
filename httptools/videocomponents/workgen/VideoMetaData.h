//===========================================================================80>
/**
 * @file VideoMetaData.h
 *
 * VideoTitleMetaData and VideoSegmentMetaData struct declarations.
 *
 * Created: Aug 4, 2010
 *
 * @todo Add GPL notice.
 */

#ifndef _VIDEOMETADATA_H_
#define _VIDEOMETADATA_H_

#define MIN_QUALITY_LEVEL 1
#define MIN_SEGMENT_NUMBER 0

// from inet
#include "InternetTypeDefsWithStandardTypes.h"
#include "HTTPMsgDefs.h"

// From standard C++ libraries
#include <string>

typedef int vidt_id_t;

#define VIDTITLE_NULL ((vidt_id_t) -1)

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

	virtual void setFromURI(uri_cref_t uri);
	virtual uri_t getURI() const;
	virtual bool isValid() const;

	static bool isValid(const struct VideoTitleMetaData & vtmd);
	static bool isValid(uri_cref_t uri);
	static vidt_id_t randomID();
	static str_t asTitle(vidt_id_t id);
	static vidt_id_t asID(str_cref_t title);

private:
	void initialize(str_cref_t title, str_cref_t type, int segments,
			double seg_duration, int quality_levels, int interval);
};

/**
 * @todo make it dependent on VideoTitleMetaData
 *
 * @todo make fields private so that they can be validated against
 * the associated VideoTitleMetaData
 */
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
	VideoSegmentMetaData(str_cref_t title, str_cref_t type, int level,
			int number, int fbp = BRS_UNSPECIFIED, int lbp = BRS_UNSPECIFIED);
	virtual ~VideoSegmentMetaData();

	virtual void setFromURI(uri_cref_t uri);
	virtual uri_t getURI() const;
	virtual bool pertainsTo(const struct VideoTitleMetaData & vtmd) const;

	static uri_t createURI(str_cref_t title, str_cref_t type, int level,
			int number, int fbp = BRS_UNSPECIFIED, int lbp = BRS_UNSPECIFIED);

private:
	void initialize(str_cref_t title, str_cref_t type, int quality_level,
			int segment_number, int fbp = BRS_UNSPECIFIED, int lbp = BRS_UNSPECIFIED);
};

#endif /* _VIDEOMETADATA_H_ */
