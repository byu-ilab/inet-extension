// Author: Kevin Black
//
// The Zipf distribution functions are based off of code
// provided in BYU CS 360's Winter 2010 semester.

#include "VideoTitleWorkloadGenerator.h"

#include <cmath>	// for pow and ceil
#include <fstream>	// for writing out to files
#include <vector>	// for parsing results
#include <sstream>	// for crafting URIs
#include "httptUtils.h"

Define_Module(VideoTitleWorkloadGenerator);

VideoTitleWorkloadGenerator::VideoTitleWorkloadGenerator () :
	_metadata_directory(), _video_title_map()
{
	_popularity_array = NULL;
	_popularity_array_size = 0;
	_num_requests_to_make = 0;
	_num_requests_made = 0;
}

VideoTitleWorkloadGenerator::~VideoTitleWorkloadGenerator ()
{
	if (_popularity_array)
	{
		delete[] _popularity_array;
		_popularity_array = NULL;
	}
}

void VideoTitleWorkloadGenerator::initialize()
{
	std::string input_file = par("inputFile").stdstringValue();

	if (!input_file.empty())
	{
		throw cRuntimeError(this, "Configuration from input directory not implemented.");
		//configureFromInput(input_file);
	}
	else
	{
		generateConfiguration();
	}
	_num_requests_to_make = par("requests");
	if (_num_requests_to_make <= 0)
	{
		return;
	}
	cMessage * msg = new cMessage("video title request");
	scheduleAt(simTime()+1.0, msg);
}

void VideoTitleWorkloadGenerator::handleMessage(cMessage *msg)
{
	// for testing
	if (msg->isSelfMessage())
	{
		getNextVideoTitle();
		_num_requests_made++;
		if (_num_requests_made < _num_requests_to_make)
		{
			scheduleAt(simTime()+1.0, msg);
		}
		else
		{
			delete msg;
			EV << "number of requests: "<<_num_requests_made<<endl;
			EV << "rank title popularity requests"<<endl;
			VideoTitlePopularity vtp_entry;
			for (int r = 1; r < _popularity_array_size; r++)
			{
				vtp_entry = _popularity_array[r];
				std::string video_title = getVideoTitleAsString(vtp_entry.video_title_id);
				EV << r << "\t" << video_title << "\t";
				if (video_title.size() < 8)
				{
					EV << "\t";
				}
				EV << vtp_entry.popularity << "\t" << vtp_entry.times_requested << endl;
			}
		}
	}
	else
	{
		delete msg;
	}


	// In the future could handle requests to modify popularity
	// and/or to log information
}


void VideoTitleWorkloadGenerator::finish()
{
	EV << "Total titles: "<<_popularity_array_size<<endl;
	EV << "rank title popularity requests"<<endl;
	VideoTitlePopularity vtp_entry;
	for (int i = 1; i < _popularity_array_size; i++)
	{
		vtp_entry = _popularity_array[i];
		if (vtp_entry.times_requested > 0)
		{
			std::string video_title = getVideoTitleAsString(vtp_entry.video_title_id);
			EV << i << "\t" << video_title << "\t";
			if (video_title.size() < 8)
			{
				EV << "\t";
			}
			EV << vtp_entry.popularity << "\t" << vtp_entry.times_requested << endl;
		}
	}
}


int VideoTitleWorkloadGenerator::getNextVideoTitle()
{
	// get a probability [0,1]
	double desired_popularity = uniform(0,1);

	int current_rank = 0;
	double current_popularity = 0;

	do {
		current_rank++;
		current_popularity += _popularity_array[current_rank].popularity;
	} while (current_popularity < desired_popularity);

	_popularity_array[current_rank].times_requested++;

	return _popularity_array[current_rank].video_title_id;
}

struct VideoTitleMetaData VideoTitleWorkloadGenerator::getMetaData(int video_title_id)
{
	std::map<int, VideoTitleMetaData>::iterator vt_itr = _video_title_map.find(video_title_id);

	if (vt_itr == _video_title_map.end())
	{
		struct VideoTitleMetaData nonexist;
		nonexist.num_segments = -1;
		return nonexist;
	}

	return vt_itr->second;
}

struct VideoTitleMetaData VideoTitleWorkloadGenerator::getMetaData(const std::string & video_title)
{
	int video_title_id = getVideoTitleAsInt(video_title);

