//===========================================================================80>
/**
 * @file VMDWorkloadGenerator.h
 *
 * VMDWorkloadGenerator class definitions.
 *
 * Created: Aug 26, 2010
 *
 * Based on previous VideoTitleWorkloadGenerator dating from May 2010.
 *
 * @todo Add GPL notice.
 */

// from inet
#include "VMDWorkloadGenerator.h"

// from omnetpp extension
#include <omnetppextension.h>

// from standard C++ libraries
#include <fstream>	// for writing out to files
#include <cmath>	// for ceil

//---

Define_Module(VMDWorkloadGenerator);

VMDWorkloadGenerator::VMDWorkloadGenerator () :
		_video_title_map(), _title_collection(), _metadata_directory(), _test_output_file()
{
	_num_requests_to_make = 0;
	_num_requests_made = 0;
}

VMDWorkloadGenerator::~VMDWorkloadGenerator ()
{
	ID_VTMD_Map::iterator vt_itr = _video_title_map.begin();
	for ( ; vt_itr != _video_title_map.end(); vt_itr++)
	{
		deleteSafe(vt_itr->second);
	}
}

void VMDWorkloadGenerator::initialize()
{
	str_t input_file = par("inputFile").stdstringValue();

	if (!input_file.empty())
	{
		throw cRuntimeError(this, "Configuration from input directory not implemented.");
		//configureFromInput(input_file);
	}
	else
	{
		// Get and verify NED parameters
		_metadata_directory = par("outputDir").stdstringValue();
		if (!_metadata_directory.empty() && _metadata_directory.at((int)_metadata_directory.size()-1) != '/')
		{
			_metadata_directory += "/";
		}
		//	int num_vod_titles = par("numVODTitles");
		//		ASSERT(num_vod_titles >= 0);
		//	int num_live_titles = par("numLiveTitles");
		//		ASSERT(num_live_titles >= 0);
		//		ASSERT(num_vod_titles + num_live_titles >= 1 &&
		//				num_vod_titles + num_live_titles < std::Int32::MaxValue);
		titles_to_generate = par("numTitles");
			ASSERT(1 <= titles_to_generate && titles_to_generate < 0x7FFFFFFF);
		bit_rates_to_generate = par("numBitRates");
			ASSERT(1 <= bit_rates_to_generate);
		quality_intervals_to_generate = par("qualityInterval");
			ASSERT(1 <= quality_intervals_to_generate);
		uniform_segment_duration = par("segmentDuration");
			ASSERT(0 < uniform_segment_duration);

		_title_collection.setRankOffset(par("zipfRankOffset").doubleValue());
		_title_collection.setExponent(par("zipfExponent").doubleValue());

		segments_in_smallest_title = 0x7FFFFFFF;
		segments_in_largest_title = 0;

		generateConfiguration();
	}

	// run tests
	_test_output_file = par("testOutputFile").stdstringValue();
	_num_requests_to_make = par("requests");
	if (0 < _num_requests_to_make)
	{
		_num_requests_made = 0;
		cMessage * msg = new cMessage("video title request");
		scheduleAt(simTime()+1.0, msg);
	}
	// else don't run tests
}

void VMDWorkloadGenerator::handleMessage(cMessage *msg)
{
	// for testing
	if (msg->isSelfMessage())
	{
		selectNextVideoTitle();
		_num_requests_made++;
		if (_num_requests_made < _num_requests_to_make)
		{
			scheduleAt(simTime()+1.0, msg);
		}
		else
		{
			delete msg;
			// now finish should be called
		}
	}
	else
	{
		delete msg;
	}
}


void VMDWorkloadGenerator::finish()
{
	if (_test_output_file.empty())
	{
		_title_collection.writeSummary(ev.getOStream());
	}
	else
	{
		str_t filename = _metadata_directory + _test_output_file;
		std::ofstream output_file(filename.c_str());

		if (output_file.bad() || output_file.fail())
		{
			EV << "Can't write summary to " << _test_output_file
					<< ".  Writing to console..." << endl;
			_title_collection.writeSummary(ev.getOStream());
		}
		else
		{
			_title_collection.writeSummary(output_file);
		}
	}
}


vidt_id_t VMDWorkloadGenerator::selectNextVideoTitle()
{
	PopularizableVideoTitleID * resource = dynamic_cast<PopularizableVideoTitleID *>(_title_collection.selectResource());
	if (resource == NULL)
	{
		return VIDTITLE_NULL;
	}
	// else
	return resource->videoTitleID();
}

struct VideoTitleMetaData VMDWorkloadGenerator::getVTMD(vidt_id_t id)
{
	ID_VTMD_Map::iterator vt_itr = _video_title_map.find(id);

	if (vt_itr == _video_title_map.end())
	{
		struct VideoTitleMetaData nonexist;
		nonexist.num_segments = -1;
		return nonexist;
	}

	return *(vt_itr->second);
}

