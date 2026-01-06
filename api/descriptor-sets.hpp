#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.hpp>

namespace shbind::api {
struct DescriptorSet {
  struct Binding {
    uint32_t id;
    vk::DescriptorType type;
    inline vk::WriteDescriptorSet GetWriteingTemplate() {
      return vk::WriteDescriptorSet {
        .dstBinding = id,
        .descriptorType = type,
      };
    }
  };
  struct BufferBinding : Binding {
    inline vk::WriteDescriptorSet Update(vk::CommandBuffer cmd,
                                         vk::DescriptorSet set,
                                         const vk::DescriptorBufferInfo *buf_info,
                                         uint32_t array_element = 0) {
      return vk::WriteDescriptorSet{.dstSet = set,
                                    .dstBinding = id,
                                    .dstArrayElement = array_element, // TODO: arrays elements!
                                    .descriptorCount = 1,
                                    .descriptorType = type,
                                    .pBufferInfo = buf_info};
    }
  };
};
} // namespace shbind::api