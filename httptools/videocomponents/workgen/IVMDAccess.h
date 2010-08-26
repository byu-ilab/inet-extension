//===========================================================================80>
/**
 * @file IVtmdAccess.h
 *
 * IVtmdAccess interface declarations.
 *
 * Created: Aug 25, 2010
 *
 * @todo Add GPL notice.
 */

#ifndef _IVTMDACCESS_H_
#define _IVTMDACCESS_H_

// from inet
#include "InternetTypeDefsWithStandardTypes.h"

/**
 * Specifies functions for accessing and manipulating video meta data structs.
 */
class IVMDAccess
{
public:
	/** @name Destructor */
	//@{

	/** Empty. */
	virtual ~IVMDAccess() {}

	//@}

	/** @name Video Title functions */
	//@{

	virtual struct VideoTitleMetaData getVideoTitleMetaData(str_cref_t video_title) =0;

	//@}

	/** @name Video Segment functions */
	//@{

	/**
	 * Returns the video segment data as contained in the uri.
	 * Throws an error if the uri isn't in the right format.  Does not check if
	 * the uri pertains to an actual segment.
	 */
	virtual struct VideoSegmentMetaData parseVideoSegmentUri(uri_t uri) =0;

	/**
	 * Simple creates a video segment uri according to the provided parameters.
	 * Does not check if the parameters are valid.
	 */
	virtual uri_t createVideoSegmentUri(str_cref_t type, str_cref_t title,
			int quality_level, int segment_number, int fbp = -1, int lbp = -1) =0;

	/**
	 * Determines whether the provided video segment data is valid according to
	 * it's corresponding video title's meta data.
	 */
	virtual bool isVideoSegmentDataValid(const struct VideoSegmentMetaData & vsdata) =0;

	virtual bool isVideoSegmentDataValid(uri_t uri) =0;

	//@}
};

#endif /* _IVTMDACCESS_H_ */
