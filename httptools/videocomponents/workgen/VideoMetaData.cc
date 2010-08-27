//===========================================================================80>
/**
 * @file VideoMetaData.cc
 *
 * VideoTitleMetaData and VideoSegmentMetaData struct definitions.
 *
 * Created: Aug 26, 2010
 *
 * @todo Add GPL notice.
 */

// from inet
#include "VideoMetaData.h"
#include "httptUtils.h" 	// for string reversal

// from standard C++ libraries
#include <sstream>
#include <vector>

// from omnetpp extension
#include <omnetppextension.h>

void VideoTitleMetaData::initialize(str_cref_t title, str_cref_t type, int segments,
		double seg_duration, int quality_levels, int interval)
{
	video_title = title;
	video_type = type;
	num_segments = segments;
	segment_duration = seg_duration;
	num_quality_levels = quality_levels;
	quality_interval = interval;
}

VideoTitleMetaData::VideoTitleMetaData()
{
	initialize("", "", 0, 0, 0, 0);
}

VideoTitleMetaData::VideoTitleMetaData(uri_t uri)
{

}

VideoTitleMetaData::VideoTitleMetaData(str_cref_t title, str_cref_t type, int segments,
		double seg_duration, int quality_levels, int interval)
{
	initialize(title, type, segments, seg_duration, quality_levels, interval);
}

VideoTitleMetaData::~VideoTitleMetaData()
{
	// nothing
}

void VideoTitleMetaData::setFromURI(uri_cref_t uri)
{

}

uri_t VideoTitleMetaData::getURI() const
{
	uri_t uri = "/";
	uri += video_type + "/" + video_title + "/info.vtmd";
	return uri;
}

bool VideoTitleMetaData::isValid() const
{
	return isValid(*this);
}

bool VideoTitleMetaData::isValid(const struct VideoTitleMetaData & vtmd)
{
	return true;
}

bool VideoTitleMetaData::isValid(uri_cref_t uri)
{
	return true;
}

vidt_id_t VideoTitleMetaData::randomID()
{
	//from 5 to 8 hex digits, positive values
	return intuniform(0x70000, 0x7FFFFFFF);
}

