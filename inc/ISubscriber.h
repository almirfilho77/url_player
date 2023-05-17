#ifndef __I_SUBSCRIBER_H__
#define __I_SUBSCRIBER_H__

class ISubscriber
{
public:
    inline virtual ~ISubscriber(){ }

    virtual void Update(void *data) = 0; // Must have a way to know what to update based on the thing that changed in the model
};

#endif /* __I_SUBSCRIBER_H__ */