#pragma once

#import <Metal/MTLCommandQueue.h>

NS_CC_BEGIN

class CCMTLCommandPackage;

class CCMTLQueue : public GFXQueue
{
public:
    CCMTLQueue(GFXDevice* device);
    ~CCMTLQueue();
    
    virtual bool Initialize(const GFXQueueInfo &info) override;
    virtual void Destroy() override;
    virtual void submit(GFXCommandBuffer** cmd_buffs, uint count) override;
    
private:
    CC_INLINE void executeCommands(const CCMTLCommandPackage*);
    
private:
    id<MTLCommandQueue> _metalQueue = nil;
    NSAutoreleasePool* _autoReleasePool = nil;
};

NS_CC_END