	return getMetaData(video_title_id);
}

std::string VideoTitleWorkloadGenerator::getMetaDataFilePath(int video_title_id)
{
	std::map<int, VideoTitleMetaData>::iterator vt_itr = _video_title_map.find(video_title_id);

	std::string file_path = "";
	file_path = _metadata_directory + vt_itr->second.video_title + ".vtmd";

	return file_path;
}

std::string VideoTitleWorkloadGenerator::getVideoTitleAsString(int video_title_id)
{
	std::string hexrep = "";
	int remainder = 0;
	int temp = video_title_id;

	while (temp != 0)
	{
		remainder = temp % 16;
		temp = (temp - remainder) / 16;

		switch(remainder)
		{
		case 0: hexrep = hexrep + "0"; break;
		case 1: hexrep = hexrep + "1"; break;
		case 2: hexrep = hexrep + "2"; break;
		case 3: hexrep = hexrep + "3"; break;
		case 4: hexrep = hexrep + "4"; break;
		case 5: hexrep = hexrep + "5"; break;
		case 6: hexrep = hexrep + "6"; break;
		case 7: hexrep = hexrep + "7"; break;
		case 8: hexrep = hexrep + "8"; break;
		case 9: hexrep = hexrep + "9"; break;
		case 10: hexrep = hexrep + "A"; break;
		case 11: hexrep = hexrep + "B"; break;
		case 12: hexrep = hexrep + "C"; break;
		case 13: hexrep = hexrep + "D"; break;
		case 14: hexrep = hexrep + "E"; break;
		case 15: hexrep = hexrep + "F"; break;
		}
	}
	// invert the string and return it
	return getReverseString(hexrep);
}

int VideoTitleWorkloadGenerator::getVideoTitleAsInt(const std::string & video_title)
{
	// check that the indicated string can be converted to a positive integer
	if (video_title.size() > 8) {
		return -1;
	} else if (video_title.size() == 8){
		char first = video_title.at(0);
		if (first >= '8' ) {
			return -1;
		}
	}

	int video_title_id = 0;
	int digit_value = 0;
	std::string reversed = getReverseString(video_title);
	for(unsigned int i = 0; i < reversed.size(); i++)
	{
		switch(reversed.at(i))
		{
		case '0': digit_value = 0; break;
		case '1': digit_value = 1; break;
		case '2': digit_value = 2; break;
		case '3': digit_value = 3; break;
		case '4': digit_value = 4; break;
		case '5': digit_value = 5; break;
		case '6': digit_value = 6; break;
		case '7': digit_value = 7; break;
		case '8': digit_value = 8; break;
		case '9': digit_value = 9; break;
		case 'A':
		case 'a': digit_value = 10; break;
		case 'B':
		case 'b': digit_value = 11; break;
		case 'C':
		case 'c': digit_value = 12; break;
		case 'D':
		case 'd': digit_value = 13; break;
		case 'E':
		case 'e': digit_value = 14; break;
		case 'F':
		case 'f': digit_value = 15; break;
		default:
			return -1;
		}

		for (unsigned int p = 0; p < i; p++)
		{
			digit_value = digit_value * 16;
		}
		video_title_id += digit_value;
	}
	return video_title_id;
}

//std::string VideoTitleWorkloadGenerator::getReverseString(const std::string & toreverse)
//{
//	std::string retstr = "";
//	for (int i = toreverse.size()-1; i >= 0; i--)
//	{
//		retstr += toreverse.at(i);
//	}
//	return retstr;
//}


void VideoTitleWorkloadGenerator::configureFromInput(const std::string & input_file)
{
	// currently not supported
}