struct VideoTitleMetaData VMDWorkloadGenerator::getVTMD(str_cref_t title)
{
	return getVTMD(VideoTitleMetaData::asID(title));
}

str_t VMDWorkloadGenerator::getVTMDFilePath(vidt_id_t id)
{
	ID_VTMD_Map::iterator vt_itr = _video_title_map.find(id);

	if (vt_itr == _video_title_map.end())
	{
		return "";
	}
	// else
	return getVTMDFilePath(vt_itr->second);
}

str_t VMDWorkloadGenerator::getVTMDFilePath(vtmd_ptr_t vtmd)
{
	return (_metadata_directory + vtmd->video_title + ".vtmd");
}

void VMDWorkloadGenerator::configureFromInput(str_cref_t input_file)
{
	throw cRuntimeError(this, "not yet implemented");
}

void VMDWorkloadGenerator::generateConfiguration()
{
	for (int title_counter = 0; title_counter < titles_to_generate; title_counter++)
	{
		// generate the video title meta data
		vtmd_ptr_t vtmd = generateVTMD();

		// store the video title meta data
		vidt_id_t id = VideoTitleMetaData::asID(vtmd->video_title);
		_video_title_map[id] = vtmd;

		PopularizableVideoTitleID * resource_entry = new PopularizableVideoTitleID(id);
		_title_collection.addResource(resource_entry);
	}

	writeConfiguration();
	writeAllVTMDFiles();
}

vtmd_ptr_t VMDWorkloadGenerator::generateVTMD()
{
	// Generate video title meta data
	vtmd_ptr_t vtmd  = new VideoTitleMetaData();
	vtmd->num_quality_levels = bit_rates_to_generate;
	vtmd->quality_interval = quality_intervals_to_generate;
	vtmd->video_type = "vod";

	vidt_id_t id = VIDTITLE_NULL;

	// generate a title
	do
	{
		id = VideoTitleMetaData::randomID();
	} while ( _video_title_map.find(id) != _video_title_map.end() );

	vtmd->video_title = VideoTitleMetaData::asTitle(id);

	// determine number of segments
	double title_duration = par("titleDuration"); // should be in minutes
		ASSERT(0.0 < title_duration);
		ASSERT(0.0 < uniform_segment_duration);
	vtmd->num_segments = std::ceil( (title_duration*60.0) / uniform_segment_duration );

	// update min and max num segments so far
	if (vtmd->num_segments < segments_in_smallest_title)
	{
		segments_in_smallest_title = vtmd->num_segments;
	}

	if (segments_in_largest_title < vtmd->num_segments)
	{
		segments_in_largest_title = vtmd->num_segments;
	}

	return vtmd;
}

void VMDWorkloadGenerator::writeConfiguration()
{
	if (_metadata_directory.empty())
	{
		return;
	}
	// else

	// open output file
	str_t file_path = _metadata_directory+"vmdwkldgen.cfg";

	std::ofstream output_file(file_path.c_str());

	if (output_file.bad() || output_file.fail())
	{
		throw cRuntimeError(this, "Error in writing out configuration file %s", "vtwg.cfg");
	}
	// else
	// write out configuration file
	ASSERT(titles_to_generate == (int) _video_title_map.size());
	output_file << "titles: " << titles_to_generate << endl;

		// or vod and live
	output_file << "minsegments: "<< segments_in_smallest_title << endl;
	output_file << "maxsegments: "<< segments_in_largest_title << endl;

	// if going to read in data then will have to write out the file titles
}

void VMDWorkloadGenerator::writeAllVTMDFiles()
{
	if (_metadata_directory.empty())
	{
		return;
	}
	// else

	ID_VTMD_Map::iterator vt_itr = _video_title_map.begin();
	for ( ; vt_itr != _video_title_map.end(); vt_itr++)
	{
		writeVTMDFile(vt_itr->second);
	}
}

void VMDWorkloadGenerator::writeVTMDFile(vidt_id_t id)
{
	if (_metadata_directory.empty())
	{
		return;
	}
	// else

	ID_VTMD_Map::iterator vt_itr = _video_title_map.find(id);
	if (vt_itr == _video_title_map.end())
	{
		return;
	}
	// else

	writeVTMDFile(vt_itr->second);
}

void VMDWorkloadGenerator::writeVTMDFile(vtmd_ptr_t vtmd)
{
	ASSERT(vtmd != NULL);

	// open output file
	str_t file_path = getVTMDFilePath(vtmd);

	std::ofstream output_file(file_path.c_str());

	if (output_file.bad() || output_file.fail())
	{
		throw cRuntimeError(this, "Error in writing out file %s", file_path.c_str());
	}
	// else

	// write out file
	output_file << "title: " << vtmd->video_title << endl;
	output_file << "type: "  << vtmd->video_type  << endl;
	output_file << "segments: "<< vtmd->num_segments << endl;
	output_file << "rates: " << vtmd->num_quality_levels << endl;
	output_file << "interval: "<<vtmd->quality_interval << endl;

	output_file.flush();
	output_file.close();
}
