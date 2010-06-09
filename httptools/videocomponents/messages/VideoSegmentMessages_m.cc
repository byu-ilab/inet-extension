//
// Generated file, do not edit! Created by opp_msgc 4.0 from ./VideoSegmentMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "VideoSegmentMessages_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




Register_Class(VideoSegmentRequestMessage);

VideoSegmentRequestMessage::VideoSegmentRequestMessage(const char *name, int kind) : httptRequestMessage(name,kind)
{
    this->title_var = "";
    this->type_var = "";
    this->qualityLevel_var = 0;
    this->segmentNumber_var = -1;
}

VideoSegmentRequestMessage::VideoSegmentRequestMessage(const VideoSegmentRequestMessage& other) : httptRequestMessage()
{
    setName(other.getName());
    operator=(other);
}

VideoSegmentRequestMessage::~VideoSegmentRequestMessage()
{
}

VideoSegmentRequestMessage& VideoSegmentRequestMessage::operator=(const VideoSegmentRequestMessage& other)
{
    if (this==&other) return *this;
    httptRequestMessage::operator=(other);
    this->title_var = other.title_var;
    this->type_var = other.type_var;
    this->qualityLevel_var = other.qualityLevel_var;
    this->segmentNumber_var = other.segmentNumber_var;
    return *this;
}

void VideoSegmentRequestMessage::parsimPack(cCommBuffer *b)
{
    httptRequestMessage::parsimPack(b);
    doPacking(b,this->title_var);
    doPacking(b,this->type_var);
    doPacking(b,this->qualityLevel_var);
    doPacking(b,this->segmentNumber_var);
}

void VideoSegmentRequestMessage::parsimUnpack(cCommBuffer *b)
{
    httptRequestMessage::parsimUnpack(b);
    doUnpacking(b,this->title_var);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->qualityLevel_var);
    doUnpacking(b,this->segmentNumber_var);
}

const char * VideoSegmentRequestMessage::getTitle() const
{
    return title_var.c_str();
}

void VideoSegmentRequestMessage::setTitle(const char * title_var)
{
    this->title_var = title_var;
}

const char * VideoSegmentRequestMessage::getType() const
{
    return type_var.c_str();
}

void VideoSegmentRequestMessage::setType(const char * type_var)
{
    this->type_var = type_var;
}

int VideoSegmentRequestMessage::getQualityLevel() const
{
    return qualityLevel_var;
}

void VideoSegmentRequestMessage::setQualityLevel(int qualityLevel_var)
{
    this->qualityLevel_var = qualityLevel_var;
}

int VideoSegmentRequestMessage::getSegmentNumber() const
{
    return segmentNumber_var;
}

void VideoSegmentRequestMessage::setSegmentNumber(int segmentNumber_var)
{
    this->segmentNumber_var = segmentNumber_var;
}

class VideoSegmentRequestMessageDescriptor : public cClassDescriptor
{
  public:
    VideoSegmentRequestMessageDescriptor();
    virtual ~VideoSegmentRequestMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual bool getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(VideoSegmentRequestMessageDescriptor);

VideoSegmentRequestMessageDescriptor::VideoSegmentRequestMessageDescriptor() : cClassDescriptor("VideoSegmentRequestMessage", "httptRequestMessage")
{
}

VideoSegmentRequestMessageDescriptor::~VideoSegmentRequestMessageDescriptor()
{
}

bool VideoSegmentRequestMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<VideoSegmentRequestMessage *>(obj)!=NULL;
}

const char *VideoSegmentRequestMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int VideoSegmentRequestMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int VideoSegmentRequestMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return FD_ISEDITABLE;
        case 1: return FD_ISEDITABLE;
        case 2: return FD_ISEDITABLE;
        case 3: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *VideoSegmentRequestMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "title";
        case 1: return "type";
        case 2: return "qualityLevel";
        case 3: return "segmentNumber";
        default: return NULL;
    }
}

const char *VideoSegmentRequestMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "string";
        case 1: return "string";
        case 2: return "int";
        case 3: return "int";
        default: return NULL;
    }
}