void VideoTitleWorkloadGenerator::generateConfiguration()
{
	// read in NED settings
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
	int num_titles = par("numTitles");
		ASSERT(num_titles >= 1 && num_titles < 0x7FFFFFFF);
	int num_bit_rates = par("numBitRates");
		ASSERT(num_bit_rates >= 1);
	int quality_interval = par("qualityInterval");
		ASSERT(quality_interval >= 1);
	double segment_duration = par("segmentDuration");
		ASSERT(segment_duration > 0);

	double zipf_exponent = par("zipfExponent");
	double zipf_rank_offset = par("zipfRankOffset");
		ASSERT(zipf_rank_offset >= 0);

	// variables to read in volatile settings
	double title_duration = 0.0;

	// allocate popularity array
		// +1 to accommodate for empty 0 index
//	_popularity_array_size = num_vod_titles + num_live_titles + 1;
	_popularity_array_size = num_titles + 1;
	_popularity_array = new VideoTitlePopularity[_popularity_array_size];

	// compute Zipf normalization constant
	double zipf_normalization_constant = 0.0;
	for (int r = 1; r < _popularity_array_size; r++)
	{
		zipf_normalization_constant += ( 1.0 / std::pow( (r+zipf_rank_offset), zipf_exponent) );
	}
	zipf_normalization_constant = 1.0 / zipf_normalization_constant;

	// generate video title meta data
	VideoTitleMetaData vtmd_template;
	vtmd_template.num_quality_levels = num_bit_rates;
	vtmd_template.quality_interval = quality_interval;
	vtmd_template.video_type = "vod";

	int video_title_id = -1;
	VideoTitlePopularity popularity_entry;
	popularity_entry.times_requested = 0;
	std::string file_path = "";
	std::ofstream output_file;
	int min_segments = 0x7FFFFFFF;
	int max_segments = 0;

	for (int current_rank = 1; current_rank < _popularity_array_size; current_rank++)
	{
		// generate a title
		do
		{
			//from 5 to 8 hex digits, positive values
			video_title_id = intuniform(0x70000, 0x7FFFFFFF);
		} while ( _video_title_map.find(video_title_id) != _video_title_map.end() );
		vtmd_template.video_title = getVideoTitleAsString(video_title_id);

		// determine number of segments
		title_duration = par("titleDuration"); // should be in minutes
			ASSERT(title_duration > 0.0);
		vtmd_template.num_segments = std::ceil( (title_duration*60.0) / segment_duration );

		if (vtmd_template.num_segments < min_segments)
		{
			min_segments = vtmd_template.num_segments;
		}
		if (vtmd_template.num_segments > max_segments)
		{
			max_segments = vtmd_template.num_segments;
		}

		// store the meta data
		_video_title_map[video_title_id] = vtmd_template;

		// calculate the popularity and store it
		popularity_entry.video_title_id = video_title_id;
		popularity_entry.popularity = (zipf_normalization_constant /
				( std::pow(current_rank + zipf_rank_offset, zipf_exponent) ) );

		_popularity_array[current_rank] = popularity_entry;

		// write out .vtmd file
		// writes out all files in the same directory
		if (!_metadata_directory.empty())
		{
			file_path = getMetaDataFilePath(video_title_id);
			output_file.open(file_path.c_str());
			if (output_file.bad() || output_file.fail())
			{
				throw cRuntimeError(this, "Error in writing out file %s", file_path.c_str());
			}
			output_file << "title: "<<vtmd_template.video_title<<endl;
			output_file << "type: "<<vtmd_template.video_type<<endl;
			output_file << "segments: "<<vtmd_template.num_segments<<endl;
			output_file << "rates: "<<vtmd_template.num_quality_levels<<endl;
			output_file << "interval: "<<vtmd_template.quality_interval<<endl;
			output_file.flush();
			output_file.close();
		}
	}

	// write out configuration file
	if (!_metadata_directory.empty())
	{
		file_path = _metadata_directory+"vtwg.cfg";
		output_file.open(file_path.c_str());
		if (output_file.bad() || output_file.fail())
		{
			throw cRuntimeError(this, "Error in writing out configuration file %s", "vtwg.cfg");
		}
		output_file << "titles: "<<num_titles<<endl;
		// or vod and live
		output_file << "minsegments: "<<min_segments<<endl;
		output_file << "maxsegments: "<<max_segments<<endl;
		// if going to read in data then will have to write out the file titles
	}
}

#define NUM_URI_FIELDS_DEFAULT 4
#define NUM_URI_FIELDS_WITH_BP 6
#define TYPE_INDEX 0
#define TITLE_INDEX 1
#define QUALITY_LEVEL_INDEX 2
#define SEGMENT_NUM_INDEX 3
#define FIRST_BYTE_POS_INDEX 4
#define LAST_BYTE_POS_INDEX 5

