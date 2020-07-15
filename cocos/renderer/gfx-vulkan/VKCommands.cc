#include "VKStd.h"

#include "VKBuffer.h"
#include "VKCommands.h"
#include "VKContext.h"
#include "VKDevice.h"
#include "VKQueue.h"
#include "VKSPIRV.h"

#include <algorithm>

#define BUFFER_OFFSET(idx) (static_cast<char *>(0) + (idx))

namespace cc {
namespace gfx {

void insertVkDynamicStates(vector<VkDynamicState> &out, const vector<DynamicState> &dynamicStates) {
    for (DynamicState dynamicState : dynamicStates) {
        switch (dynamicState) {
            case DynamicState::VIEWPORT: break; // we make this dynamic by default
            case DynamicState::SCISSOR: break;  // we make this dynamic by default
            case DynamicState::LINE_WIDTH: out.push_back(VK_DYNAMIC_STATE_LINE_WIDTH); break;
            case DynamicState::DEPTH_BIAS: out.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS); break;
            case DynamicState::BLEND_CONSTANTS: out.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS); break;
            case DynamicState::DEPTH_BOUNDS: out.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS); break;
            case DynamicState::STENCIL_WRITE_MASK: out.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK); break;
            case DynamicState::STENCIL_COMPARE_MASK:
                out.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
                out.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
                break;
            default: {
                CCASSERT(false, "Unsupported PrimitiveMode, convert to VkPrimitiveTopology failed.");
                break;
            }
        }
    }
}

void beginOneTimeCommands(CCVKDevice *device, CCVKGPUCommandBuffer *cmdBuff) {
    CCVKGPUQueue *queue = ((CCVKQueue *)device->getQueue())->gpuQueue();
    cmdBuff->level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBuff->queueFamilyIndex = queue->queueFamilyIndex;
    if (queue->maintenanceCmdBuff) {
        cmdBuff->vkCommandBuffer = queue->maintenanceCmdBuff;
    } else {
        device->gpuCommandBufferPool()->request(cmdBuff);
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(cmdBuff->vkCommandBuffer, &beginInfo));
        queue->maintenanceCmdBuff = cmdBuff->vkCommandBuffer;
    }
}

