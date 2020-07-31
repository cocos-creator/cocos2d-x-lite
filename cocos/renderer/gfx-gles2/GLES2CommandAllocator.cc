#include "GLES2Std.h"
#include "GLES2CommandAllocator.h"

namespace cc {
namespace gfx {

GLES2CommandAllocator::GLES2CommandAllocator() {
}

GLES2CommandAllocator::~GLES2CommandAllocator() {
}

void GLES2CommandAllocator::clearCmds(GLES2CmdPackage *cmd_package) {
    if (cmd_package->beginRenderPassCmds.size()) {
        beginRenderPassCmdPool.freeCmds(cmd_package->beginRenderPassCmds);
    }
    if (cmd_package->bindStatesCmds.size()) {
        bindStatesCmdPool.freeCmds(cmd_package->bindStatesCmds);
    }
    if (cmd_package->drawCmds.size()) {
        drawCmdPool.freeCmds(cmd_package->drawCmds);
    }
    if (cmd_package->updateBufferCmds.size()) {
        updateBufferCmdPool.freeCmds(cmd_package->updateBufferCmds);
    }
    if (cmd_package->copyBufferToTextureCmds.size()) {
        copyBufferToTextureCmdPool.freeCmds(cmd_package->copyBufferToTextureCmds);
    }

    cmd_package->cmds.clear();
}

} // namespace gfx
} // namespace cc