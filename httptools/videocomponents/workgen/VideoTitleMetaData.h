/*
 * VideoTitleMetaData.h
 *
 *  Created on: Jun 4, 2010
 *      Author: Kevin Black
 */

#ifndef VIDEOTITLEMETADATA_H_
#define VIDEOTITLEMETADATA_H_

struct VideoTitleMetaData
{
	std::string video_title;
	std::string video_type;
	int num_segments;
	int num_rates;
	int quality_interval;
};

#endif /* VIDEOTITLEMETADATA_H_ */
