#ifndef CC_CORE_GFX_DEVICE_H_
#define CC_CORE_GFX_DEVICE_H_

#include "GFXDef.h"

NS_CC_BEGIN

class CC_CORE_API GFXDevice : public Object {
 public:
  GFXDevice();
  virtual ~GFXDevice();
  
 public:
  virtual bool initialize(const GFXDeviceInfo& info) = 0;
  virtual void destroy() = 0;
  virtual void resize(uint width, uint height) = 0;
  virtual void present() = 0;
  virtual GFXWindow* createWindow(const GFXWindowInfo& info) = 0;
  virtual GFXQueue* createQueue(const GFXQueueInfo& info) = 0;
  virtual GFXCommandAllocator* createCommandAllocator(const GFXCommandAllocatorInfo& info) = 0;
  virtual GFXCommandBuffer* createCommandBuffer(const GFXCommandBufferInfo& info) = 0;
  virtual GFXBuffer* createBuffer(const GFXBufferInfo& info) = 0;
  virtual GFXTexture* createTexture(const GFXTextureInfo& info) = 0;
  virtual GFXTextureView* createTextureView(const GFXTextureViewInfo& info) = 0;
  virtual GFXSampler* createSampler(const GFXSamplerInfo& info) = 0;
  virtual GFXShader* createShader(const GFXShaderInfo& info) = 0;
  virtual GFXInputAssembler* createInputAssembler(const GFXInputAssemblerInfo& info) = 0;
  virtual GFXRenderPass* createRenderPass(const GFXRenderPassInfo& info) = 0;
  virtual GFXFramebuffer* createFramebuffer(const GFXFramebufferInfo& info) = 0;
  virtual GFXBindingLayout* createBindingLayout(const GFXBindingLayoutInfo& info) = 0;
  virtual GFXPipelineState* createPipelineState(const GFXPipelineStateInfo& info) = 0;
  virtual GFXPipelineLayout* createPipelineLayout(const GFXPipelineLayoutInfo& info) = 0;
  virtual void copyBuffersToTexture(GFXBuffer* src, GFXTexture* dst, const GFXBufferTextureCopyList& regions) = 0;

  CC_INLINE GFXAPI gfxAPI() const { return _api; }
  CC_INLINE uint width() { return _width; }
  CC_INLINE uint height() { return _height; }
  CC_INLINE uint nativeWidth() { return _nativeWidth; }
  CC_INLINE uint nativeHeight() { return _nativeHeight; }
  CC_INLINE GFXMemoryStatus& memoryStatus() { return _memoryStatus; }
  CC_INLINE GFXContext* context() const { return _context; }
  CC_INLINE GFXWindow* mainWindow() const { return _window; }
  CC_INLINE GFXQueue* queue() const { return _queue; }
  CC_INLINE GFXCommandAllocator* commandAllocator() const { return _cmdAllocator; }
  CC_INLINE const String& renderer() const { return _renderer; }
  CC_INLINE const String& vendor() const { return _vendor; }
  CC_INLINE uint numDrawCalls() const { return _numDrawCalls; }
  CC_INLINE uint numTris() const { return _numTriangles; }
  CC_INLINE bool HasFeature(GFXFeature feature) const { return _features[static_cast<uint8_t>(feature)]; }
  
 protected:
  GFXAPI _api = GFXAPI::UNKNOWN;
  String _device_name;
  String _renderer;
  String _vendor;
  String _version;
  bool _features[static_cast<uint8_t>(GFXFeature::COUNT)];
  uint _width = 0;
  uint _height = 0;
  uint _nativeWidth = 0;
  uint _nativeHeight = 0;
  GFXMemoryStatus _memoryStatus;
  intptr_t _windowHandle = 0;
  GFXContext* _context = nullptr;
  GFXWindow* _window = nullptr;
  GFXQueue* _queue = nullptr;
  GFXCommandAllocator* _cmdAllocator = nullptr;
  uint _numDrawCalls = 0;
  uint _numTriangles = 0;
};

NS_CC_END

#endif // CC_CORE_GFX_DEVICE_H_
