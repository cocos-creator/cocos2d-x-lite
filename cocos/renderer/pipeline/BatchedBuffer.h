#pragma once

#include "Define.h"

namespace cc {
namespace pipeline {
struct PassView;
struct SubModelView;

struct CC_DLL BatchedItem {
    gfx::BufferList vbs;
    vector<uint8_t *> vbDatas;
    gfx::Buffer *indexBuffer = nullptr;
    float *indexData = nullptr;
    uint vbCount = 0;
    uint mergeCount = 0;
    gfx::InputAssembler *ia = nullptr;
    gfx::Buffer *ubo = nullptr;
    std::array<float, UBOLocalBatched::COUNT> uboData;
    gfx::DescriptorSet *descriptorSet = nullptr;
    const PassView *pass = nullptr;
    gfx::Shader *shader = nullptr;
};
typedef vector<BatchedItem> BatchedItemList;
typedef vector<uint> DynamicOffsetList;

class CC_DLL BatchedBuffer : public Object {
public:
    static BatchedBuffer *get(uint pass);
    static BatchedBuffer *get(uint pass, uint extraKey);

    BatchedBuffer(const PassView *pass);
    virtual ~BatchedBuffer();

    void destroy();
    void merge(const SubModelView *, uint passIdx, const ModelView *);
    void clear();
    void setDynamicOffset(uint idx, uint value);

    CC_INLINE const BatchedItemList &getBatches() const { return _batches; }
    CC_INLINE const PassView *getPass() const { return _pass; }
    CC_INLINE const DynamicOffsetList &getDynamicOffset() const { return _dynamicOffsets; }

private:
    static map<uint, map<uint, BatchedBuffer *>> _buffers;
    DynamicOffsetList _dynamicOffsets;
    BatchedItemList _batches;
    const PassView *_pass = nullptr;
    gfx::Device *_device = nullptr;
};

} // namespace pipeline
} // namespace cc
