//
// Generated file, do not edit! Created by opp_msgc 4.0 from ./httptByteRangeMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "httptByteRangeMessages_m.h"

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




Register_Class(httptByteRangeRequestMessage);

httptByteRangeRequestMessage::httptByteRangeRequestMessage(const char *name, int kind) : httptRequestMessage(name,kind)
{
    this->firstBytePos_var = 0;
    this->lastBytePos_var = -1;
}

httptByteRangeRequestMessage::httptByteRangeRequestMessage(const httptByteRangeRequestMessage& other) : httptRequestMessage()
{
    setName(other.getName());
    operator=(other);
}

httptByteRangeRequestMessage::~httptByteRangeRequestMessage()
{
}

httptByteRangeRequestMessage& httptByteRangeRequestMessage::operator=(const httptByteRangeRequestMessage& other)
{
    if (this==&other) return *this;
    httptRequestMessage::operator=(other);
    this->firstBytePos_var = other.firstBytePos_var;
    this->lastBytePos_var = other.lastBytePos_var;
    return *this;
}

void httptByteRangeRequestMessage::parsimPack(cCommBuffer *b)
{
    httptRequestMessage::parsimPack(b);
    doPacking(b,this->firstBytePos_var);
    doPacking(b,this->lastBytePos_var);
}

void httptByteRangeRequestMessage::parsimUnpack(cCommBuffer *b)
{
    httptRequestMessage::parsimUnpack(b);
    doUnpacking(b,this->firstBytePos_var);
    doUnpacking(b,this->lastBytePos_var);
}

int httptByteRangeRequestMessage::firstBytePos() const
{
    return firstBytePos_var;
}

void httptByteRangeRequestMessage::setFirstBytePos(int firstBytePos_var)
{
    this->firstBytePos_var = firstBytePos_var;
}

int httptByteRangeRequestMessage::lastBytePos() const
{
    return lastBytePos_var;
}

void httptByteRangeRequestMessage::setLastBytePos(int lastBytePos_var)
{
    this->lastBytePos_var = lastBytePos_var;
}

class httptByteRangeRequestMessageDescriptor : public cClassDescriptor
{
  public:
    httptByteRangeRequestMessageDescriptor();
    virtual ~httptByteRangeRequestMessageDescriptor();

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

Register_ClassDescriptor(httptByteRangeRequestMessageDescriptor);

httptByteRangeRequestMessageDescriptor::httptByteRangeRequestMessageDescriptor() : cClassDescriptor("httptByteRangeRequestMessage", "httptRequestMessage")
{
}

httptByteRangeRequestMessageDescriptor::~httptByteRangeRequestMessageDescriptor()
{
}

bool httptByteRangeRequestMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptByteRangeRequestMessage *>(obj)!=NULL;
}

const char *httptByteRangeRequestMessageDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptByteRangeRequestMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int httptByteRangeRequestMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *httptByteRangeRequestMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "firstBytePos";
        case 1: return "lastBytePos";
        default: return NULL;
    }
}

const char *httptByteRangeRequestMessageDescriptor::getFieldTypeString(void *object, int field) const
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

