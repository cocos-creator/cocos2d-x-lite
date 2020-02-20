#include "GLES2Std.h"
#include "GLES2Queue.h"
#include "GLES2Commands.h"
#include "GLES2CommandBuffer.h"

NS_CC_BEGIN

GLES2Queue::GLES2Queue(GFXDevice* device)
    : GFXQueue(device) {
}

GLES2Queue::~GLES2Queue() {
}

bool GLES2Queue::initialize(const GFXQueueInfo &info) {
  _type = info.type;
  
  return true;
}

void GLES2Queue::destroy() {
}

void GLES2Queue::submit(GFXCommandBuffer** cmd_buffs, uint count) {
  if (!_isAsync) {
    for (uint i = 0; i < count; ++i) {
      GLES2CommandBuffer* cmd_buff = (GLES2CommandBuffer*)cmd_buffs[i];
      GLES2CmdFuncExecuteCmds((GLES2Device*)_device, cmd_buff->_cmdPackage);
      _numDrawCalls += cmd_buff->_numDrawCalls;
      _numTriangles += cmd_buff->_numTriangles;
    }
  }
}

NS_CC_END
