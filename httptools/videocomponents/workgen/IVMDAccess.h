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
#include "VideoMetaData.h"

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

	/** @name Video Title Accessors */
	//@{

	virtual struct VideoTitleMetaData getVTMD(str_cref_t video_title) =0;
	virtual struct VideoTitleMetaData getVTMD(vidt_id_t id) =0;

	//@}
};

#endif /* _IVTMDACCESS_H_ */
