#include "RenderFlow.h"
#include "RenderStage.h"

namespace cc {
namespace pipeline {

void RenderFlow::initialize(const RenderFlowInfo &info) {
    _name = info.name;
    _priority = info.priority;
    _tag = info.tag;
}

void RenderFlow::activate(RenderPipeline *pipeline) {
    _pipeline = pipeline;
    
    std::sort(_stages.begin(), _stages.end(), [](const RenderStage *s1, const RenderStage *s2) {
        return s1->getPriority() - s2->getPriority();
    });
    
    for (auto stage : _stages)
        stage->activate(pipeline, this);
}

void RenderFlow::render(RenderView *view) {
    for (auto stage : _stages)
        stage->render(view);
}

void RenderFlow::destroy() {
    for (auto stage : _stages)
        stage->destroy();
    
    _stages.clear();
}

} //namespace pipeline
} // namespace cc