str_t VideoTitleMetaData::asTitle(vidt_id_t id)
{
	str_t hexrep = "";
	vidt_id_t remainder = 0;
	vidt_id_t temp = id;

	while (temp != 0)
	{
		remainder = temp % 16;
		temp = (temp - remainder) / 16;

		switch(remainder)
		{
		case 0:  hexrep = hexrep + "0"; break;
		case 1:  hexrep = hexrep + "1"; break;
		case 2:  hexrep = hexrep + "2"; break;
		case 3:  hexrep = hexrep + "3"; break;
		case 4:  hexrep = hexrep + "4"; break;
		case 5:  hexrep = hexrep + "5"; break;
		case 6:  hexrep = hexrep + "6"; break;
		case 7:  hexrep = hexrep + "7"; break;
		case 8:  hexrep = hexrep + "8"; break;
		case 9:  hexrep = hexrep + "9"; break;
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

vidt_id_t VideoTitleMetaData::asID(str_cref_t title)
{
	// check that the indicated string can be converted to a positive integer
	if (title.empty() || 8 < title.size())
	{
		return VIDTITLE_NULL;
	}
	else if (title.size() == 8)
	{
		char first = title.at(0);
		if ('8' <= first) // ascii comparison
		{
			return VIDTITLE_NULL;
		}
	}

	// make the conversion
	vidt_id_t id = 0;
	int digit_value = 0;
	str_t reversed = getReverseString(title);

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
			return VIDTITLE_NULL;
		}

		for (unsigned int p = 0; p < i; p++)
		{
			digit_value = digit_value * 16;
		}
		id += digit_value;
	}

	return id;
}



//---


void VideoSegmentMetaData::initialize(str_cref_t title, str_cref_t type,
		int level, int number, int fbp, int lbp)
{
	video_title = title;
	video_type = type;
	quality_level = level;
	segment_number = number;
	first_byte_position = fbp;
	last_byte_position = lbp;
}


VideoSegmentMetaData::VideoSegmentMetaData()
{
	initialize("","",-1,-1);
}

VideoSegmentMetaData::VideoSegmentMetaData(uri_t uri)
{
	initialize("","",-1,-1);
	setFromURI(uri);
}

VideoSegmentMetaData::VideoSegmentMetaData(str_cref_t title, str_cref_t type, int level,
		int number, int fbp, int lbp)
{
	initialize(title, type, level, number, fbp, lbp);
}

VideoSegmentMetaData::~VideoSegmentMetaData()
{
	// nothing
}

#define NUM_URI_FIELDS_DEFAULT 4
#define NUM_URI_FIELDS_WITH_BP 6
#define TYPE_INDEX 0
#define TITLE_INDEX 1
#define QUALITY_LEVEL_INDEX 2
#define SEGMENT_NUM_INDEX 3
#define FIRST_BYTE_POS_INDEX 4
#define LAST_BYTE_POS_INDEX 5

void VideoSegmentMetaData::setFromURI(uri_cref_t uri)
{
	cStringTokenizer tokenizer = cStringTokenizer(uri.c_str(),"/#,");
	std::vector<str_t> tokens = tokenizer.asVector();

	if (tokens.size() != NUM_URI_FIELDS_DEFAULT && tokens.size() != NUM_URI_FIELDS_WITH_BP)
	{
		return;
	}
	// TODO check that tokens[QAULITY_LEVEL_INDEX] and tokens[SEGMENT_NUM_INDEX] are numbers

	// else

	video_type = tokens[TYPE_INDEX];
	video_title = tokens[TITLE_INDEX];
	quality_level = atoi(tokens[QUALITY_LEVEL_INDEX].c_str());
	segment_number = atoi(tokens[SEGMENT_NUM_INDEX].c_str());

	if (tokens.size() == NUM_URI_FIELDS_WITH_BP)
	{
		first_byte_position = atoi(tokens[FIRST_BYTE_POS_INDEX].c_str());
		last_byte_position = atoi(tokens[LAST_BYTE_POS_INDEX].c_str());
	}
	else
	{
		first_byte_position = BRS_UNSPECIFIED;
		last_byte_position = BRS_UNSPECIFIED;
	}
}

uri_t VideoSegmentMetaData::getURI() const
{
	return createURI(video_title, video_type, quality_level,
			segment_number, first_byte_position, last_byte_position);
}

bool VideoSegmentMetaData::pertainsTo(const struct VideoTitleMetaData & vtmd) const
{
	if (vtmd.video_title.compare(this->video_title) != 0)
	{
		return false;
	}

	// else
	if (vtmd.video_type != this->video_type)
	{
		return false;
	}

	// else
	if (this->quality_level < MIN_QUALITY_LEVEL || vtmd.num_quality_levels < this->quality_level )
	{
		return false;
	}

	// else
	if (vtmd.num_segments < MIN_SEGMENT_NUMBER || this->segment_number < MIN_SEGMENT_NUMBER ||
			(vtmd.num_segments - 1) < this->segment_number)
	{
		return false;
	}

	// TODO check the byte ranges

	return true;
}

uri_t VideoSegmentMetaData::createURI(str_cref_t title, str_cref_t type, int level,
		int number, int fbp, int lbp)
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
		case QUALITY_LEVEL_INDEX: uri_template <<"/"<< level; break;
		case SEGMENT_NUM_INDEX: uri_template <<"/"<< number<<".vid"; break;
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

	return uri_template.str();

	//	Other Format possibility; requires #include <iomanip>
	//	setfill('0');
	//	std::stringstream uri;
	//	uri << "/" << video_type << "/" << video_title << "/" << setw(2) << quality_level << data->segment << ".vid";
}


