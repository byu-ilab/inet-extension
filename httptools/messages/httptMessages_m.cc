//
// Generated file, do not edit! Created by opp_msgc 4.0 from ./httptMessages.msg.
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




EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("HTTPMessageKind");
    if (!e) enums.getInstance()->add(e = new cEnum("HTTPMessageKind"));
    e->insert(HTTPT_REQUEST_MESSAGE, "HTTPT_REQUEST_MESSAGE");
    e->insert(HTTPT_DELAYED_REQUEST_MESSAGE, "HTTPT_DELAYED_REQUEST_MESSAGE");
    e->insert(HTTPT_RESPONSE_MESSAGE, "HTTPT_RESPONSE_MESSAGE");
    e->insert(HTTPT_DELAYED_RESPONSE_MESSAGE, "HTTPT_DELAYED_RESPONSE_MESSAGE");
);

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("HTTPProtocol");
    if (!e) enums.getInstance()->add(e = new cEnum("HTTPProtocol"));
    e->insert(HTTP_10, "HTTP_10");
    e->insert(HTTP_11, "HTTP_11");
);

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("RequestMethod");
    if (!e) enums.getInstance()->add(e = new cEnum("RequestMethod"));
    e->insert(RM_OPTIONS, "RM_OPTIONS");
    e->insert(RM_GET, "RM_GET");
    e->insert(RM_HEAD, "RM_HEAD");
    e->insert(RM_POST, "RM_POST");
    e->insert(RM_PUT, "RM_PUT");
    e->insert(RM_DELETE, "RM_DELETE");
    e->insert(RM_TRACE, "RM_TRACE");
    e->insert(RM_CONNECT, "RM_CONNECT");
);

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ByteRangeState");
    if (!e) enums.getInstance()->add(e = new cEnum("ByteRangeState"));
    e->insert(BRS_UNSPECIFIED, "BRS_UNSPECIFIED");
    e->insert(BRS_ASTERISK, "BRS_ASTERISK");
);

Register_Class(httptBaseMessage);

httptBaseMessage::httptBaseMessage(const char *name, int kind) : cPacket(name,kind)
{
    this->targetUrl_var = 0;
    this->originatorUrl_var = "";
    this->protocol_var = HTTP_11;
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
        case 2:
            if (!strcmp(propertyname,"enum")) return "HTTPProtocol";
            return NULL;
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
    this->method_var = RM_GET;
    this->uri_var = "";
    this->firstBytePos_var = BRS_UNSPECIFIED;
    this->lastBytePos_var = BRS_UNSPECIFIED;
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
    this->method_var = other.method_var;
    this->uri_var = other.uri_var;
    this->firstBytePos_var = other.firstBytePos_var;
    this->lastBytePos_var = other.lastBytePos_var;
    return *this;
}

void httptRequestMessage::parsimPack(cCommBuffer *b)
{
    httptBaseMessage::parsimPack(b);
    doPacking(b,this->badRequest_var);
    doPacking(b,this->method_var);
    doPacking(b,this->uri_var);
    doPacking(b,this->firstBytePos_var);
    doPacking(b,this->lastBytePos_var);
}

void httptRequestMessage::parsimUnpack(cCommBuffer *b)
{
    httptBaseMessage::parsimUnpack(b);
    doUnpacking(b,this->badRequest_var);
    doUnpacking(b,this->method_var);
    doUnpacking(b,this->uri_var);
    doUnpacking(b,this->firstBytePos_var);
    doUnpacking(b,this->lastBytePos_var);
}

bool httptRequestMessage::badRequest() const
{
    return badRequest_var;
}

void httptRequestMessage::setBadRequest(bool badRequest_var)
{
    this->badRequest_var = badRequest_var;
}

int httptRequestMessage::method() const
{
    return method_var;
}

void httptRequestMessage::setMethod(int method_var)
{
    this->method_var = method_var;
}

const char * httptRequestMessage::uri() const
{
    return uri_var.c_str();
}

void httptRequestMessage::setUri(const char * uri_var)
{
    this->uri_var = uri_var;
}

int httptRequestMessage::firstBytePos() const
{
    return firstBytePos_var;
}

void httptRequestMessage::setFirstBytePos(int firstBytePos_var)
{
    this->firstBytePos_var = firstBytePos_var;
}

int httptRequestMessage::lastBytePos() const
{
    return lastBytePos_var;
}

