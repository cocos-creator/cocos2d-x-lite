#ifndef CC_CORE_GFX_PIPELINE_STATE_H_
#define CC_CORE_GFX_PIPELINE_STATE_H_

#include "GFXDef.h"

NS_CC_BEGIN

class CC_CORE_API GFXPipelineState : public Object {
 public:
  GFXPipelineState(GFXDevice* device);
  virtual ~GFXPipelineState();
  
public:
  virtual bool initialize(const GFXPipelineStateInfo& info) = 0;
  virtual void destroy() = 0;
  
  CC_INLINE GFXDevice* device() const { return _device; }
  CC_INLINE GFXShader* shader() const { return shader_; }
  CC_INLINE GFXPrimitiveMode primitive() const { return primitive_; }
  CC_INLINE const GFXInputState& is() const { return is_; }
  CC_INLINE const GFXRasterizerState& rs() const { return rs_; }
  CC_INLINE const GFXDepthStencilState& dss() const { return dss_; }
  CC_INLINE const GFXBlendState& bs() const { return bs_; }
  CC_INLINE const GFXDynamicStateList& dynamic_states() const { return dynamic_states_; }
  CC_INLINE const GFXPipelineLayout* layout() const { return layout_; }
  CC_INLINE const GFXRenderPass* renderPass() const { return _renderPass; }

protected:
  GFXDevice* _device;
  GFXShader* shader_;
  GFXPrimitiveMode primitive_;
  GFXInputState is_;
  GFXRasterizerState rs_;
  GFXDepthStencilState dss_;
  GFXBlendState bs_;
  GFXDynamicStateList dynamic_states_;
  GFXPipelineLayout* layout_;
  GFXRenderPass* _renderPass;
};

NS_CC_END

#endif // CC_CORE_GFX_PIPELINE_STATE_H_