/*
 * Returns the video segment data as contained in the uri.
 * Throws an error if the uri isn't in the right format.
 */
struct VideoSegmentMetaData VideoTitleWorkloadGenerator::parseVideoSegmentUri(const std::string & uri)
{
	cStringTokenizer tokenizer = cStringTokenizer(uri.c_str(),"/#,");
	vector<string> res = tokenizer.asVector();
	if (res.size() != NUM_URI_FIELDS_DEFAULT && res.size() != NUM_URI_FIELDS_WITH_BP)
	{
		error("invalid uri provided");
	}

	// TODO check that res[QAULITY_LEVEL_INDEX] and res[SEGMENT_NUM_INDEX] are numbers?
	VideoSegmentMetaData vsmd;
	vsmd.video_type = res[TYPE_INDEX];
	vsmd.video_title = res[TITLE_INDEX];
	vsmd.quality_level = atoi(res[QUALITY_LEVEL_INDEX].c_str());
	vsmd.segment_number = atoi(res[SEGMENT_NUM_INDEX].c_str());

	if (res.size() == NUM_URI_FIELDS_WITH_BP)
	{
		vsmd.first_byte_position = atoi(res[FIRST_BYTE_POS_INDEX].c_str());
		vsmd.last_byte_position = atoi(res[LAST_BYTE_POS_INDEX].c_str());
	}
	else
	{
		vsmd.first_byte_position = -1;
		vsmd.last_byte_position = -1;
	}

	return vsmd;
}

/*
 * Simple creates a video segment uri according to the provided parameters.
 * Does not check if the parameters are valid.
 */
std::string VideoTitleWorkloadGenerator::createVideoSegmentUri(const std::string & type,
		const std::string & title, int quality_level, int segment_number, int fbp, int lbp)
{
	if (0 <= lbp)
	{
		ASSERT( 0 <= fbp );
	}

	stringstream uri_template;
	int i = 0;
	while (i < NUM_URI_FIELDS_WITH_BP)
	{
		//uri_template << "/";
		switch(i)
		{
		case TYPE_INDEX: uri_template <<"/"<< type; break;
		case TITLE_INDEX: uri_template <<"/"<< title; break;
		case QUALITY_LEVEL_INDEX: uri_template <<"/"<< quality_level; break;
		case SEGMENT_NUM_INDEX: uri_template <<"/"<< segment_number<<".vid"; break;
		case FIRST_BYTE_POS_INDEX:
			if (0 <= fbp)
			{
				uri_template << "#" << fbp;
			}
			break;
		case LAST_BYTE_POS_INDEX:
			if (0 <= fbp)
			{
				if (0 <= lbp)
				{
					uri_template << ","<<lbp;
				}
				else
				{
					uri_template << ","<<-1;
				}
			}
			break;
		default:
			throw cRuntimeError("Check the #define constants used to generate URIs.");
		}
		i++;
	}
	//uri_template << ".vid";

	return uri_template.str();

//	Other Format possibility; requires #include <iomanip>
//	setfill('0');
//	std::stringstream uri;
//	uri << "/" << video_type << "/" << video_title << "/" << setw(2) << quality_level << data->segment << ".vid";
}

/*
 * Determines whether the provided video segment data is valid according to
 * it's corresponding video title's meta data.
 */
bool VideoTitleWorkloadGenerator::isVideoSegmentDataValid(const struct VideoSegmentMetaData & vsdata)
{
	VideoTitleMetaData vtmd = getMetaData(vsdata.video_title);

	if (vtmd.num_segments < MIN_SEGMENT_NUMBER) //implicitly checks that the titles are the same
	{
		return false;
	}

	if (vtmd.video_type != vsdata.video_type)
	{
		return false;
	}

	if (vsdata.quality_level < MIN_QUALITY_LEVEL || vtmd.num_quality_levels < vsdata.quality_level )
	{
		return false;
	}

	if (vsdata.segment_number < MIN_SEGMENT_NUMBER || (vtmd.num_segments - 1) < vsdata.segment_number )
	{
		return false;
	}

	//TODO check the byte ranges if they are kept long term

	return true;
}

bool VideoTitleWorkloadGenerator::isVideoSegmentDataValid(const std::string & uri)
{
	return isVideoSegmentDataValid(parseVideoSegmentUri(uri));
}
