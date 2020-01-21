#include "CoreStd.h"
#include "GFXInputAssembler.h"

NS_CC_BEGIN

GFXInputAssembler::GFXInputAssembler(GFXDevice* device)
    : _device(device),
      index_buffer_(nullptr),
      indirect_buffer_(nullptr),
      vertex_count_(0),
      first_vertex_(0),
      index_count_(0),
      first_index_(0),
      vertex_offset_(0),
      instance_count_(0),
      first_instance_(0) {
}

GFXInputAssembler::~GFXInputAssembler() {
}

NS_CC_END
