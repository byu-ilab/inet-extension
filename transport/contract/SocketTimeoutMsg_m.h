//
// Generated file, do not edit! Created by opp_msgc 4.1 from transport/contract/SocketTimeoutMsg.msg.
//

#ifndef _SOCKETTIMEOUTMSG_M_H_
#define _SOCKETTIMEOUTMSG_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0401
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Class generated from <tt>transport/contract/SocketTimeoutMsg.msg</tt> by opp_msgc.
 * <pre>
 * message SocketTimeoutMsg {
 *     int socketId;
 *     double timeoutInterval;
 * }
 * </pre>
 */
class SocketTimeoutMsg : public ::cMessage
{
  protected:
    int socketId_var;
    double timeoutInterval_var;

    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const SocketTimeoutMsg&);

  public:
    SocketTimeoutMsg(const char *name=NULL, int kind=0);
    SocketTimeoutMsg(const SocketTimeoutMsg& other);
    virtual ~SocketTimeoutMsg();
    SocketTimeoutMsg& operator=(const SocketTimeoutMsg& other);
    virtual SocketTimeoutMsg *dup() const {return new SocketTimeoutMsg(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getSocketId() const;
    virtual void setSocketId(int socketId_var);
    virtual double getTimeoutInterval() const;
    virtual void setTimeoutInterval(double timeoutInterval_var);
};

inline void doPacking(cCommBuffer *b, SocketTimeoutMsg& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, SocketTimeoutMsg& obj) {obj.parsimUnpack(b);}


#endif // _SOCKETTIMEOUTMSG_M_H_
