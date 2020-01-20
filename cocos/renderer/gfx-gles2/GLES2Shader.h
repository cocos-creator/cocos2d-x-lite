#ifndef CC_GFXGLES2_GLES2_SHADER_H_
#define CC_GFXGLES2_GLES2_SHADER_H_

NS_CC_BEGIN

class GLES2GPUShader;

class CC_GLES2_API GLES2Shader : public GFXShader {
 public:
  GLES2Shader(GFXDevice* device);
  ~GLES2Shader();
  
 public:
  bool Initialize(const GFXShaderInfo& info);
  void destroy();
  
  CC_INLINE GLES2GPUShader* gpu_shader() const { return gpu_shader_; }
 private:
  GLES2GPUShader* gpu_shader_;
};

NS_CC_END

#endif