void endOneTimeCommands(CCVKDevice *device, CCVKGPUCommandBuffer *cmdBuff) {
    /* */
    VK_CHECK(vkEndCommandBuffer(cmdBuff->vkCommandBuffer));

    VkFence fence = device->gpuFencePool()->alloc();
    CCVKGPUQueue *queue = ((CCVKQueue *)device->getQueue())->gpuQueue();
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuff->vkCommandBuffer;
    VK_CHECK(vkQueueSubmit(queue->vkQueue, 1, &submitInfo, fence));
    VK_CHECK(vkWaitForFences(device->gpuDevice()->vkDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
    device->gpuCommandBufferPool()->yield(cmdBuff);
    queue->maintenanceCmdBuff = VK_NULL_HANDLE;
    /* */
}

void CCVKCmdFuncCreateRenderPass(CCVKDevice *device, CCVKGPURenderPass *gpuRenderPass) {
    const size_t colorAttachmentCount = gpuRenderPass->colorAttachments.size();
    const size_t hasDepth = gpuRenderPass->depthStencilAttachment.format != Format::UNKNOWN ? 1 : 0;
    vector<VkAttachmentDescription> attachmentDescriptions(colorAttachmentCount + hasDepth);
    gpuRenderPass->clearValues.resize(colorAttachmentCount + hasDepth);
    for (size_t i = 0u; i < colorAttachmentCount; i++) {
        const ColorAttachment &attachment = gpuRenderPass->colorAttachments[i];
        const VkImageLayout beginLayout = MapVkImageLayout(attachment.beginLayout);
        const VkImageLayout endLayout = MapVkImageLayout(attachment.endLayout);
        const VkAccessFlags beginAccessMask = MapVkAccessFlags(attachment.beginLayout);
        const VkAccessFlags endAccessMask = MapVkAccessFlags(attachment.endLayout);
        attachmentDescriptions[i].format = MapVkFormat(attachment.format);
        attachmentDescriptions[i].samples = MapVkSampleCount(attachment.sampleCount);
        attachmentDescriptions[i].loadOp = MapVkLoadOp(attachment.loadOp);
        attachmentDescriptions[i].storeOp = MapVkStoreOp(attachment.storeOp);
        attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[i].initialLayout = beginLayout;
        attachmentDescriptions[i].finalLayout = endLayout;
    }
    if (hasDepth) {
        const DepthStencilAttachment &depthStencilAttachment = gpuRenderPass->depthStencilAttachment;
        const VkImageLayout beginLayout = MapVkImageLayout(depthStencilAttachment.beginLayout);
        const VkImageLayout endLayout = MapVkImageLayout(depthStencilAttachment.endLayout);
        const VkAccessFlags beginAccessMask = MapVkAccessFlags(depthStencilAttachment.beginLayout);
        const VkAccessFlags endAccessMask = MapVkAccessFlags(depthStencilAttachment.endLayout);
        const bool hasStencil = GFX_FORMAT_INFOS[(uint)depthStencilAttachment.format].hasStencil;
        const VkImageAspectFlags aspectMask = hasStencil ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
        attachmentDescriptions[colorAttachmentCount].format = MapVkFormat(depthStencilAttachment.format);
        attachmentDescriptions[colorAttachmentCount].samples = MapVkSampleCount(depthStencilAttachment.sampleCount);
        attachmentDescriptions[colorAttachmentCount].loadOp = MapVkLoadOp(depthStencilAttachment.depthLoadOp);
        attachmentDescriptions[colorAttachmentCount].storeOp = MapVkStoreOp(depthStencilAttachment.depthStoreOp);
        attachmentDescriptions[colorAttachmentCount].stencilLoadOp = MapVkLoadOp(depthStencilAttachment.stencilLoadOp);
        attachmentDescriptions[colorAttachmentCount].stencilStoreOp = MapVkStoreOp(depthStencilAttachment.stencilStoreOp);
        attachmentDescriptions[colorAttachmentCount].initialLayout = beginLayout;
        attachmentDescriptions[colorAttachmentCount].finalLayout = endLayout;
    }

    const size_t subpassCount = gpuRenderPass->subPasses.size();
    vector<VkSubpassDescription> subpassDescriptions(1, {VK_PIPELINE_BIND_POINT_GRAPHICS});
    vector<VkAttachmentReference> attachmentReferences;
    if (subpassCount) { // pass on user-specified subpasses
        subpassDescriptions.resize(subpassCount);
        for (size_t i = 0u; i < subpassCount; i++) {
            const SubPass &subpass = gpuRenderPass->subPasses[i];
            subpassDescriptions[i].pipelineBindPoint = MapVkPipelineBindPoint(subpass.bindPoint);
            // TODO
        }
    } else { // generate a default subpass from attachment info
        for (size_t i = 0u; i < colorAttachmentCount; i++) {
            attachmentReferences.push_back({(uint32_t)i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        attachmentReferences.push_back({(uint32_t)colorAttachmentCount, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
        subpassDescriptions[0].colorAttachmentCount = attachmentReferences.size() - 1;
        subpassDescriptions[0].pColorAttachments = attachmentReferences.data();
        subpassDescriptions[0].pDepthStencilAttachment = &attachmentReferences.back();
    }

    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = subpassDescriptions.size();
    renderPassCreateInfo.pSubpasses = subpassDescriptions.data();

    VK_CHECK(vkCreateRenderPass(device->gpuDevice()->vkDevice, &renderPassCreateInfo, nullptr, &gpuRenderPass->vkRenderPass));
}

void CCVKCmdFuncDestroyRenderPass(CCVKDevice *device, CCVKGPURenderPass *gpuRenderPass) {
    if (gpuRenderPass->vkRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device->gpuDevice()->vkDevice, gpuRenderPass->vkRenderPass, nullptr);
        gpuRenderPass->vkRenderPass = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncGetDeviceQueue(CCVKDevice *device, CCVKGPUQueue *gpuQueue) {
    uint queueType = 0u;
    VkBool32 needPresentable = VK_FALSE;
    switch (gpuQueue->type) {
        case QueueType::GRAPHICS:
            queueType = VK_QUEUE_GRAPHICS_BIT;
            needPresentable = VK_TRUE;
            break;
        case QueueType::COMPUTE: queueType = VK_QUEUE_COMPUTE_BIT; break;
        case QueueType::TRANSFER: queueType = VK_QUEUE_TRANSFER_BIT; break;
    }

    const CCVKGPUContext *context = ((CCVKContext *)device->getContext())->gpuContext();

    size_t queueCount = context->queueFamilyProperties.size();
    for (size_t i = 0u; i < queueCount; ++i) {
        const VkQueueFamilyProperties &properties = context->queueFamilyProperties[i];
        const VkBool32 isPresentable = context->queueFamilyPresentables[i];
        if (properties.queueCount > 0 && (properties.queueFlags & queueType) && (!needPresentable || isPresentable)) {
            vkGetDeviceQueue(device->gpuDevice()->vkDevice, i, 0, &gpuQueue->vkQueue);
            gpuQueue->queueFamilyIndex = i;
            break;
        }
    }
}

void CCVKCmdFuncCreateBuffer(CCVKDevice *device, CCVKGPUBuffer *gpuBuffer) {
    if (!gpuBuffer->size) {
        return;
    }

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = gpuBuffer->size;
    bufferInfo.usage = MapVkBufferUsageFlagBits(gpuBuffer->usage);

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (gpuBuffer->memUsage == MemoryUsage::HOST) {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    } else if (gpuBuffer->memUsage == MemoryUsage::DEVICE) {
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    } else if (gpuBuffer->memUsage == (MemoryUsage::HOST | MemoryUsage::DEVICE)) {
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    }

    VmaAllocationInfo res;
    VK_CHECK(vmaCreateBuffer(device->gpuDevice()->memoryAllocator, &bufferInfo, &allocInfo, &gpuBuffer->vkBuffer, &gpuBuffer->vmaAllocation, &res));
    //CC_LOG_DEBUG("Allocated buffer: %llu, %llx %llx %llu %x", res.size, gpuBuffer->vkBuffer, res.deviceMemory, res.offset, res.pMappedData);

    gpuBuffer->mappedData = (uint8_t *)res.pMappedData;
    gpuBuffer->startOffset = 0; // we are creating one VkBuffer each for now
}

void CCVKCmdFuncDestroyBuffer(CCVKDevice *device, CCVKGPUBuffer *gpuBuffer) {
    if (gpuBuffer->vmaAllocation) {
        vmaDestroyBuffer(device->gpuDevice()->memoryAllocator, gpuBuffer->vkBuffer, gpuBuffer->vmaAllocation);
        gpuBuffer->buffer = VK_NULL_HANDLE;
        gpuBuffer->vmaAllocation = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncResizeBuffer(CCVKDevice *device, CCVKGPUBuffer *gpuBuffer) {
    CCVKCmdFuncDestroyBuffer(device, gpuBuffer);
    CCVKCmdFuncCreateBuffer(device, gpuBuffer);
}

void CCVKCmdFuncUpdateBuffer(CCVKDevice *device, CCVKGPUBuffer *gpuBuffer, void *buffer, uint offset, uint size) {
    if (!gpuBuffer) return;

    const void *dataToUpload = nullptr;
    size_t sizeToUpload = 0u;

    if (gpuBuffer->usage & BufferUsageBit::INDIRECT) {
        size_t drawInfoCount = size / sizeof(DrawInfo);
        DrawInfo *drawInfo = static_cast<DrawInfo *>(buffer);
        if (drawInfoCount > 0) {
            if (drawInfo->indexCount) {
                for (size_t i = 0; i < drawInfoCount; i++) {
                    gpuBuffer->indexedIndirectCmds[i].indexCount = drawInfo->indexCount;
                    gpuBuffer->indexedIndirectCmds[i].instanceCount = std::max(drawInfo->instanceCount, 1u);
                    gpuBuffer->indexedIndirectCmds[i].firstIndex = drawInfo->firstIndex;
                    gpuBuffer->indexedIndirectCmds[i].vertexOffset = drawInfo->vertexOffset;
                    gpuBuffer->indexedIndirectCmds[i].firstInstance = drawInfo->firstInstance;
                    drawInfo++;
                }
                dataToUpload = gpuBuffer->indexedIndirectCmds.data();
                sizeToUpload = drawInfoCount * sizeof(VkDrawIndexedIndirectCommand);
                gpuBuffer->isDrawIndirectByIndex = true;
            } else {
                for (size_t i = 0; i < drawInfoCount; i++) {
                    gpuBuffer->indirectCmds[i].vertexCount = drawInfo->vertexCount;
                    gpuBuffer->indirectCmds[i].instanceCount = drawInfo->instanceCount;
                    gpuBuffer->indirectCmds[i].firstVertex = drawInfo->firstVertex;
                    gpuBuffer->indirectCmds[i].firstInstance = drawInfo->firstInstance;
                    drawInfo++;
                }
                dataToUpload = gpuBuffer->indirectCmds.data();
                sizeToUpload = drawInfoCount * sizeof(VkDrawIndirectCommand);
                gpuBuffer->isDrawIndirectByIndex = false;
            }
        }
    } else {
        dataToUpload = buffer;
        sizeToUpload = size;
    }

    if (gpuBuffer->mappedData) {
        memcpy(gpuBuffer->mappedData + offset, dataToUpload, sizeToUpload);
    } else {
        CCVKGPUBuffer stagingBuffer;
        stagingBuffer.size = sizeToUpload;
        device->gpuStagingBufferPool()->alloc(&stagingBuffer);
        memcpy(stagingBuffer.mappedData, dataToUpload, sizeToUpload);

        CCVKGPUCommandBuffer cmdBuff;
        beginOneTimeCommands(device, &cmdBuff);

        VkBufferCopy region{stagingBuffer.startOffset, gpuBuffer->startOffset + offset, sizeToUpload};
        vkCmdCopyBuffer(cmdBuff.vkCommandBuffer, stagingBuffer.vkBuffer, gpuBuffer->vkBuffer, 1, &region);

        endOneTimeCommands(device, &cmdBuff);
    }
}

void CCVKCmdFuncCreateTexture(CCVKDevice *device, CCVKGPUTexture *gpuTexture) {
    if (!gpuTexture->size) return;

    VkFormat format = MapVkFormat(gpuTexture->format);
    VkFormatFeatureFlags features = MapVkFormatFeatureFlags(gpuTexture->usage);
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device->gpuContext()->physicalDevice, format, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & features)) {
        const char *formatName = GFX_FORMAT_INFOS[(uint)gpuTexture->format].name.c_str();
        CC_LOG_ERROR("CCVKCmdFuncCreateTexture: The specified usage for %s is not supported on this platform", formatName);
        return;
    }

    VkImageUsageFlags usageFlags = MapVkImageUsageFlagBits(gpuTexture->usage);
    if (gpuTexture->flags & TextureFlags::GEN_MIPMAP) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    createInfo.flags = MapVkImageCreateFlags(gpuTexture->type);
    createInfo.imageType = MapVkImageType(gpuTexture->type);
    createInfo.format = format;
    createInfo.extent = {gpuTexture->width, gpuTexture->height, gpuTexture->depth};
    createInfo.mipLevels = gpuTexture->mipLevel;
    createInfo.arrayLayers = gpuTexture->arrayLayer;
    createInfo.samples = MapVkSampleCount(gpuTexture->samples);
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usageFlags;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaAllocationInfo res;
    VK_CHECK(vmaCreateImage(device->gpuDevice()->memoryAllocator, &createInfo, &allocInfo, &gpuTexture->vkImage, &gpuTexture->vmaAllocation, &res));
    //CC_LOG_DEBUG("Allocated texture: %llu %llx %llx %llu %x", res.size, gpuTexture->vkImage, res.deviceMemory, res.offset, res.pMappedData);

    gpuTexture->layout = MapVkImageLayout(gpuTexture->usage, gpuTexture->format);
    gpuTexture->accessMask = MapVkAccessFlags(gpuTexture->usage, gpuTexture->format);
    gpuTexture->aspectMask = MapVkImageAspectFlags(gpuTexture->format);
    gpuTexture->targetStage = MapVkPipelineStageFlags(gpuTexture->usage);
}

void CCVKCmdFuncDestroyTexture(CCVKDevice *device, CCVKGPUTexture *gpuTexture) {
    if (gpuTexture->vmaAllocation) {
        vmaDestroyImage(device->gpuDevice()->memoryAllocator, gpuTexture->vkImage, gpuTexture->vmaAllocation);
        gpuTexture->vkImage = VK_NULL_HANDLE;
        gpuTexture->vmaAllocation = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncResizeTexture(CCVKDevice *device, CCVKGPUTexture *gpuTexture) {
    CCVKCmdFuncDestroyTexture(device, gpuTexture);
    CCVKCmdFuncCreateTexture(device, gpuTexture);
}

void CCVKCmdFuncCreateTextureView(CCVKDevice *device, CCVKGPUTextureView *gpuTextureView) {
    if (!gpuTextureView->gpuTexture || !gpuTextureView->gpuTexture->vkImage) return;

    VkImageViewCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    createInfo.image = gpuTextureView->gpuTexture->vkImage;
    createInfo.viewType = MapVkImageViewType(gpuTextureView->type);
    createInfo.format = MapVkFormat(gpuTextureView->format);
    createInfo.subresourceRange.aspectMask = GFX_FORMAT_INFOS[(uint)gpuTextureView->format].hasDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = gpuTextureView->baseLevel;
    createInfo.subresourceRange.levelCount = gpuTextureView->levelCount;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = gpuTextureView->gpuTexture->arrayLayer;

    VK_CHECK(vkCreateImageView(device->gpuDevice()->vkDevice, &createInfo, nullptr, &gpuTextureView->vkImageView));
}

void CCVKCmdFuncDestroyTextureView(CCVKDevice *device, CCVKGPUTextureView *gpuTextureView) {
    if (gpuTextureView->vkImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device->gpuDevice()->vkDevice, gpuTextureView->vkImageView, nullptr);
        gpuTextureView->vkImageView = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncCreateSampler(CCVKDevice *device, CCVKGPUSampler *gpuSampler) {
    VkSamplerCreateInfo createInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    createInfo.magFilter = VK_FILTERS[(uint)gpuSampler->magFilter];
    createInfo.minFilter = VK_FILTERS[(uint)gpuSampler->minFilter];
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODES[(uint)gpuSampler->mipFilter];
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODES[(uint)gpuSampler->addressU];
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODES[(uint)gpuSampler->addressV];
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODES[(uint)gpuSampler->addressW];
    createInfo.mipLodBias = gpuSampler->mipLODBias;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = (float)gpuSampler->maxAnisotropy;
    createInfo.compareEnable = VK_TRUE;
    createInfo.compareOp = VK_CMP_FUNCS[(uint)gpuSampler->cmpFunc];
    createInfo.minLod = (float)gpuSampler->minLOD;
    createInfo.maxLod = (float)gpuSampler->maxLOD;
    //createInfo.borderColor; // TODO

    VK_CHECK(vkCreateSampler(device->gpuDevice()->vkDevice, &createInfo, nullptr, &gpuSampler->vkSampler));
}

void CCVKCmdFuncDestroySampler(CCVKDevice *device, CCVKGPUSampler *gpuSampler) {
    if (gpuSampler->vkSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device->gpuDevice()->vkDevice, gpuSampler->vkSampler, nullptr);
        gpuSampler->vkSampler = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncCreateShader(CCVKDevice *device, CCVKGPUShader *gpuShader) {
    for (CCVKGPUShaderStage &stage : gpuShader->gpuStages) {
        vector<unsigned int> spirv = GLSL2SPIRV(stage.type, "#version 450\n" + stage.source, ((CCVKContext *)device->getContext())->minorVersion());
        VkShaderModuleCreateInfo createInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        createInfo.codeSize = spirv.size() * sizeof(unsigned int);
        createInfo.pCode = spirv.data();
        VK_CHECK(vkCreateShaderModule(device->gpuDevice()->vkDevice, &createInfo, nullptr, &stage.vkShader));
    }
    CC_LOG_INFO("Shader '%s' compilation succeeded.", gpuShader->name.c_str());

    gpuShader->vkDescriptorSetLayouts.resize(1);

    const UniformBlockList &blocks = gpuShader->blocks;
    const UniformSamplerList &samplers = gpuShader->samplers;
    const uint bindingCount = blocks.size() + samplers.size();

    vector<VkDescriptorSetLayoutBinding> setBindings(bindingCount);
    for (size_t i = 0u; i < blocks.size(); i++) {
        const UniformBlock &binding = blocks[i];
        VkDescriptorSetLayoutBinding &setBinding = setBindings[i];
        setBinding.stageFlags = MapVkShaderStageFlags(binding.shaderStages);
        setBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setBinding.binding = binding.binding;
        setBinding.descriptorCount = 1;
    }
    for (size_t i = 0u; i < samplers.size(); i++) {
        const UniformSampler &binding = samplers[i];
        VkDescriptorSetLayoutBinding &setBinding = setBindings[blocks.size() + i];
        setBinding.stageFlags = MapVkShaderStageFlags(binding.shaderStages);
        setBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        setBinding.binding = binding.binding;
        setBinding.descriptorCount = binding.count;
    }

    VkDescriptorSetLayoutCreateInfo setCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    if (device->isPushDescriptorSetSupported()) setCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    setCreateInfo.bindingCount = bindingCount;
    setCreateInfo.pBindings = setBindings.data();
    VK_CHECK(vkCreateDescriptorSetLayout(device->gpuDevice()->vkDevice, &setCreateInfo, nullptr, &gpuShader->vkDescriptorSetLayouts[0]));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutCreateInfo.setLayoutCount = gpuShader->vkDescriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = gpuShader->vkDescriptorSetLayouts.data();
    VK_CHECK(vkCreatePipelineLayout(device->gpuDevice()->vkDevice, &pipelineLayoutCreateInfo, nullptr, &gpuShader->vkPipelineLayout));

    if (device->isDescriptorUpdateTemplateSupported()) {
        gpuShader->vkDescriptorUpdateTemplates.resize(1);

        vector<VkDescriptorUpdateTemplateEntry> entries(bindingCount);
        for (size_t i = 0u; i < bindingCount; i++) {
            entries[i].dstBinding = setBindings[i].binding;
            entries[i].dstArrayElement = 0;
            entries[i].descriptorCount = 1;
            entries[i].descriptorType = setBindings[i].descriptorType;
            entries[i].offset = sizeof(CCVKDescriptorInfo) * i;
            entries[i].stride = sizeof(CCVKDescriptorInfo);
        }

        VkDescriptorUpdateTemplateCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO};
        createInfo.descriptorUpdateEntryCount = bindingCount;
        createInfo.pDescriptorUpdateEntries = entries.data();
        if (device->isPushDescriptorSetSupported()) {
            createInfo.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
            createInfo.descriptorSetLayout = VK_NULL_HANDLE;
        } else {
            createInfo.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
            createInfo.descriptorSetLayout = gpuShader->vkDescriptorSetLayouts[0];
        }
        createInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        createInfo.pipelineLayout = gpuShader->vkPipelineLayout;
        VK_CHECK(vkCreateDescriptorUpdateTemplateKHR(device->gpuDevice()->vkDevice, &createInfo, nullptr, &gpuShader->vkDescriptorUpdateTemplates[0]));
    } else {
        gpuShader->manualDescriptorUpdateTemplates.resize(1);
        vector<VkWriteDescriptorSet> &entries = gpuShader->manualDescriptorUpdateTemplates[0];
        entries.resize(bindingCount, {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});
        for (size_t i = 0u; i < bindingCount; i++) {
            entries[i].dstBinding = setBindings[i].binding;
            entries[i].dstArrayElement = 0;
            entries[i].descriptorCount = 1;
            entries[i].descriptorType = setBindings[i].descriptorType;
        }
    }
}

void CCVKCmdFuncDestroyShader(CCVKDevice *device, CCVKGPUShader *gpuShader) {
    for (uint i = 0u; i < gpuShader->vkDescriptorUpdateTemplates.size(); i++) {
        if (gpuShader->vkDescriptorUpdateTemplates[i] != VK_NULL_HANDLE) {
            vkDestroyDescriptorUpdateTemplateKHR(device->gpuDevice()->vkDevice, gpuShader->vkDescriptorUpdateTemplates[i], nullptr);
        }
    }
    gpuShader->vkDescriptorUpdateTemplates.clear();

    if (gpuShader->vkPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->gpuDevice()->vkDevice, gpuShader->vkPipelineLayout, nullptr);
        gpuShader->vkPipelineLayout = VK_NULL_HANDLE;
    }

    for (uint i = 0u; i < gpuShader->vkDescriptorSetLayouts.size(); i++) {
        if (gpuShader->vkDescriptorSetLayouts[i] != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device->gpuDevice()->vkDevice, gpuShader->vkDescriptorSetLayouts[i], nullptr);
        }
    }
    gpuShader->vkDescriptorSetLayouts.clear();

    for (CCVKGPUShaderStage &stage : gpuShader->gpuStages) {
        vkDestroyShaderModule(device->gpuDevice()->vkDevice, stage.vkShader, nullptr);
        stage.vkShader = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncCreateInputAssembler(CCVKDevice *device, CCVKGPUInputAssembler *gpuInputAssembler) {
    size_t vbCount = gpuInputAssembler->gpuVertexBuffers.size();
    gpuInputAssembler->vertexBuffers.resize(vbCount);
    gpuInputAssembler->vertexBufferOffsets.resize(vbCount);

    for (size_t i = 0u; i < vbCount; i++) {
        gpuInputAssembler->vertexBuffers[i] = gpuInputAssembler->gpuVertexBuffers[i]->vkBuffer;
        gpuInputAssembler->vertexBufferOffsets[i] = gpuInputAssembler->gpuVertexBuffers[i]->startOffset;
    }
}

void CCVKCmdFuncDestroyInputAssembler(CCVKDevice *device, CCVKGPUInputAssembler *gpuInputAssembler) {
    gpuInputAssembler->vertexBuffers.clear();
    gpuInputAssembler->vertexBufferOffsets.clear();
}

void CCVKCmdFuncCreateFramebuffer(CCVKDevice *device, CCVKGPUFramebuffer *gpuFramebuffer) {
    size_t colorViewCount = gpuFramebuffer->gpuColorViews.size();
    bool depthSpecified = gpuFramebuffer->gpuDepthStencilView;
    bool hasDepth = gpuFramebuffer->gpuRenderPass->depthStencilAttachment.format == device->getDepthStencilFormat();
    vector<VkImageView> attachments(colorViewCount + (depthSpecified || hasDepth ? 1 : 0));
    uint swapchainImageIndices = 0;
    CCVKGPUTexture *gpuTexture = nullptr;
    for (size_t i = 0u; i < colorViewCount; i++) {
        CCVKGPUTextureView *texView = gpuFramebuffer->gpuColorViews[i];
        if (texView) {
            gpuTexture = gpuFramebuffer->gpuColorViews[i]->gpuTexture;
            attachments[i] = gpuFramebuffer->gpuColorViews[i]->vkImageView;
        } else {
            swapchainImageIndices |= (1 << i);
        }
    }
    if (depthSpecified) {
        gpuTexture = gpuFramebuffer->gpuDepthStencilView->gpuTexture;
        attachments[colorViewCount] = gpuFramebuffer->gpuDepthStencilView->vkImageView;
    } else if (hasDepth) {
        swapchainImageIndices |= (1 << colorViewCount);
    }
    gpuFramebuffer->isOffscreen = !swapchainImageIndices;

    if (gpuFramebuffer->isOffscreen) {
        VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        createInfo.renderPass = gpuFramebuffer->gpuRenderPass->vkRenderPass;
        createInfo.attachmentCount = attachments.size();
        createInfo.pAttachments = attachments.data();
        createInfo.width = gpuTexture ? gpuTexture->width : 1;
        createInfo.height = gpuTexture ? gpuTexture->height : 1;
        createInfo.layers = 1;
        VK_CHECK(vkCreateFramebuffer(device->gpuDevice()->vkDevice, &createInfo, nullptr, &gpuFramebuffer->vkFramebuffer));
    } else {
        // swapchain-related framebuffers need special treatments: rebuild is needed
        // whenever a user-specified attachment or swapchain itself is changed

        gpuFramebuffer->swapchain = device->gpuSwapchain();
        FramebufferListMap &fboListMap = gpuFramebuffer->swapchain->vkSwapchainFramebufferListMap;
        FramebufferListMapIter fboListMapIter = fboListMap.find(gpuFramebuffer);
        if (fboListMapIter != fboListMap.end() && fboListMapIter->second.size()) {
            return;
        }
        size_t swapchainImageCount = gpuFramebuffer->swapchain->vkSwapchainImageViews.size();
        if (fboListMapIter != fboListMap.end()) {
            fboListMapIter->second.resize(swapchainImageCount);
        } else {
            fboListMap[gpuFramebuffer] = FramebufferList(swapchainImageCount);
        }
        VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        createInfo.renderPass = gpuFramebuffer->gpuRenderPass->vkRenderPass;
        createInfo.attachmentCount = attachments.size();
        createInfo.pAttachments = attachments.data();
        createInfo.width = device->getWidth();
        createInfo.height = device->getHeight();
        createInfo.layers = 1;
        for (size_t i = 0u; i < swapchainImageCount; i++) {
            for (size_t j = 0u; j < colorViewCount; j++) {
                if (swapchainImageIndices & (1 << j)) {
                    attachments[j] = gpuFramebuffer->swapchain->vkSwapchainImageViews[i];
                }
            }
            if (swapchainImageIndices & (1 << colorViewCount)) {
                attachments[colorViewCount] = gpuFramebuffer->swapchain->depthStencilImageViews[i];
            }
            VK_CHECK(vkCreateFramebuffer(device->gpuDevice()->vkDevice, &createInfo, nullptr, &fboListMap[gpuFramebuffer][i]));
        }
    }
}

void CCVKCmdFuncDestroyFramebuffer(CCVKDevice *device, CCVKGPUFramebuffer *gpuFramebuffer) {
    if (gpuFramebuffer->isOffscreen) {
        if (gpuFramebuffer->vkFramebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device->gpuDevice()->vkDevice, gpuFramebuffer->vkFramebuffer, nullptr);
            gpuFramebuffer->vkFramebuffer = VK_NULL_HANDLE;
        }
    } else {
        FramebufferListMap &fboListMap = gpuFramebuffer->swapchain->vkSwapchainFramebufferListMap;
        FramebufferListMapIter fboListMapIter = fboListMap.find(gpuFramebuffer);
        if (fboListMapIter != fboListMap.end()) {
            for (size_t i = 0u; i < fboListMapIter->second.size(); i++) {
                vkDestroyFramebuffer(device->gpuDevice()->vkDevice, fboListMapIter->second[i], nullptr);
            }
            fboListMapIter->second.clear();
            fboListMap.erase(fboListMapIter);
        }
    }
}

void CCVKCmdFuncCreatePipelineState(CCVKDevice *device, CCVKGPUPipelineState *gpuPipelineState) {
    VkGraphicsPipelineCreateInfo createInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

    ///////////////////// Shader Stage /////////////////////

    const CCVKGPUShaderStageList &stages = gpuPipelineState->gpuShader->gpuStages;
    const size_t stageCount = stages.size();
    vector<VkPipelineShaderStageCreateInfo> stageInfos(stageCount, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO});
    for (size_t i = 0u; i < stageCount; i++) {
        stageInfos[i].stage = MapVkShaderStageFlagBits(stages[i].type);
        stageInfos[i].module = stages[i].vkShader;
        stageInfos[i].pName = "main";
    }
    createInfo.stageCount = stageCount;
    createInfo.pStages = stageInfos.data();

    ///////////////////// Input State /////////////////////

    const AttributeList &attributes = gpuPipelineState->inputState.attributes;
    const size_t attributeCount = attributes.size();
    size_t bindingCount = 1u;
    for (size_t i = 0u; i < attributeCount; i++) {
        const Attribute &attr = attributes[i];
        bindingCount = std::max((size_t)bindingCount, (size_t)(attr.stream + 1));
    }

    vector<VkVertexInputBindingDescription> bindingDescriptions(bindingCount);
    for (size_t i = 0u; i < bindingCount; i++) {
        bindingDescriptions[i].binding = i;
        bindingDescriptions[i].stride = 0;
        bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    for (size_t i = 0u; i < attributeCount; i++) {
        const Attribute &attr = attributes[i];
        bindingDescriptions[attr.stream].stride += GFX_FORMAT_INFOS[(uint)attr.format].size;
        if (attr.isInstanced) {
            bindingDescriptions[attr.stream].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        }
    }

    const AttributeList &shaderAttrs = gpuPipelineState->gpuShader->attributes;
    const size_t shaderAttrCount = shaderAttrs.size();
    vector<VkVertexInputAttributeDescription> attributeDescriptions(shaderAttrCount);
    vector<uint> offsets(bindingCount, 0);
    bool attributeFound = false;

    for (size_t i = 0; i < shaderAttrCount; i++) {
        attributeFound = false;
        for (const Attribute &attr : attributes) {
            if (shaderAttrs[i].name == attr.name) {
                attributeDescriptions[i].location = shaderAttrs[i].location;
                attributeDescriptions[i].binding = attr.stream;
                attributeDescriptions[i].format = MapVkFormat(attr.format);
                attributeDescriptions[i].offset = offsets[attr.stream];
                offsets[attr.stream] += GFX_FORMAT_INFOS[(uint)attr.format].size;
                attributeFound = true;
                break;
            }
        }
        if (!attributeFound) { //handle absent attribute
            attributeDescriptions[i].location = shaderAttrs[i].location;
            attributeDescriptions[i].format = MapVkFormat(shaderAttrs[i].format);
            attributeDescriptions[i].offset = 0; // reuse the first attribute as dummy data
            CC_LOG_WARNING("Attribute %s is missing, add a dummy data for it.", shaderAttrs[i].name.c_str());
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount = bindingCount;
    vertexInput.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInput.vertexAttributeDescriptionCount = shaderAttrCount;
    vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
    createInfo.pVertexInputState = &vertexInput;

    ///////////////////// Input Asembly State /////////////////////

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_MODES[(uint)gpuPipelineState->primitive];
    createInfo.pInputAssemblyState = &inputAssembly;

    ///////////////////// Dynamic State /////////////////////

    vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    insertVkDynamicStates(dynamicStates, gpuPipelineState->dynamicStates);

    VkPipelineDynamicStateCreateInfo dynamicState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();
    createInfo.pDynamicState = &dynamicState;

    ///////////////////// Viewport State /////////////////////

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1; // dynamic by default
    viewportState.scissorCount = 1;  // dynamic by default
    createInfo.pViewportState = &viewportState;

    ///////////////////// Rasterization State /////////////////////

    VkPipelineRasterizationStateCreateInfo rasterizationState{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    //rasterizationState.depthClampEnable;
    rasterizationState.rasterizerDiscardEnable = gpuPipelineState->rs.isDiscard;
    rasterizationState.polygonMode = VK_POLYGON_MODES[(uint)gpuPipelineState->rs.polygonMode];
    rasterizationState.cullMode = VK_CULL_MODES[(uint)gpuPipelineState->rs.cullMode];
    rasterizationState.frontFace = gpuPipelineState->rs.isFrontFaceCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable = gpuPipelineState->rs.depthBiasEnabled;
    rasterizationState.depthBiasConstantFactor = gpuPipelineState->rs.depthBias;
    rasterizationState.depthBiasClamp = gpuPipelineState->rs.depthBiasClamp;
    rasterizationState.depthBiasSlopeFactor = gpuPipelineState->rs.depthBiasSlop;
    rasterizationState.lineWidth = gpuPipelineState->rs.lineWidth;
    createInfo.pRasterizationState = &rasterizationState;

    ///////////////////// Multisample State /////////////////////

    VkPipelineMultisampleStateCreateInfo multisampleState{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.alphaToCoverageEnable = gpuPipelineState->bs.isA2C;
    //multisampleState.sampleShadingEnable;
    //multisampleState.minSampleShading;
    //multisampleState.pSampleMask;
    //multisampleState.alphaToOneEnable;
    createInfo.pMultisampleState = &multisampleState;

    ///////////////////// Depth Stencil State /////////////////////

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable = gpuPipelineState->dss.depthTest;
    depthStencilState.depthWriteEnable = gpuPipelineState->dss.depthWrite;
    depthStencilState.depthCompareOp = VK_CMP_FUNCS[(uint)gpuPipelineState->dss.depthFunc];
    depthStencilState.stencilTestEnable = gpuPipelineState->dss.stencilTestFront;
    depthStencilState.front = {
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilFailOpFront],
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilPassOpFront],
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilZFailOpFront],
        VK_CMP_FUNCS[(uint)gpuPipelineState->dss.stencilFuncFront],
        gpuPipelineState->dss.stencilReadMaskFront,
        gpuPipelineState->dss.stencilWriteMaskFront,
        gpuPipelineState->dss.stencilRefFront,
    };
    depthStencilState.back = {
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilFailOpBack],
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilPassOpBack],
        VK_STENCIL_OPS[(uint)gpuPipelineState->dss.stencilZFailOpBack],
        VK_CMP_FUNCS[(uint)gpuPipelineState->dss.stencilFuncBack],
        gpuPipelineState->dss.stencilReadMaskBack,
        gpuPipelineState->dss.stencilWriteMaskBack,
        gpuPipelineState->dss.stencilRefBack,
    };
    //depthStencilState.depthBoundsTestEnable;
    //depthStencilState.minDepthBounds;
    //depthStencilState.maxDepthBounds;
    createInfo.pDepthStencilState = &depthStencilState;

    ///////////////////// Blend State /////////////////////

    size_t blendTargetCount = gpuPipelineState->bs.targets.size();
    vector<VkPipelineColorBlendAttachmentState> blendTargets(blendTargetCount);
    for (size_t i = 0u; i < blendTargetCount; i++) {
        BlendTarget &target = gpuPipelineState->bs.targets[i];
        blendTargets[i].blendEnable = target.blend;
        blendTargets[i].srcColorBlendFactor = VK_BLEND_FACTORS[(uint)target.blendSrc];
        blendTargets[i].dstColorBlendFactor = VK_BLEND_FACTORS[(uint)target.blendDst];
        blendTargets[i].colorBlendOp = VK_BLEND_OPS[(uint)target.blendEq];
        blendTargets[i].srcAlphaBlendFactor = VK_BLEND_FACTORS[(uint)target.blendSrcAlpha];
        blendTargets[i].dstAlphaBlendFactor = VK_BLEND_FACTORS[(uint)target.blendDstAlpha];
        blendTargets[i].alphaBlendOp = VK_BLEND_OPS[(uint)target.blendAlphaEq];
        blendTargets[i].colorWriteMask = MapVkColorComponentFlags(target.blendColorMask);
    }
    Color &blendColor = gpuPipelineState->bs.blendColor;

    VkPipelineColorBlendStateCreateInfo colorBlendState{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    //colorBlendState.logicOpEnable;
    //colorBlendState.logicOp;
    colorBlendState.attachmentCount = blendTargetCount;
    colorBlendState.pAttachments = blendTargets.data();
    colorBlendState.blendConstants[0] = blendColor.r;
    colorBlendState.blendConstants[1] = blendColor.g;
    colorBlendState.blendConstants[2] = blendColor.b;
    colorBlendState.blendConstants[3] = blendColor.a;
    createInfo.pColorBlendState = &colorBlendState;

    ///////////////////// References /////////////////////

    createInfo.layout = gpuPipelineState->gpuShader->vkPipelineLayout;
    createInfo.renderPass = gpuPipelineState->gpuRenderPass->vkRenderPass;

    ///////////////////// Creation /////////////////////

    VK_CHECK(vkCreateGraphicsPipelines(device->gpuDevice()->vkDevice, gpuPipelineState->vkPipelineCache,
                                       1, &createInfo, nullptr, &gpuPipelineState->vkPipeline));
}

void CCVKCmdFuncDestroyPipelineState(CCVKDevice *device, CCVKGPUPipelineState *gpuPipelineState) {
    if (gpuPipelineState->vkPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->gpuDevice()->vkDevice, gpuPipelineState->vkPipeline, nullptr);
        gpuPipelineState->vkPipeline = VK_NULL_HANDLE;
    }
}

void CCVKCmdFuncCopyBuffersToTexture(CCVKDevice *device, const BufferDataList &buffers, CCVKGPUTexture *gpuTexture, const BufferTextureCopyList &regions) {
    //bool isCompressed = GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed;
    CCVKGPUCommandBuffer cmdBuff;
    beginOneTimeCommands(device, &cmdBuff);

    VkImageMemoryBarrier barriers[2]{};
    barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barriers[0].image = gpuTexture->vkImage;
    barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barriers[0].subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barriers[0].subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    barriers[0].subresourceRange.aspectMask = gpuTexture->aspectMask;
    barriers[0].srcAccessMask = gpuTexture->accessMask;
    barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barriers[0].oldLayout = gpuTexture->currentLayout;
    barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkCmdPipelineBarrier(cmdBuff.vkCommandBuffer, gpuTexture->targetStage, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, barriers);

    uint regionCount = regions.size(), totalSize = 0u;
    vector<uint> regionSizes(regionCount);
    for (size_t i = 0u; i < regionCount; ++i) {
        const BufferTextureCopy &region = regions[i];
        uint w = region.buffStride > 0 ? region.buffStride : region.texExtent.width;
        uint h = region.buffTexHeight > 0 ? region.buffTexHeight : region.texExtent.height;
        totalSize += regionSizes[i] = FormatSize(gpuTexture->format, w, h, region.texExtent.depth);
    }

    CCVKGPUBuffer stagingBuffer;
    stagingBuffer.size = totalSize;
    device->gpuStagingBufferPool()->alloc(&stagingBuffer);

    vector<VkBufferImageCopy> stagingRegions(regionCount);
    VkDeviceSize offset = 0;
    for (size_t i = 0u; i < regionCount; ++i) {
        const BufferTextureCopy &region = regions[i];
        VkBufferImageCopy &stagingRegion = stagingRegions[i];
        stagingRegion.bufferOffset = stagingBuffer.startOffset + offset;
        stagingRegion.bufferRowLength = region.buffStride;
        stagingRegion.bufferImageHeight = region.buffTexHeight;
        stagingRegion.imageSubresource = {gpuTexture->aspectMask, region.texSubres.mipLevel, region.texSubres.baseArrayLayer, region.texSubres.layerCount};
        stagingRegion.imageOffset = {region.texOffset.x, region.texOffset.y, region.texOffset.z};
        stagingRegion.imageExtent = {region.texExtent.width, region.texExtent.height, region.texExtent.depth};

        memcpy(stagingBuffer.mappedData + offset, buffers[i], regionSizes[i]);
        offset += regionSizes[i];
    }

    vkCmdCopyBufferToImage(cmdBuff.vkCommandBuffer, stagingBuffer.vkBuffer, gpuTexture->vkImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, stagingRegions.size(), stagingRegions.data());

    bool layoutReady = false;
    if (gpuTexture->flags & TextureFlags::GEN_MIPMAP) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device->gpuContext()->physicalDevice, MapVkFormat(gpuTexture->format), &formatProperties);
        VkFormatFeatureFlags mipmapFeatures = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

        if (formatProperties.optimalTilingFeatures & mipmapFeatures) {
            const int32_t width = gpuTexture->width;
            const int32_t height = gpuTexture->height;

            VkImageBlit blitInfo{};
            blitInfo.srcSubresource.aspectMask = gpuTexture->aspectMask;
            blitInfo.srcSubresource.layerCount = gpuTexture->arrayLayer;
            blitInfo.dstSubresource.aspectMask = gpuTexture->aspectMask;
            blitInfo.dstSubresource.layerCount = gpuTexture->arrayLayer;
            blitInfo.srcOffsets[1] = {width, height, 1};
            blitInfo.dstOffsets[1] = {std::max(width >> 1, 1), std::max(height >> 1, 1), 1};
            barriers[0].subresourceRange.levelCount = 1;
            barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

            for (uint i = 1u; i < gpuTexture->mipLevel; i++) {
                barriers[0].subresourceRange.baseMipLevel = i - 1;
                vkCmdPipelineBarrier(cmdBuff.vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, barriers);

                blitInfo.srcSubresource.mipLevel = i - 1;
                blitInfo.dstSubresource.mipLevel = i;
                vkCmdBlitImage(cmdBuff.vkCommandBuffer, gpuTexture->vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               gpuTexture->vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitInfo, VK_FILTER_LINEAR);

                const int32_t w = blitInfo.srcOffsets[1].x = blitInfo.dstOffsets[1].x;
                const int32_t h = blitInfo.srcOffsets[1].y = blitInfo.dstOffsets[1].y;
                blitInfo.dstOffsets[1].x = std::max(w >> 1, 1);
                blitInfo.dstOffsets[1].y = std::max(h >> 1, 1);
            }

            barriers[0].subresourceRange.baseMipLevel = gpuTexture->mipLevel - 1;
            barriers[0].dstAccessMask = gpuTexture->accessMask;
            barriers[0].newLayout = gpuTexture->layout;

            barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[1].image = gpuTexture->vkImage;
            barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1].subresourceRange.aspectMask = gpuTexture->aspectMask;
            barriers[1].subresourceRange.baseMipLevel = 0;
            barriers[1].subresourceRange.levelCount = gpuTexture->mipLevel - 1;
            barriers[1].subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barriers[1].dstAccessMask = gpuTexture->accessMask;
            barriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barriers[1].newLayout = gpuTexture->layout;

            vkCmdPipelineBarrier(cmdBuff.vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, gpuTexture->targetStage,
                                 VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2, barriers);
            layoutReady = true;
        } else {
            const char *formatName = GFX_FORMAT_INFOS[(uint)gpuTexture->format].name.c_str();
            CC_LOG_WARNING("CCVKCmdFuncCopyBuffersToTexture: generate mipmap for %s is not supported on this platform", formatName);
        }
    }

    if (!layoutReady) {
        barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barriers[0].dstAccessMask = gpuTexture->accessMask;
        barriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barriers[0].newLayout = gpuTexture->layout;
        vkCmdPipelineBarrier(cmdBuff.vkCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, gpuTexture->targetStage,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, barriers);
    }

    endOneTimeCommands(device, &cmdBuff);

    gpuTexture->currentLayout = gpuTexture->layout;
}

} // namespace gfx
} // namespace cc
