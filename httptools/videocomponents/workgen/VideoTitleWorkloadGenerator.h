// Author: Kevin Black

#ifndef __VIDEOTITLEWORKLOADGENERATOR_H__
#define __VIDEOTITLEWORKLOADGENERATOR_H__

// from omnetpp
#include <omnetpp.h>

// from inet
#include "VideoMetaData.h"
#include "IVMDAccess.h"

// from standard C++ libraries
#include <sstream>


struct VideoTitlePopularity
{
	int video_title_id;
	double popularity;
	int times_requested;
};


/// To Improve: in the event that the configuration should be
/// read in from a configuration file provide the option
/// to output sequential video titles instead of random
/// ones as that will only require the starting video title
/// to be recorded in the vtwg.cfg file instead of every
/// video title, and/or just output the files in a sequence
/// that doesn't have to correspond to their actual title, and
/// rather would pertain to their rank.  Also record the zipf
/// exponent and rank offset in the cfg file (perhaps even the
/// zipf normalization constant).
class VideoTitleWorkloadGenerator : public cSimpleModule, public IVMDAccess
{
protected:
	/// the directory to write meta data out to
	/// currently only used for testing
	std::string _metadata_directory;

    /// rank (index) to popularity for ranked item
    /// supports getNextVideoTitle
    VideoTitlePopularity * _popularity_array;
    int _popularity_array_size;

    /// maps video title id to video title meta data
    /// supports getMetaData and getMetaDataFilePath
    std::map<int, VideoTitleMetaData> _video_title_map;

    // for testing
    int _num_requests_to_make;
    int _num_requests_made;

public:
	VideoTitleWorkloadGenerator();
	virtual ~VideoTitleWorkloadGenerator();

	/** @name Video Title functions */
	//@{
	// returns a video title id according to the Zipf popularity distribution
	virtual int getNextVideoTitle();

	virtual struct VideoTitleMetaData getVideoTitleMetaData(int video_title_id);


	virtual struct VideoTitleMetaData getVideoTitleMetaData(str_cref_t video_title);

	virtual str_t getMetaDataFilePath(int video_title_id);

	virtual str_t getVideoTitleAsString(int video_title_id);
	// returns -1 if there is an error in the video title
	virtual int getVideoTitleAsInt(str_cref_t video_title);
	//@}

	/** @name Video Segment functions */
	//@{
	/*
	 * Returns the video segment data as contained in the uri.
	 * Throws an error if the uri isn't in the right format.  Does not check if
	 * the uri pertains to an actual segment.
	 */
	virtual struct VideoSegmentMetaData parseVideoSegmentUri(uri_t uri);

	/*
	 * Simple creates a video segment uri according to the provided parameters.
	 * Does not check if the parameters are valid.
	 */
	virtual uri_t createVideoSegmentUri(str_cref_t type, str_cref_t title,
			int quality_level, int segment_number, int fbp = -1, int lbp = -1);

	/*
	 * Determines whether the provided video segment data is valid according to
	 * it's corresponding video title's meta data.
	 */
	virtual bool isVideoSegmentDataValid(const struct VideoSegmentMetaData & vsdata);

	virtual bool isVideoSegmentDataValid(uri_t uri);
	//@}

protected:
	/** @name Overridden from cSimpleModule */
	//@{
    virtual void initialize();

    /// The workload generator could conceivably change the popularity distribution
    /// during the simulation if that was desired.  Or perhaps could handle
    /// requests to log information or results of some kind.
    virtual void handleMessage(cMessage *msg);

    virtual void finish();
    //@}

    /** @name Protected VideoTitleWorkloadGenerator functions */
    //@{

    //std::string getReverseString(const std::string & toreverse);

    virtual void configureFromInput(const std::string & input_file);

    /// need to write a file that contains number of title info so as to support
    /// reading the configuration back in, that is to allocate heap space for the
    /// _record_array
    /// To Improve: change the ASSERT to cRuntimeErrors which will make more sense
    /// to the user (only if this is going to be distributed)
    virtual void generateConfiguration();
    //@}
};

#endif