const char *httptByteRangeRequestMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptByteRangeRequestMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeRequestMessage *pp = (httptByteRangeRequestMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool httptByteRangeRequestMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeRequestMessage *pp = (httptByteRangeRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: long2string(pp->firstBytePos(),resultbuf,bufsize); return true;
        case 1: long2string(pp->lastBytePos(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool httptByteRangeRequestMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeRequestMessage *pp = (httptByteRangeRequestMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setFirstBytePos(string2long(value)); return true;
        case 1: pp->setLastBytePos(string2long(value)); return true;
        default: return false;
    }
}

const char *httptByteRangeRequestMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *httptByteRangeRequestMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeRequestMessage *pp = (httptByteRangeRequestMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(httptByteRangeResponseMessage);

httptByteRangeResponseMessage::httptByteRangeResponseMessage(const char *name, int kind) : httptResponseMessage(name,kind)
{
    this->firstBytePos_var = 0;
    this->lastBytePos_var = 0;
    this->instanceLength_var = 0;
}

httptByteRangeResponseMessage::httptByteRangeResponseMessage(const httptByteRangeResponseMessage& other) : httptResponseMessage()
{
    setName(other.getName());
    operator=(other);
}

httptByteRangeResponseMessage::~httptByteRangeResponseMessage()
{
}

httptByteRangeResponseMessage& httptByteRangeResponseMessage::operator=(const httptByteRangeResponseMessage& other)
{
    if (this==&other) return *this;
    httptResponseMessage::operator=(other);
    this->firstBytePos_var = other.firstBytePos_var;
    this->lastBytePos_var = other.lastBytePos_var;
    this->instanceLength_var = other.instanceLength_var;
    return *this;
}

void httptByteRangeResponseMessage::parsimPack(cCommBuffer *b)
{
    httptResponseMessage::parsimPack(b);
    doPacking(b,this->firstBytePos_var);
    doPacking(b,this->lastBytePos_var);
    doPacking(b,this->instanceLength_var);
}

void httptByteRangeResponseMessage::parsimUnpack(cCommBuffer *b)
{
    httptResponseMessage::parsimUnpack(b);
    doUnpacking(b,this->firstBytePos_var);
    doUnpacking(b,this->lastBytePos_var);
    doUnpacking(b,this->instanceLength_var);
}

int httptByteRangeResponseMessage::firstBytePos() const
{
    return firstBytePos_var;
}

void httptByteRangeResponseMessage::setFirstBytePos(int firstBytePos_var)
{
    this->firstBytePos_var = firstBytePos_var;
}

int httptByteRangeResponseMessage::lastBytePos() const
{
    return lastBytePos_var;
}

void httptByteRangeResponseMessage::setLastBytePos(int lastBytePos_var)
{
    this->lastBytePos_var = lastBytePos_var;
}

int httptByteRangeResponseMessage::instanceLength() const
{
    return instanceLength_var;
}

void httptByteRangeResponseMessage::setInstanceLength(int instanceLength_var)
{
    this->instanceLength_var = instanceLength_var;
}

class httptByteRangeResponseMessageDescriptor : public cClassDescriptor
{
  public:
    httptByteRangeResponseMessageDescriptor();
    virtual ~httptByteRangeResponseMessageDescriptor();

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

Register_ClassDescriptor(httptByteRangeResponseMessageDescriptor);

httptByteRangeResponseMessageDescriptor::httptByteRangeResponseMessageDescriptor() : cClassDescriptor("httptByteRangeResponseMessage", "httptResponseMessage")
{
}

httptByteRangeResponseMessageDescriptor::~httptByteRangeResponseMessageDescriptor()
{
}

bool httptByteRangeResponseMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptByteRangeResponseMessage *>(obj)!=NULL;
}

const char *httptByteRangeResponseMessageDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptByteRangeResponseMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int httptByteRangeResponseMessageDescriptor::getFieldTypeFlags(void *object, int field) const
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
        default: return 0;
    }
}

const char *httptByteRangeResponseMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "firstBytePos";
        case 1: return "lastBytePos";
        case 2: return "instanceLength";
        default: return NULL;
    }
}

const char *httptByteRangeResponseMessageDescriptor::getFieldTypeString(void *object, int field) const
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
        case 2: return "int";
        default: return NULL;
    }
}

const char *httptByteRangeResponseMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptByteRangeResponseMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeResponseMessage *pp = (httptByteRangeResponseMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool httptByteRangeResponseMessageDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeResponseMessage *pp = (httptByteRangeResponseMessage *)object; (void)pp;
    switch (field) {
        case 0: long2string(pp->firstBytePos(),resultbuf,bufsize); return true;
        case 1: long2string(pp->lastBytePos(),resultbuf,bufsize); return true;
        case 2: long2string(pp->instanceLength(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool httptByteRangeResponseMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeResponseMessage *pp = (httptByteRangeResponseMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setFirstBytePos(string2long(value)); return true;
        case 1: pp->setLastBytePos(string2long(value)); return true;
        case 2: pp->setInstanceLength(string2long(value)); return true;
        default: return false;
    }
}

const char *httptByteRangeResponseMessageDescriptor::getFieldStructName(void *object, int field) const
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

void *httptByteRangeResponseMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptByteRangeResponseMessage *pp = (httptByteRangeResponseMessage *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


