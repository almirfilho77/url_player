#ifndef __I_SUBJECT_H__
#define __I_SUBJECT_H__

#include "ISubscriber.h"

class ISubject
{
public:
    virtual ~ISubject();

    virtual void Subscribe(ISubscriber *subscriber) = 0;
    virtual void Unsubscribe(ISubscriber *subscriber) = 0;
    virtual void Notify() = 0;
    virtual void Notify(ISubscriber *subscriber) = 0;
};

#endif /* __I_SUBJECT_H__ */