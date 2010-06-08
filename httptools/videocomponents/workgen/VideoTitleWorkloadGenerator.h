// Author: Kevin Black

#ifndef __VIDEOTITLEWORKLOADGENERATOR_H__
#define __VIDEOTITLEWORKLOADGENERATOR_H__

#include <omnetpp.h>
#include <string.h>

#include "VideoTitleMetaData.h"

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
class VideoTitleWorkloadGenerator : public cSimpleModule
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

	// returns a video title id according to the Zipf popularity distribution
	virtual int getNextVideoTitle();

	virtual struct VideoTitleMetaData getMetaData(int video_title_id);
	virtual struct VideoTitleMetaData getMetaData(const std::string & video_title);

	virtual std::string getMetaDataFilePath(int video_title_id);

	virtual std::string getVideoTitleAsString(int video_title_id);
	// returns -1 if there is an error in the video title
	virtual int getVideoTitleAsInt(const std::string & video_title);

protected:
	/** @name Overridden from cSimpleModule */
	//@{
    virtual void initialize();

    /// The workload generator could conceivably change the popularity distribution
    /// during the simulation if that was desired.  Or perhaps could handle
    /// requests to log information or results of some kind.
    virtual void handleMessage(cMessage *msg);
    //@}

    /** @name Protected VideoTitleWorkloadGenerator functions */
    //@{

    std::string getReverseString(const std::string & toreverse);

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
