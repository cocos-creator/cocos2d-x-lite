#include "CCVKStd.h"
#include "CCVKSampler.h"
#include "CCVKCommands.h"

NS_CC_BEGIN

CCVKSampler::CCVKSampler(GFXDevice* device)
    : GFXSampler(device) {
}

CCVKSampler::~CCVKSampler() {
}

bool CCVKSampler::initialize(const GFXSamplerInfo &info) {
  _name = info.name;
  _minFilter = info.minFilter;
  _magFilter = info.magFilter;
  _mipFilter = info.mipFilter;
  _addressU = info.addressU;
  _addressV = info.addressV;
  _addressW = info.addressW;
  _maxAnisotropy = info.maxAnisotropy;
  _cmpFunc = info.cmpFunc;
  _borderColor = info.borderColor;
  _minLOD = info.minLOD;
  _maxLOD = info.maxLOD;
  _mipLODBias = info.mipLODBias;
  
  _gpuSampler = CC_NEW(CCVKGPUSampler);
  _gpuSampler->minFilter = _minFilter;
  _gpuSampler->magFilter = _magFilter;
  _gpuSampler->mipFilter = _mipFilter;
  _gpuSampler->addressU = _addressU;
  _gpuSampler->addressV = _addressV;
  _gpuSampler->addressW = _addressW;
  _gpuSampler->minLOD = _minLOD;
  _gpuSampler->maxLOD = _maxLOD;
    
    _status = GFXStatus::SUCCESS;
  
  CCVKCmdFuncCreateSampler((CCVKDevice*)_device, _gpuSampler);
  
  return true;
}

void CCVKSampler::destroy() {
  if (_gpuSampler) {
    CCVKCmdFuncDestroySampler((CCVKDevice*)_device, _gpuSampler);
    CC_DELETE(_gpuSampler);
    _gpuSampler = nullptr;
  }
    _status = GFXStatus::UNREADY;
}

NS_CC_END
