#include "RenderFlow.h"
#include "RenderStage.h"

namespace cc {
namespace pipeline {

RenderFlow::~RenderFlow() {
}

bool RenderFlow::initialize(const RenderFlowInfo &info) {
    _name = info.name;
    _priority = info.priority;
    _tag = info.tag;
    _stages = info.stages;
    return true;
}

void RenderFlow::activate(RenderPipeline *pipeline) {
    _pipeline = pipeline;

    std::sort(_stages.begin(), _stages.end(), [](const RenderStage *s1, const RenderStage *s2) {
        return s1->getPriority() - s2->getPriority();
    });

    for (const auto stage : _stages)
        stage->activate(pipeline, this);
}

void RenderFlow::render(Camera *camera) {
    for (const auto stage : _stages)
        stage->render(camera);
}

void RenderFlow::destroy() {
    for (const auto stage : _stages)
        stage->destroy();

    _stages.clear();
}

} //namespace pipeline
} // namespace cc
