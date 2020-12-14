#pragma once

#include "Define.h"

namespace cc {
namespace pipeline {

class RenderPipeline;
class RenderStage;
struct Camera;

struct CC_DLL RenderFlowInfo {
    String name;
    uint priority = 0;
    uint tag = 0;
    RenderStageList stages;
};

class CC_DLL RenderFlow : public Object {
public:
    RenderFlow() = default;
    virtual ~RenderFlow();

    virtual bool initialize(const RenderFlowInfo &info);
    virtual void activate(RenderPipeline *pipeline);
    virtual void render(Camera *camera);
    virtual void destroy();

    CC_INLINE const String &getName() const { return _name; }
    CC_INLINE uint getPriority() const { return _priority; }
    CC_INLINE uint getTag() const { return _tag; }

protected:
    RenderStageList _stages;
    String _name;
    RenderPipeline *_pipeline = nullptr;
    uint _priority = 0;
    uint _tag = 0;
};

} // namespace pipeline
} // namespace cc