const char *VideoSegmentRequestMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int VideoSegmentRequestMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentRequestMessage *pp = (VideoSegmentRequestMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool VideoSegmentRequestMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentRequestMessage *pp = (VideoSegmentRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: oppstring2string(pp->getTitle(),resultbuf,bufsize); return true;
        case 1: oppstring2string(pp->getType(),resultbuf,bufsize); return true;
        case 2: long2string(pp->getQualityLevel(),resultbuf,bufsize); return true;
        case 3: long2string(pp->getSegmentNumber(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool VideoSegmentRequestMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentRequestMessage *pp = (VideoSegmentRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setTitle((value)); return true;
        case 1: pp->setType((value)); return true;
        case 2: pp->setQualityLevel(string2long(value)); return true;
        case 3: pp->setSegmentNumber(string2long(value)); return true;
        default: return false;
    }
}

const char *VideoSegmentRequestMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

void *VideoSegmentRequestMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentRequestMessage *pp = (VideoSegmentRequestMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(VideoSegmentReplyMessage);

VideoSegmentReplyMessage::VideoSegmentReplyMessage(const char *name, int kind) : httptReplyMessage(name,kind)
{
    this->title_var = "";
    this->type_var = "";
    this->qualityLevel_var = 0;
    this->segmentNumber_var = -1;
}

VideoSegmentReplyMessage::VideoSegmentReplyMessage(const VideoSegmentReplyMessage& other) : httptReplyMessage()
{
    setName(other.getName());
    operator=(other);
}

VideoSegmentReplyMessage::~VideoSegmentReplyMessage()
{
}

VideoSegmentReplyMessage& VideoSegmentReplyMessage::operator=(const VideoSegmentReplyMessage& other)
{
    if (this==&other) return *this;
    httptReplyMessage::operator=(other);
    this->title_var = other.title_var;
    this->type_var = other.type_var;
    this->qualityLevel_var = other.qualityLevel_var;
    this->segmentNumber_var = other.segmentNumber_var;
    return *this;
}

void VideoSegmentReplyMessage::parsimPack(cCommBuffer *b)
{
    httptReplyMessage::parsimPack(b);
    doPacking(b,this->title_var);
    doPacking(b,this->type_var);
    doPacking(b,this->qualityLevel_var);
    doPacking(b,this->segmentNumber_var);
}

void VideoSegmentReplyMessage::parsimUnpack(cCommBuffer *b)
{
    httptReplyMessage::parsimUnpack(b);
    doUnpacking(b,this->title_var);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->qualityLevel_var);
    doUnpacking(b,this->segmentNumber_var);
}

const char * VideoSegmentReplyMessage::getTitle() const
{
    return title_var.c_str();
}

void VideoSegmentReplyMessage::setTitle(const char * title_var)
{
    this->title_var = title_var;
}

const char * VideoSegmentReplyMessage::getType() const
{
    return type_var.c_str();
}

void VideoSegmentReplyMessage::setType(const char * type_var)
{
    this->type_var = type_var;
}

int VideoSegmentReplyMessage::getQualityLevel() const
{
    return qualityLevel_var;
}

void VideoSegmentReplyMessage::setQualityLevel(int qualityLevel_var)
{
    this->qualityLevel_var = qualityLevel_var;
}

int VideoSegmentReplyMessage::getSegmentNumber() const
{
    return segmentNumber_var;
}

void VideoSegmentReplyMessage::setSegmentNumber(int segmentNumber_var)
{
    this->segmentNumber_var = segmentNumber_var;
}

class VideoSegmentReplyMessageDescriptor : public cClassDescriptor
{
  public:
    VideoSegmentReplyMessageDescriptor();
    virtual ~VideoSegmentReplyMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual bool getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(VideoSegmentReplyMessageDescriptor);

VideoSegmentReplyMessageDescriptor::VideoSegmentReplyMessageDescriptor() : cClassDescriptor("VideoSegmentReplyMessage", "httptReplyMessage")
{
}

VideoSegmentReplyMessageDescriptor::~VideoSegmentReplyMessageDescriptor()
{
}

bool VideoSegmentReplyMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<VideoSegmentReplyMessage *>(obj)!=NULL;
}

const char *VideoSegmentReplyMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int VideoSegmentReplyMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int VideoSegmentReplyMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return FD_ISEDITABLE;
        case 1: return FD_ISEDITABLE;
        case 2: return FD_ISEDITABLE;
        case 3: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *VideoSegmentReplyMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "title";
        case 1: return "type";
        case 2: return "qualityLevel";
        case 3: return "segmentNumber";
        default: return NULL;
    }
}

const char *VideoSegmentReplyMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "string";
        case 1: return "string";
        case 2: return "int";
        case 3: return "int";
        default: return NULL;
    }
}

const char *VideoSegmentReplyMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int VideoSegmentReplyMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentReplyMessage *pp = (VideoSegmentReplyMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool VideoSegmentReplyMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentReplyMessage *pp = (VideoSegmentReplyMessage *)object; (void)pp;
    switch (field) {
        case 0: oppstring2string(pp->getTitle(),resultbuf,bufsize); return true;
        case 1: oppstring2string(pp->getType(),resultbuf,bufsize); return true;
        case 2: long2string(pp->getQualityLevel(),resultbuf,bufsize); return true;
        case 3: long2string(pp->getSegmentNumber(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool VideoSegmentReplyMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentReplyMessage *pp = (VideoSegmentReplyMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setTitle((value)); return true;
        case 1: pp->setType((value)); return true;
        case 2: pp->setQualityLevel(string2long(value)); return true;
        case 3: pp->setSegmentNumber(string2long(value)); return true;
        default: return false;
    }
}

const char *VideoSegmentReplyMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

void *VideoSegmentReplyMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    VideoSegmentReplyMessage *pp = (VideoSegmentReplyMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


