//
// Generated file, do not edit! Created by opp_msgc 4.1 from httptools/messages/httptEventMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "httptEventMessages_m.h"

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




Register_Class(httptServerStatusUpdateMsg);

httptServerStatusUpdateMsg::httptServerStatusUpdateMsg(const char *name, int kind) : cPacket(name,kind)
{
    this->www_var = 0;
    this->setTime_var = 0;
    this->eventKind_var = 0;
    this->pvalue_var = 0;
    this->pamortize_var = 0;
}

httptServerStatusUpdateMsg::httptServerStatusUpdateMsg(const httptServerStatusUpdateMsg& other) : cPacket()
{
    setName(other.getName());
    operator=(other);
}

httptServerStatusUpdateMsg::~httptServerStatusUpdateMsg()
{
}

httptServerStatusUpdateMsg& httptServerStatusUpdateMsg::operator=(const httptServerStatusUpdateMsg& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    this->www_var = other.www_var;
    this->setTime_var = other.setTime_var;
    this->eventKind_var = other.eventKind_var;
    this->pvalue_var = other.pvalue_var;
    this->pamortize_var = other.pamortize_var;
    return *this;
}

void httptServerStatusUpdateMsg::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->www_var);
    doPacking(b,this->setTime_var);
    doPacking(b,this->eventKind_var);
    doPacking(b,this->pvalue_var);
    doPacking(b,this->pamortize_var);
}

void httptServerStatusUpdateMsg::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->www_var);
    doUnpacking(b,this->setTime_var);
    doUnpacking(b,this->eventKind_var);
    doUnpacking(b,this->pvalue_var);
    doUnpacking(b,this->pamortize_var);
}

const char * httptServerStatusUpdateMsg::www() const
{
    return www_var.c_str();
}

void httptServerStatusUpdateMsg::setWww(const char * www_var)
{
    this->www_var = www_var;
}

simtime_t httptServerStatusUpdateMsg::setTime() const
{
    return setTime_var;
}

void httptServerStatusUpdateMsg::setSetTime(simtime_t setTime_var)
{
    this->setTime_var = setTime_var;
}

int httptServerStatusUpdateMsg::eventKind() const
{
    return eventKind_var;
}

void httptServerStatusUpdateMsg::setEventKind(int eventKind_var)
{
    this->eventKind_var = eventKind_var;
}

double httptServerStatusUpdateMsg::pvalue() const
{
    return pvalue_var;
}

void httptServerStatusUpdateMsg::setPvalue(double pvalue_var)
{
    this->pvalue_var = pvalue_var;
}

double httptServerStatusUpdateMsg::pamortize() const
{
    return pamortize_var;
}

void httptServerStatusUpdateMsg::setPamortize(double pamortize_var)
{
    this->pamortize_var = pamortize_var;
}

class httptServerStatusUpdateMsgDescriptor : public cClassDescriptor
{
  public:
    httptServerStatusUpdateMsgDescriptor();
    virtual ~httptServerStatusUpdateMsgDescriptor();

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

Register_ClassDescriptor(httptServerStatusUpdateMsgDescriptor);

httptServerStatusUpdateMsgDescriptor::httptServerStatusUpdateMsgDescriptor() : cClassDescriptor("httptServerStatusUpdateMsg", "cPacket")
{
}

httptServerStatusUpdateMsgDescriptor::~httptServerStatusUpdateMsgDescriptor()
{
}

bool httptServerStatusUpdateMsgDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<httptServerStatusUpdateMsg *>(obj)!=NULL;
}

const char *httptServerStatusUpdateMsgDescriptor::getProperty(const char *propertyname) const
{
    if (!strcmp(propertyname,"omitGetVerb")) return "true";
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int httptServerStatusUpdateMsgDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int httptServerStatusUpdateMsgDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *httptServerStatusUpdateMsgDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "www",
        "setTime",
        "eventKind",
        "pvalue",
        "pamortize",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int httptServerStatusUpdateMsgDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='w' && strcmp(fieldName, "www")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "setTime")==0) return base+1;
    if (fieldName[0]=='e' && strcmp(fieldName, "eventKind")==0) return base+2;
    if (fieldName[0]=='p' && strcmp(fieldName, "pvalue")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "pamortize")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *httptServerStatusUpdateMsgDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "string",
        "simtime_t",
        "int",
        "double",
        "double",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *httptServerStatusUpdateMsgDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int httptServerStatusUpdateMsgDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    httptServerStatusUpdateMsg *pp = (httptServerStatusUpdateMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string httptServerStatusUpdateMsgDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    httptServerStatusUpdateMsg *pp = (httptServerStatusUpdateMsg *)object; (void)pp;
    switch (field) {
        case 0: return oppstring2string(pp->www());
        case 1: return double2string(pp->setTime());
        case 2: return long2string(pp->eventKind());
        case 3: return double2string(pp->pvalue());
        case 4: return double2string(pp->pamortize());
        default: return "";
    }
}

bool httptServerStatusUpdateMsgDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    httptServerStatusUpdateMsg *pp = (httptServerStatusUpdateMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setWww((value)); return true;
        case 1: pp->setSetTime(string2double(value)); return true;
        case 2: pp->setEventKind(string2long(value)); return true;
        case 3: pp->setPvalue(string2double(value)); return true;
        case 4: pp->setPamortize(string2double(value)); return true;
        default: return false;
    }
}

const char *httptServerStatusUpdateMsgDescriptor::getFieldStructName(void *object, int field) const
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
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<5) ? fieldStructNames[field] : NULL;
}

void *httptServerStatusUpdateMsgDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    httptServerStatusUpdateMsg *pp = (httptServerStatusUpdateMsg *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


