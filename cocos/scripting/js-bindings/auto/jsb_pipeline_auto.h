#pragma once
#include "base/Config.h"
#if (CC_PLATFORM == CC_PLATFORM_ANDROID || CC_PLATFORM == CC_PLATFORM_MAC_IOS || CC_PLATFORM == CC_PLATFORM_MAC_OSX || CC_PLATFORM == CC_PLATFORM_WINDOWS)

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"

extern se::Object* __jsb_cc_pipeline_RenderQueueDesc_proto;
extern se::Class* __jsb_cc_pipeline_RenderQueueDesc_class;

bool js_register_cc_pipeline_RenderQueueDesc(se::Object* obj);
bool register_all_pipeline(se::Object* obj);

extern se::Object* __jsb_cc_pipeline_RenderPipelineInfo_proto;
extern se::Class* __jsb_cc_pipeline_RenderPipelineInfo_class;

bool js_register_cc_pipeline_RenderPipelineInfo(se::Object* obj);
bool register_all_pipeline(se::Object* obj);

extern se::Object* __jsb_cc_pipeline_RenderPipeline_proto;
extern se::Class* __jsb_cc_pipeline_RenderPipeline_class;

bool js_register_cc_pipeline_RenderPipeline(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_RenderPipeline_activate);
SE_DECLARE_FUNC(js_pipeline_RenderPipeline_render);
SE_DECLARE_FUNC(js_pipeline_RenderPipeline_initialize);
SE_DECLARE_FUNC(js_pipeline_RenderPipeline_destroy);
SE_DECLARE_FUNC(js_pipeline_RenderPipeline_getInstance);

extern se::Object* __jsb_cc_pipeline_ForwardPipeline_proto;
extern se::Class* __jsb_cc_pipeline_ForwardPipeline_class;

bool js_register_cc_pipeline_ForwardPipeline(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_ForwardPipeline_init);
SE_DECLARE_FUNC(js_pipeline_ForwardPipeline_ForwardPipeline);

extern se::Object* __jsb_cc_pipeline_RenderFlowInfo_proto;
extern se::Class* __jsb_cc_pipeline_RenderFlowInfo_class;

bool js_register_cc_pipeline_RenderFlowInfo(se::Object* obj);
bool register_all_pipeline(se::Object* obj);

extern se::Object* __jsb_cc_pipeline_RenderFlow_proto;
extern se::Class* __jsb_cc_pipeline_RenderFlow_class;

bool js_register_cc_pipeline_RenderFlow(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_RenderFlow_activate);
SE_DECLARE_FUNC(js_pipeline_RenderFlow_initialize);
SE_DECLARE_FUNC(js_pipeline_RenderFlow_getTag);

extern se::Object* __jsb_cc_pipeline_ForwardFlow_proto;
extern se::Class* __jsb_cc_pipeline_ForwardFlow_class;

bool js_register_cc_pipeline_ForwardFlow(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_ForwardFlow_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_ForwardFlow_ForwardFlow);

extern se::Object* __jsb_cc_pipeline_RenderStageInfo_proto;
extern se::Class* __jsb_cc_pipeline_RenderStageInfo_class;

bool js_register_cc_pipeline_RenderStageInfo(se::Object* obj);
bool register_all_pipeline(se::Object* obj);

extern se::Object* __jsb_cc_pipeline_RenderStage_proto;
extern se::Class* __jsb_cc_pipeline_RenderStage_class;

bool js_register_cc_pipeline_RenderStage(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_RenderStage_activate);
SE_DECLARE_FUNC(js_pipeline_RenderStage_initialize);
SE_DECLARE_FUNC(js_pipeline_RenderStage_getTag);

extern se::Object* __jsb_cc_pipeline_ForwardStage_proto;
extern se::Class* __jsb_cc_pipeline_ForwardStage_class;

bool js_register_cc_pipeline_ForwardStage(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_ForwardStage_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_ForwardStage_ForwardStage);

extern se::Object* __jsb_cc_pipeline_ShadowFlow_proto;
extern se::Class* __jsb_cc_pipeline_ShadowFlow_class;

bool js_register_cc_pipeline_ShadowFlow(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_ShadowFlow_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_ShadowFlow_ShadowFlow);

extern se::Object* __jsb_cc_pipeline_ShadowStage_proto;
extern se::Class* __jsb_cc_pipeline_ShadowStage_class;

bool js_register_cc_pipeline_ShadowStage(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_ShadowStage_setFramebuffer);
SE_DECLARE_FUNC(js_pipeline_ShadowStage_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_ShadowStage_ShadowStage);

extern se::Object* __jsb_cc_pipeline_UIFlow_proto;
extern se::Class* __jsb_cc_pipeline_UIFlow_class;

bool js_register_cc_pipeline_UIFlow(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_UIFlow_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_UIFlow_UIFlow);

extern se::Object* __jsb_cc_pipeline_UIStage_proto;
extern se::Class* __jsb_cc_pipeline_UIStage_class;

bool js_register_cc_pipeline_UIStage(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_UIStage_getInitializeInfo);
SE_DECLARE_FUNC(js_pipeline_UIStage_UIStage);

extern se::Object* __jsb_cc_pipeline_RenderView_proto;
extern se::Class* __jsb_cc_pipeline_RenderView_class;

bool js_register_cc_pipeline_RenderView(se::Object* obj);
bool register_all_pipeline(se::Object* obj);
SE_DECLARE_FUNC(js_pipeline_RenderView_enable);
SE_DECLARE_FUNC(js_pipeline_RenderView_setExecuteFlows);
SE_DECLARE_FUNC(js_pipeline_RenderView_onGlobalPipelineStateChanged);
SE_DECLARE_FUNC(js_pipeline_RenderView_getName);
SE_DECLARE_FUNC(js_pipeline_RenderView_getPriority);
SE_DECLARE_FUNC(js_pipeline_RenderView_getVisibility);
SE_DECLARE_FUNC(js_pipeline_RenderView_setPriority);
SE_DECLARE_FUNC(js_pipeline_RenderView_setVisibility);
SE_DECLARE_FUNC(js_pipeline_RenderView_initialize);
SE_DECLARE_FUNC(js_pipeline_RenderView_destroy);
SE_DECLARE_FUNC(js_pipeline_RenderView_getFlows);
SE_DECLARE_FUNC(js_pipeline_RenderView_setWindow);
SE_DECLARE_FUNC(js_pipeline_RenderView_isEnabled);
SE_DECLARE_FUNC(js_pipeline_RenderView_RenderView);

#endif //#if (CC_PLATFORM == CC_PLATFORM_ANDROID || CC_PLATFORM == CC_PLATFORM_MAC_IOS || CC_PLATFORM == CC_PLATFORM_MAC_OSX || CC_PLATFORM == CC_PLATFORM_WINDOWS)