void httptRequestMessage::setLastBytePos(int lastBytePos_var)
{
    this->lastBytePos_var = lastBytePos_var;
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
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
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
        case 1: return FD_ISEDITABLE;
        case 2: return FD_ISEDITABLE;
        case 3: return FD_ISEDITABLE;
        case 4: return FD_ISEDITABLE;
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
        case 1: return "method";
        case 2: return "uri";
        case 3: return "firstBytePos";
        case 4: return "lastBytePos";
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
        case 1: return "int";
        case 2: return "string";
        case 3: return "int";
        case 4: return "int";
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
        case 1:
            if (!strcmp(propertyname,"enum")) return "RequestMethod";
            return NULL;
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
        case 1: long2string(pp->method(),resultbuf,bufsize); return true;
        case 2: oppstring2string(pp->uri(),resultbuf,bufsize); return true;
        case 3: long2string(pp->firstBytePos(),resultbuf,bufsize); return true;
        case 4: long2string(pp->lastBytePos(),resultbuf,bufsize); return true;
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
        case 1: pp->setMethod(string2long(value)); return true;
        case 2: pp->setUri((value)); return true;
        case 3: pp->setFirstBytePos(string2long(value)); return true;
        case 4: pp->setLastBytePos(string2long(value)); return true;
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
    this->phrase_var = "";
    this->relatedUri_var = "";
    this->firstBytePos_var = BRS_UNSPECIFIED;
    this->lastBytePos_var = BRS_UNSPECIFIED;
    this->instanceLength_var = BRS_UNSPECIFIED;
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
    this->phrase_var = other.phrase_var;
    this->relatedUri_var = other.relatedUri_var;
    this->firstBytePos_var = other.firstBytePos_var;
    this->lastBytePos_var = other.lastBytePos_var;
    this->instanceLength_var = other.instanceLength_var;
    return *this;
}

void httptReplyMessage::parsimPack(cCommBuffer *b)
{
    httptBaseMessage::parsimPack(b);
    doPacking(b,this->result_var);
    doPacking(b,this->contentType_var);
    doPacking(b,this->phrase_var);
    doPacking(b,this->relatedUri_var);
    doPacking(b,this->firstBytePos_var);
    doPacking(b,this->lastBytePos_var);
    doPacking(b,this->instanceLength_var);
}

void httptReplyMessage::parsimUnpack(cCommBuffer *b)
{
    httptBaseMessage::parsimUnpack(b);
    doUnpacking(b,this->result_var);
    doUnpacking(b,this->contentType_var);
    doUnpacking(b,this->phrase_var);
    doUnpacking(b,this->relatedUri_var);
    doUnpacking(b,this->firstBytePos_var);
    doUnpacking(b,this->lastBytePos_var);
    doUnpacking(b,this->instanceLength_var);
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

const char * httptReplyMessage::phrase() const
{
    return phrase_var.c_str();
}

void httptReplyMessage::setPhrase(const char * phrase_var)
{
    this->phrase_var = phrase_var;
}

const char * httptReplyMessage::relatedUri() const
{
    return relatedUri_var.c_str();
}

void httptReplyMessage::setRelatedUri(const char * relatedUri_var)
{
    this->relatedUri_var = relatedUri_var;
}

int httptReplyMessage::firstBytePos() const
{
    return firstBytePos_var;
}

void httptReplyMessage::setFirstBytePos(int firstBytePos_var)
{
    this->firstBytePos_var = firstBytePos_var;
}

int httptReplyMessage::lastBytePos() const
{
    return lastBytePos_var;
}

void httptReplyMessage::setLastBytePos(int lastBytePos_var)
{
    this->lastBytePos_var = lastBytePos_var;
}

int httptReplyMessage::instanceLength() const
{
    return instanceLength_var;
}

void httptReplyMessage::setInstanceLength(int instanceLength_var)
{
    this->instanceLength_var = instanceLength_var;
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
    return basedesc ? 7+basedesc->getFieldCount(object) : 7;
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
        case 2: return FD_ISEDITABLE;
        case 3: return FD_ISEDITABLE;
        case 4: return FD_ISEDITABLE;
        case 5: return FD_ISEDITABLE;
        case 6: return FD_ISEDITABLE;
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
        case 2: return "phrase";
        case 3: return "relatedUri";
        case 4: return "firstBytePos";
        case 5: return "lastBytePos";
        case 6: return "instanceLength";
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
        case 2: return "string";
        case 3: return "string";
        case 4: return "int";
        case 5: return "int";
        case 6: return "int";
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
        case 2: oppstring2string(pp->phrase(),resultbuf,bufsize); return true;
        case 3: oppstring2string(pp->relatedUri(),resultbuf,bufsize); return true;
        case 4: long2string(pp->firstBytePos(),resultbuf,bufsize); return true;
        case 5: long2string(pp->lastBytePos(),resultbuf,bufsize); return true;
        case 6: long2string(pp->instanceLength(),resultbuf,bufsize); return true;
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
        case 2: pp->setPhrase((value)); return true;
        case 3: pp->setRelatedUri((value)); return true;
        case 4: pp->setFirstBytePos(string2long(value)); return true;
        case 5: pp->setLastBytePos(string2long(value)); return true;
        case 6: pp->setInstanceLength(string2long(value)); return true;
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


