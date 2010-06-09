//
// Generated file, do not edit! Created by opp_msgc 4.0 from httptools/videocomponents/messages/VideoSegmentMessages.msg.
//

#ifndef _VIDEOSEGMENTMESSAGES_M_H_
#define _VIDEOSEGMENTMESSAGES_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0400
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif

// cplusplus {{ ... }} section:

#include "httptMessages_m.h"
// end cplusplus



/**
 * Class generated from <tt>httptools/videocomponents/messages/VideoSegmentMessages.msg</tt> by opp_msgc.
 * <pre>
 * message VideoSegmentRequestMessage extends httptRequestMessage
 * {
 *     string title = "";
 *     string type = "";
 *     int qualityLevel = 0;
 *     int segmentNumber = -1;
 * }
 * </pre>
 */
class VideoSegmentRequestMessage : public httptRequestMessage
{
  protected:
    opp_string title_var;
    opp_string type_var;
    int qualityLevel_var;
    int segmentNumber_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const VideoSegmentRequestMessage&);

  public:
    VideoSegmentRequestMessage(const char *name=NULL, int kind=0);
    VideoSegmentRequestMessage(const VideoSegmentRequestMessage& other);
    virtual ~VideoSegmentRequestMessage();
    VideoSegmentRequestMessage& operator=(const VideoSegmentRequestMessage& other);
    virtual VideoSegmentRequestMessage *dup() const {return new VideoSegmentRequestMessage(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual const char * getTitle() const;
    virtual void setTitle(const char * title_var);
    virtual const char * getType() const;
    virtual void setType(const char * type_var);
    virtual int getQualityLevel() const;
    virtual void setQualityLevel(int qualityLevel_var);
    virtual int getSegmentNumber() const;
    virtual void setSegmentNumber(int segmentNumber_var);
};

inline void doPacking(cCommBuffer *b, VideoSegmentRequestMessage& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, VideoSegmentRequestMessage& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>httptools/videocomponents/messages/VideoSegmentMessages.msg</tt> by opp_msgc.
 * <pre>
 * message VideoSegmentReplyMessage extends httptReplyMessage 
 * {
 *     string title = "";
 *     string type  = "";
 *     int qualityLevel = 0;
 *     int segmentNumber = -1;
 * }
 * </pre>
 */
class VideoSegmentReplyMessage : public httptReplyMessage
{
  protected:
    opp_string title_var;
    opp_string type_var;
    int qualityLevel_var;
    int segmentNumber_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const VideoSegmentReplyMessage&);

  public:
    VideoSegmentReplyMessage(const char *name=NULL, int kind=0);
    VideoSegmentReplyMessage(const VideoSegmentReplyMessage& other);
    virtual ~VideoSegmentReplyMessage();
    VideoSegmentReplyMessage& operator=(const VideoSegmentReplyMessage& other);
    virtual VideoSegmentReplyMessage *dup() const {return new VideoSegmentReplyMessage(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual const char * getTitle() const;
    virtual void setTitle(const char * title_var);
    virtual const char * getType() const;
    virtual void setType(const char * type_var);
    virtual int getQualityLevel() const;
    virtual void setQualityLevel(int qualityLevel_var);
    virtual int getSegmentNumber() const;
    virtual void setSegmentNumber(int segmentNumber_var);
};

inline void doPacking(cCommBuffer *b, VideoSegmentReplyMessage& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, VideoSegmentReplyMessage& obj) {obj.parsimUnpack(b);}


#endif // _VIDEOSEGMENTMESSAGES_M_H_
