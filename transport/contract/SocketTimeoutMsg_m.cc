//
// Generated file, do not edit! Created by opp_msgc 4.0 from ./SocketTimeoutMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "SocketTimeoutMsg_m.h"

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




Register_Class(SocketTimeoutMsg);

SocketTimeoutMsg::SocketTimeoutMsg(const char *name, int kind) : cMessage(name,kind)
{
    this->socketId_var = 0;
    this->timeoutInterval_var = 0;
}

SocketTimeoutMsg::SocketTimeoutMsg(const SocketTimeoutMsg& other) : cMessage()
{
    setName(other.getName());
    operator=(other);
}

SocketTimeoutMsg::~SocketTimeoutMsg()
{
}

SocketTimeoutMsg& SocketTimeoutMsg::operator=(const SocketTimeoutMsg& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    this->socketId_var = other.socketId_var;
    this->timeoutInterval_var = other.timeoutInterval_var;
    return *this;
}

void SocketTimeoutMsg::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->socketId_var);
    doPacking(b,this->timeoutInterval_var);
}

void SocketTimeoutMsg::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->socketId_var);
    doUnpacking(b,this->timeoutInterval_var);
}

int SocketTimeoutMsg::getSocketId() const
{
    return socketId_var;
}

void SocketTimeoutMsg::setSocketId(int socketId_var)
{
    this->socketId_var = socketId_var;
}

double SocketTimeoutMsg::getTimeoutInterval() const
{
    return timeoutInterval_var;
}

void SocketTimeoutMsg::setTimeoutInterval(double timeoutInterval_var)
{
    this->timeoutInterval_var = timeoutInterval_var;
}

class SocketTimeoutMsgDescriptor : public cClassDescriptor
{
  public:
    SocketTimeoutMsgDescriptor();
    virtual ~SocketTimeoutMsgDescriptor();

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

Register_ClassDescriptor(SocketTimeoutMsgDescriptor);

SocketTimeoutMsgDescriptor::SocketTimeoutMsgDescriptor() : cClassDescriptor("SocketTimeoutMsg", "cMessage")
{
}

SocketTimeoutMsgDescriptor::~SocketTimeoutMsgDescriptor()
{
}

bool SocketTimeoutMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<SocketTimeoutMsg *>(obj)!=NULL;
}

const char *SocketTimeoutMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int SocketTimeoutMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int SocketTimeoutMsgDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *SocketTimeoutMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "socketId";
        case 1: return "timeoutInterval";
        default: return NULL;
    }
}

const char *SocketTimeoutMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "int";
        case 1: return "double";
        default: return NULL;
    }
}

const char *SocketTimeoutMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int SocketTimeoutMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    SocketTimeoutMsg *pp = (SocketTimeoutMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool SocketTimeoutMsgDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    SocketTimeoutMsg *pp = (SocketTimeoutMsg *)object; (void)pp;
    switch (field) {
        case 0: long2string(pp->getSocketId(),resultbuf,bufsize); return true;
        case 1: double2string(pp->getTimeoutInterval(),resultbuf,bufsize); return true;
        default: return false;
    }
}

bool SocketTimeoutMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    SocketTimeoutMsg *pp = (SocketTimeoutMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSocketId(string2long(value)); return true;
        case 1: pp->setTimeoutInterval(string2double(value)); return true;
        default: return false;
    }
}

const char *SocketTimeoutMsgDescriptor::getFieldStructName(void *object, int field) const
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

void *SocketTimeoutMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    SocketTimeoutMsg *pp = (SocketTimeoutMsg *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


