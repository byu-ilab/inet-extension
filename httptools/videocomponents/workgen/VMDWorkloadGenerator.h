//===========================================================================80>
/**
 * @file VMDWorkloadGenerator.h
 *
 * VMDWorkloadGenerator class declarations
 *
 * Created: Aug 26, 2010
 *
 * Based on previous VideoTitleWorkloadGenerator dating from May 2010.
 *
 * @todo Add GPL notice.
 */

#ifndef _VMDWORKLOADGENERATOR_H_
#define _VMDWORKLOADGENERATOR_H_

// from inet
#include "VideoMetaData.h"
#include "IVMDAccess.h"
#include "PopularizedResourceCollection.h"
#include "IFileSystem.h"
// from omnetp
#include <omnetpp.h>
// from standard C++ libraries
#include <map>
#include <string>
typedef VideoTitleMetaData * vtmd_ptr_t;

/** Maps video title id to video title meta data. */
typedef std::map<vidt_id_t, vtmd_ptr_t> ID_VTMD_Map;

using namespace std;
/**
 * To Improve: in the event that the configuration should be
 * read in from a configuration file provide the option
 * to output sequential video titles instead of random
 * ones as that will only require the starting video title
 * to be recorded in the vtwg.cfg file instead of every
 * video title, and/or just output the files in a sequence
 * that doesn't have to correspond to their actual title, and
 * rather would pertain to their rank.  Also record the zipf
 * exponent and rank offset in the cfg file (perhaps even the
 * zipf normalization constant).
 */
class VMDWorkloadGenerator : public cSimpleModule, public IVMDAccess, public IFileSystem
{
public:
	/** @name Constructor and Desctructor */
	//@{

	VMDWorkloadGenerator();
	virtual ~VMDWorkloadGenerator();

	//@}



	/** @name Video Title functions */
	//@{

	virtual struct VideoTitleMetaData getVTMD(vidt_id_t id);

	virtual struct VideoTitleMetaData getVTMD(str_cref_t video_title);

	/**
	 * Returns a video title id according to the Zipf popularity distribution
	 */
	virtual vidt_id_t selectNextVideoTitle();

	virtual str_t getVTMDFilePath(vidt_id_t id);
	virtual str_t getVTMDFilePath(vtmd_ptr_t vtmd);

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

    virtual void configureFromInput(str_cref_t input_file);

    /// need to write a file that contains number of title info so as to support
    /// reading the configuration back in, that is to allocate heap space for the
    /// _record_array
    /// To Improve: change the ASSERT to cRuntimeErrors which will make more sense
    /// to the user (only if this is going to be distributed)
    virtual void generateConfiguration();

    virtual vtmd_ptr_t generateVTMD();

    virtual void writeConfiguration();
    virtual void writeAllVTMDFiles();
    virtual void writeVTMDFile(vidt_id_t id);
    virtual void writeVTMDFile(vtmd_ptr_t vtmd);

    //@}

	/** @name Overridden from IFileSystem */
	//@{
	virtual bool hasResource(string resource);
	virtual int getResourceSize(httptRequestMessage *, string resource);
    //@}
private:

    /** Stores the VTMD structs. */
    ID_VTMD_Map _video_title_map;

    /** Determines the popularity of a given video title. */
    PopularizedResourceCollection _title_collection;

    /** Directory to write meta data out to; currently only used for testing. */
    str_t _metadata_directory;

    int titles_to_generate;
   	int bit_rates_to_generate;
   	int quality_intervals_to_generate;
   	double uniform_segment_duration;
   	int segments_in_smallest_title;
   	int segments_in_largest_title;

    // for testing
    int _num_requests_to_make;
    int _num_requests_made;
    str_t _test_output_file;
};

/**
 * VTMD resource wrapper to store VTMD structs in the
 * PopularizedResourceCollection.
 */
class PopularizableVideoTitleID : public PopularizableResource
{
public:
	PopularizableVideoTitleID() { _id = VIDTITLE_NULL; }
	PopularizableVideoTitleID(vidt_id_t id) { setVideoTitleID(id); }
	virtual ~PopularizableVideoTitleID() {}

	virtual void setVideoTitleID(vidt_id_t id) { _id = id; }
	vidt_id_t videoTitleID() const { return _id; }

	virtual str_t toString() const { return VideoTitleMetaData::asTitle(_id); }

private:
	vidt_id_t _id;
};

#endif /* _VMDWORKLOADGENERATOR_H_ */
