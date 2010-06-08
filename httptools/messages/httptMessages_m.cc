//
// Generated file, do not edit! Created by opp_msgc 4.0 from httptools/messages/httptMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "httptMessages_m.h"

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




Register_Class(httptBaseMessage);

httptBaseMessage::httptBaseMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->targetUrl_var = 0;
    this->originatorUrl_var = "";
    this->protocol_var = 11;
    this->keepAlive_var = true;
    this->serial_var = 0;
    this->heading_var = "";
    this->payload_var = "";
}

httptBaseMessage::httptBaseMessage(const httptBaseMessage& other) : cPacket()
{
    setName(other.getName());
    operator=(other);
}

httptBaseMessage::~httptBaseMessage()
{
}

httptBaseMessage& httptBaseMessage::operator=(const httptBaseMessage& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    this->targetUrl_var = other.targetUrl_var;
    this->originatorUrl_var = other.originatorUrl_var;
    this->protocol_var = other.protocol_var;
    this->keepAlive_var = other.keepAlive_var;
    this->serial_var = other.serial_var;
    this->heading_var = other.heading_var;
    this->payload_var = other.payload_var;
    return *this;
}

void httptBaseMessage::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->targetUrl_var);
    doPacking(b,this->originatorUrl_var);
    doPacking(b,this->protocol_var);
    doPacking(b,this->keepAlive_var);
    doPacking(b,this->serial_var);
    doPacking(b,this->heading_var);
    doPacking(b,this->payload_var);
}

void httptBaseMessage::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->targetUrl_var);
    doUnpacking(b,this->originatorUrl_var);
    doUnpacking(b,this->protocol_var);
    doUnpacking(b,this->keepAlive_var);
    doUnpacking(b,this->serial_var);
    doUnpacking(b,this->heading_var);
    doUnpacking(b,this->payload_var);
}

const char * httptBaseMessage::targetUrl() const
{
    return targetUrl_var.c_str();
}

void httptBaseMessage::setTargetUrl(const char * targetUrl_var)
{
    this->targetUrl_var = targetUrl_var;
}

const char * httptBaseMessage::originatorUrl() const
{
    return originatorUrl_var.c_str();
}

void httptBaseMessage::setOriginatorUrl(const char * originatorUrl_var)
{
    this->originatorUrl_var = originatorUrl_var;
}

int httptBaseMessage::protocol() const
{
    return protocol_var;
}

void httptBaseMessage::setProtocol(int protocol_var)
{
    this->protocol_var = protocol_var;
}

bool httptBaseMessage::keepAlive() const
{
    return keepAlive_var;
}

void httptBaseMessage::setKeepAlive(bool keepAlive_var)
{
    this->keepAlive_var = keepAlive_var;
}

int httptBaseMessage::serial() const
{
    return serial_var;
}

void httptBaseMessage::setSerial(int serial_var)
{
    this->serial_var = serial_var;
}

const char * httptBaseMessage::heading() const
{
    return heading_var.c_str();
}

void httptBaseMessage::setHeading(const char * heading_var)
{
    this->heading_var = heading_var;
}

const char * httptBaseMessage::payload() const
{
    return payload_var.c_str();
}

void httptBaseMessage::setPayload(const char * payload_var)
{
    this->payload_var = payload_var;
}

class httptBaseMessageDescriptor : public cClassDescriptor
{
  public:
    httptBaseMessageDescriptor();
    virtual ~httptBaseMessageDescriptor();

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

Register_ClassDescriptor(httptBaseMessageDescriptor);

httptBaseMessageDescriptor::httptBaseMessageDescriptor() : cClassDescriptor("httptBaseMessage", "cPacket")
{
}

httptBaseMessageDescriptor::~httptBaseMessageDescriptor()
{
}

bool httptBaseMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptBaseMessage *>(obj)!=NULL;
}

const char *httptBaseMessageDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptBaseMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
}

unsigned int httptBaseMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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
        case 4: return FD_ISEDITABLE;
        case 5: return FD_ISEDITABLE;
        case 6: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *httptBaseMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "targetUrl";
        case 1: return "originatorUrl";
        case 2: return "protocol";
        case 3: return "keepAlive";
        case 4: return "serial";
        case 5: return "heading";
        case 6: return "payload";
        default: return NULL;
    }
}

const char *httptBaseMessageDescriptor::getFieldTypeString(void *object, int field) const
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
        case 3: return "bool";
        case 4: return "int";
        case 5: return "string";
        case 6: return "string";
        default: return NULL;
    }
}

