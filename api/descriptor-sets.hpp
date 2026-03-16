#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace ivy::api {
struct Binding {
  uint32_t id;
  vk::DescriptorType type;
  inline vk::WriteDescriptorSet GetWritingTemplate() {
    return vk::WriteDescriptorSet{
        .dstBinding = id,
        .descriptorType = type,
    };
  }
};
struct BufferBindingWrite : Binding {
  vk::DescriptorSetLayoutBinding b;
  // Returns the WriteDescriptorSet for updateing the current binding
  inline vk::WriteDescriptorSet
  UpdateInfo(vk::DescriptorSet set, const vk::DescriptorBufferInfo *buf_info,
             uint32_t count = 1, uint32_t array_element = 0) {
    return vk::WriteDescriptorSet{.dstSet = set,
                                  .dstBinding = id,
                                  .dstArrayElement =
                                      array_element, // TODO: arrays elements!
                                  .descriptorCount = count,
                                  .descriptorType = type,
                                  .pBufferInfo = buf_info};
  }
};
struct ImageBindingWrite : Binding {
  // TBD
};
struct SamplerBindingWrite : Binding {
  // TBD
};
struct CombinedSamplerBindingWrite : Binding {
  // TBD
};
} // namespace ivy::api

using namespace ivy::api;

// Potentially generated code
struct DescriptorSetUpdateInfo {
  std::optional<BufferBindingWrite> field0;
  std::optional<BufferBindingWrite> field1;
  // How to deal with extensions?
  void GetWrites(std::span<vk::WriteDescriptorSet> writes) {
    auto cur = writes.begin();
    auto [a, b] = *this;
    if (cur == writes.end()) {
      throw std::runtime_error("too small");
    }
  }
};

struct DescriptorSet0 {
  vk::DescriptorSetLayoutBinding binding0 /* {...}*/;
  vk::DescriptorSetLayoutBinding binding1 /* {...}*/;
  vk::DescriptorSetLayoutCreateInfo
  CreateInfo(vk::DescriptorSetLayoutCreateFlags flags = {}) {
    return {.flags = flags,
            .bindingCount = 2,
            .pBindings = (vk::DescriptorSetLayoutBinding *)this};
  }
  operator vk::ArrayProxy<vk::DescriptorSetLayoutBinding>() const {
    return {2, (vk::DescriptorSetLayoutBinding *)this};
  }
};
static_assert(offsetof(DescriptorSet0, binding0) == 0);

struct DescriptorSets {
  DescriptorSet0 ds;
  // Array for layouts?
};