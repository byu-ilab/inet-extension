//
// Generated file, do not edit! Created by opp_msgc 4.1 from transport/contract/SocketTimeoutMsg.msg.
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
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
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
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *SocketTimeoutMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "socketId",
        "timeoutInterval",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int SocketTimeoutMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "socketId")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "timeoutInterval")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *SocketTimeoutMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "double",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
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

std::string SocketTimeoutMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    SocketTimeoutMsg *pp = (SocketTimeoutMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getSocketId());
        case 1: return double2string(pp->getTimeoutInterval());
        default: return "";
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
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
    };
    return (field>=0 && field<2) ? fieldStructNames[field] : NULL;
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


