//
// Generated file, do not edit! Created by opp_msgc 4.0 from ./EmulationPlugMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "EmulationPlugMsg_m.h"

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




Register_Class(EmulationPlugMsg);

EmulationPlugMsg::EmulationPlugMsg(const char *name, int kind) : cMessage(name,kind)
{
}

EmulationPlugMsg::EmulationPlugMsg(const EmulationPlugMsg& other) : cMessage()
{
    setName(other.getName());
    operator=(other);
}

EmulationPlugMsg::~EmulationPlugMsg()
{
}

EmulationPlugMsg& EmulationPlugMsg::operator=(const EmulationPlugMsg& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    this->callback_var = other.callback_var;
    return *this;
}

void EmulationPlugMsg::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->callback_var);
}

void EmulationPlugMsg::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->callback_var);
}

EmulationPlugCallback& EmulationPlugMsg::getCallback()
{
    return callback_var;
}

void EmulationPlugMsg::setCallback(const EmulationPlugCallback& callback_var)
{
    this->callback_var = callback_var;
}

class EmulationPlugMsgDescriptor : public cClassDescriptor
{
  public:
    EmulationPlugMsgDescriptor();
    virtual ~EmulationPlugMsgDescriptor();

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

Register_ClassDescriptor(EmulationPlugMsgDescriptor);

EmulationPlugMsgDescriptor::EmulationPlugMsgDescriptor() : cClassDescriptor("EmulationPlugMsg", "cMessage")
{
}

EmulationPlugMsgDescriptor::~EmulationPlugMsgDescriptor()
{
}

bool EmulationPlugMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<EmulationPlugMsg *>(obj)!=NULL;
}

const char *EmulationPlugMsgDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int EmulationPlugMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount(object) : 1;
}

unsigned int EmulationPlugMsgDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return FD_ISCOMPOUND;
        default: return 0;
    }
}

const char *EmulationPlugMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "callback";
        default: return NULL;
    }
}

const char *EmulationPlugMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "EmulationPlugCallback";
        default: return NULL;
    }
}

const char *EmulationPlugMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int EmulationPlugMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    EmulationPlugMsg *pp = (EmulationPlugMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

bool EmulationPlugMsgDescriptor::getFieldAsString(void *object, int field, int i, char *resultbuf, int bufsize) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i,resultbuf,bufsize);
        field -= basedesc->getFieldCount(object);
    }
    EmulationPlugMsg *pp = (EmulationPlugMsg *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getCallback(); opp_strprettytrunc(resultbuf,out.str().c_str(),bufsize-1); return true;}
        default: return false;
    }
}

bool EmulationPlugMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    EmulationPlugMsg *pp = (EmulationPlugMsg *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *EmulationPlugMsgDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0: return "EmulationPlugCallback"; break;
        default: return NULL;
    }
}

void *EmulationPlugMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    EmulationPlugMsg *pp = (EmulationPlugMsg *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getCallback()); break;
        default: return NULL;
    }
}


