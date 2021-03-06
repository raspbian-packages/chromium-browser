//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayVk.h:
//    Defines the class interface for VertexArrayVk, implementing VertexArrayImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_
#define LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_

#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace gl
{
class DrawCallParams;
}  // namespace gl

namespace rx
{
class BufferVk;

namespace vk
{
class CommandGraphResource;
}  // namespace vk

class VertexArrayVk : public VertexArrayImpl
{
  public:
    VertexArrayVk(const gl::VertexArrayState &state, RendererVk *renderer);
    ~VertexArrayVk() override;

    void destroy(const gl::Context *context) override;

    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            const gl::VertexArray::DirtyAttribBitsArray &attribBits,
                            const gl::VertexArray::DirtyBindingBitsArray &bindingBits) override;

    void getPackedInputDescriptions(vk::PipelineDesc *pipelineDesc);

    gl::Error drawElements(const gl::Context *context,
                           const gl::DrawCallParams &drawCallParams,
                           vk::CommandBuffer *commandBuffer,
                           bool shouldApplyVertexArray);

    void updateDefaultAttrib(RendererVk *renderer,
                             size_t attribIndex,
                             VkBuffer bufferHandle,
                             uint32_t offset);

    angle::Result updateClientAttribs(const gl::Context *context,
                                      const gl::DrawCallParams &drawCallParams);

    angle::Result handleLineLoop(ContextVk *contextVk, const gl::DrawCallParams &drawCallParams);

    const gl::AttribArray<VkBuffer> &getCurrentArrayBufferHandles() const
    {
        return mCurrentArrayBufferHandles;
    }

    const gl::AttribArray<VkDeviceSize> &getCurrentArrayBufferOffsets() const
    {
        return mCurrentArrayBufferOffsets;
    }

    const gl::AttribArray<vk::CommandGraphResource *> &getCurrentArrayBufferResources() const
    {
        return mCurrentArrayBufferResources;
    }

    VkBuffer getCurrentElementArrayBufferHandle() const { return mCurrentElementArrayBufferHandle; }

    VkDeviceSize getCurrentElementArrayBufferOffset() const
    {
        return mCurrentElementArrayBufferOffset;
    }

    void updateCurrentElementArrayBufferOffset(const GLvoid *offset)
    {
        mCurrentElementArrayBufferOffset = reinterpret_cast<VkDeviceSize>(offset);
    }

    vk::CommandGraphResource *getCurrentElementArrayBufferResource() const
    {
        return mCurrentElementArrayBufferResource;
    }

    angle::Result updateIndexTranslation(ContextVk *contextVk,
                                         const gl::DrawCallParams &drawCallParams);

  private:
    // This will update any dirty packed input descriptions, regardless if they're used by the
    // active program. This could lead to slight inefficiencies when the app would repeatedly
    // update vertex info for attributes the program doesn't use, (very silly edge case). The
    // advantage is the cached state then doesn't depend on the Program, so doesn't have to be
    // updated when the active Program changes.
    void updatePackedInputDescriptions();
    void updatePackedInputInfo(uint32_t attribIndex,
                               const gl::VertexBinding &binding,
                               const gl::VertexAttribute &attrib);

    void updateArrayBufferReadDependencies(vk::CommandGraphResource *drawFramebuffer,
                                           const gl::AttributesMask &activeAttribsMask,
                                           Serial serial);

    angle::Result streamIndexData(ContextVk *contextVk,
                                  GLenum indexType,
                                  size_t indexCount,
                                  const void *sourcePointer,
                                  vk::DynamicBuffer *dynamicBuffer);
    angle::Result convertVertexBuffer(ContextVk *contextVk,
                                      BufferVk *srcBuffer,
                                      const gl::VertexBinding &binding,
                                      size_t attribIndex);
    void ensureConversionReleased(RendererVk *renderer, size_t attribIndex);

    angle::Result syncDirtyAttrib(ContextVk *contextVk,
                                  const gl::VertexAttribute &attrib,
                                  const gl::VertexBinding &binding,
                                  size_t attribIndex);

    gl::AttribArray<VkBuffer> mCurrentArrayBufferHandles;
    gl::AttribArray<VkDeviceSize> mCurrentArrayBufferOffsets;
    gl::AttribArray<vk::CommandGraphResource *> mCurrentArrayBufferResources;
    gl::AttribArray<const vk::Format *> mCurrentArrayBufferFormats;
    gl::AttribArray<GLuint> mCurrentArrayBufferStrides;
    gl::AttribArray<vk::DynamicBuffer> mCurrentArrayBufferConversion;
    gl::AttribArray<bool> mCurrentArrayBufferConversionCanRelease;
    VkBuffer mCurrentElementArrayBufferHandle;
    VkDeviceSize mCurrentElementArrayBufferOffset;
    vk::CommandGraphResource *mCurrentElementArrayBufferResource;

    // Keep a cache of binding and attribute descriptions for easy pipeline updates.
    // This is copied out of here into the pipeline description on a Context state change.
    gl::AttributesMask mDirtyPackedInputs;
    vk::VertexInputBindings mPackedInputBindings;
    vk::VertexInputAttributes mPackedInputAttributes;

    vk::DynamicBuffer mDynamicVertexData;
    vk::DynamicBuffer mDynamicIndexData;
    vk::DynamicBuffer mTranslatedByteIndexData;

    vk::LineLoopHelper mLineLoopHelper;
    Optional<GLint> mLineLoopBufferFirstIndex;
    Optional<size_t> mLineLoopBufferLastIndex;
    bool mDirtyLineLoopTranslation;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_
