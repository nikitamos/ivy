#pragma once
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>

namespace shbind::api {
struct VertexAttribute {
  uint32_t location;
  uint32_t component;
  vk::Format format;
};

template <uint32_t VAttrCount, uint32_t VBufLimit = VAttrCount>
struct VertexShaderInputBase {
  struct BufferRef {
    VertexShaderInputBase &pool;
    const size_t buf_id;
    constexpr void Bind(VertexAttribute vert_attr, size_t mem_offset) {
      auto &attr = pool.attrs[vert_attr.location];
      attr.setBinding(buf_id);
      attr.setLocation(vert_attr.location);
      attr.setFormat(vert_attr.format);
      attr.setOffset(mem_offset);
    }
  };

  template <typename T>
  [[nodiscard]]
  constexpr BufferRef
  AddStructBuffer(vk::VertexInputRate rate = vk::VertexInputRate::eVertex) {
    return AddBuffer(sizeof(T), rate);
  }
  [[nodiscard]]
  constexpr BufferRef AddBuffer(uint32_t size, vk::VertexInputRate rate = vk::VertexInputRate::eVertex) {
    bindings[used_bufs] =
        vk::VertexInputBindingDescription{used_bufs, size, rate};
    return BufferRef{*this, used_bufs++};
  }

  // Note: This method causes UB if returned value is used after the object
  // destruction
  constexpr vk::PipelineVertexInputStateCreateInfo
  CreateInfo(vk::PipelineVertexInputStateCreateFlags flags) const {
    return vk::PipelineVertexInputStateCreateInfo(
        vk::StructureType::ePipelineVertexInputStateCreateInfo, nullptr, flags,
        bindings.size(), bindings.data(), used_bufs, attrs.data());
  }

  uint32_t used_bufs = 0;
  std::array<vk::VertexInputBindingDescription, VBufLimit> bindings;
  // The number of locations is determined from the SPIR-V module
  std::array<vk::VertexInputAttributeDescription, VAttrCount> attrs;
};
}