const char *httptBaseMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptBaseMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptBaseMessage *pp = (httptBaseMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool httptBaseMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    httptBaseMessage *pp = (httptBaseMessage *)object; (void)pp;
    switch (field) {
        case 0: oppstring2string(pp->targetUrl(),resultbuf,bufsize); return true;
        case 1: oppstring2string(pp->originatorUrl(),resultbuf,bufsize); return true;
        case 2: long2string(pp->protocol(),resultbuf,bufsize); return true;
        case 3: bool2string(pp->keepAlive(),resultbuf,bufsize); return true;
        case 4: long2string(pp->serial(),resultbuf,bufsize); return true;
        case 5: oppstring2string(pp->heading(),resultbuf,bufsize); return true;
        case 6: oppstring2string(pp->payload(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool httptBaseMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptBaseMessage *pp = (httptBaseMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setTargetUrl((value)); return true;
        case 1: pp->setOriginatorUrl((value)); return true;
        case 2: pp->setProtocol(string2long(value)); return true;
        case 3: pp->setKeepAlive(string2bool(value)); return true;
        case 4: pp->setSerial(string2long(value)); return true;
        case 5: pp->setHeading((value)); return true;
        case 6: pp->setPayload((value)); return true;
        default: return false;
    }
}

const char *httptBaseMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *httptBaseMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptBaseMessage *pp = (httptBaseMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(httptRequestMessage);

httptRequestMessage::httptRequestMessage(const char *name, int kind) : httptBaseMessage(name,kind)
{
    this->badRequest_var = false;
}

httptRequestMessage::httptRequestMessage(const httptRequestMessage& other) : httptBaseMessage()
{
    setName(other.getName());
    operator=(other);
}

httptRequestMessage::~httptRequestMessage()
{
}

httptRequestMessage& httptRequestMessage::operator=(const httptRequestMessage& other)
{
    if (this==&other) return *this;
    httptBaseMessage::operator=(other);
    this->badRequest_var = other.badRequest_var;
    return *this;
}

void httptRequestMessage::parsimPack(cCommBuffer *b)
{
    httptBaseMessage::parsimPack(b);
    doPacking(b,this->badRequest_var);
}

void httptRequestMessage::parsimUnpack(cCommBuffer *b)
{
    httptBaseMessage::parsimUnpack(b);
    doUnpacking(b,this->badRequest_var);
}

bool httptRequestMessage::badRequest() const
{
    return badRequest_var;
}

void httptRequestMessage::setBadRequest(bool badRequest_var)
{
    this->badRequest_var = badRequest_var;
}

class httptRequestMessageDescriptor : public cClassDescriptor
{
  public:
    httptRequestMessageDescriptor();
    virtual ~httptRequestMessageDescriptor();

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

Register_ClassDescriptor(httptRequestMessageDescriptor);

httptRequestMessageDescriptor::httptRequestMessageDescriptor() : cClassDescriptor("httptRequestMessage", "httptBaseMessage")
{
}

httptRequestMessageDescriptor::~httptRequestMessageDescriptor()
{
}

bool httptRequestMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptRequestMessage *>(obj)!=NULL;
}

const char *httptRequestMessageDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptRequestMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int httptRequestMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return FD_ISEDITABLE;
        default: return 0;
    }
}

const char *httptRequestMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "badRequest";
        default: return NULL;
    }
}

const char *httptRequestMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "bool";
        default: return NULL;
    }
}

const char *httptRequestMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptRequestMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptRequestMessage *pp = (httptRequestMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool httptRequestMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    httptRequestMessage *pp = (httptRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: bool2string(pp->badRequest(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool httptRequestMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptRequestMessage *pp = (httptRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setBadRequest(string2bool(value)); return true;
        default: return false;
    }
}

const char *httptRequestMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *httptRequestMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptRequestMessage *pp = (httptRequestMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(httptReplyMessage);

httptReplyMessage::httptReplyMessage(const char *name, int kind) : httptBaseMessage(name,kind)
{
    this->result_var = 0;
    this->contentType_var = 0;
}

httptReplyMessage::httptReplyMessage(const httptReplyMessage& other) : httptBaseMessage()
{
    setName(other.getName());
    operator=(other);
}

httptReplyMessage::~httptReplyMessage()
{
}

httptReplyMessage& httptReplyMessage::operator=(const httptReplyMessage& other)
{
    if (this==&other) return *this;
    httptBaseMessage::operator=(other);
    this->result_var = other.result_var;
    this->contentType_var = other.contentType_var;
    return *this;
}

void httptReplyMessage::parsimPack(cCommBuffer *b)
{
    httptBaseMessage::parsimPack(b);
    doPacking(b,this->result_var);
    doPacking(b,this->contentType_var);
}

void httptReplyMessage::parsimUnpack(cCommBuffer *b)
{
    httptBaseMessage::parsimUnpack(b);
    doUnpacking(b,this->result_var);
    doUnpacking(b,this->contentType_var);
}

int httptReplyMessage::result() const
{
    return result_var;
}

void httptReplyMessage::setResult(int result_var)
{
    this->result_var = result_var;
}

int httptReplyMessage::contentType() const
{
    return contentType_var;
}

void httptReplyMessage::setContentType(int contentType_var)
{
    this->contentType_var = contentType_var;
}

class httptReplyMessageDescriptor : public cClassDescriptor
{
  public:
    httptReplyMessageDescriptor();
    virtual ~httptReplyMessageDescriptor();

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

Register_ClassDescriptor(httptReplyMessageDescriptor);

httptReplyMessageDescriptor::httptReplyMessageDescriptor() : cClassDescriptor("httptReplyMessage", "httptBaseMessage")
{
}

httptReplyMessageDescriptor::~httptReplyMessageDescriptor()
{
}

bool httptReplyMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptReplyMessage *>(obj)!=NULL;
}

const char *httptReplyMessageDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptReplyMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int httptReplyMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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
        default: return 0;
    }
}

const char *httptReplyMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "result";
        case 1: return "contentType";
        default: return NULL;
    }
}

const char *httptReplyMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "int";
        case 1: return "int";
        default: return NULL;
    }
}

const char *httptReplyMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptReplyMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptReplyMessage *pp = (httptReplyMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool httptReplyMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    httptReplyMessage *pp = (httptReplyMessage *)object; (void)pp;
    switch (field) {
        case 0: long2string(pp->result(),resultbuf,bufsize); return true;
        case 1: long2string(pp->contentType(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool httptReplyMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptReplyMessage *pp = (httptReplyMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setResult(string2long(value)); return true;
        case 1: pp->setContentType(string2long(value)); return true;
        default: return false;
    }
}

const char *httptReplyMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *httptReplyMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptReplyMessage *pp = (httptReplyMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


