#pragma once
#include "host-types.hpp"
#include "vertex-attribs.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <string>

namespace shbind {
struct VertexAttributeMetadata {
  std::string name;
  uint32_t max_component;
  api::VertexAttribute attribute;
};
struct DescriptorBindingMetadata {
  vk::DescriptorSetLayoutBinding binding;
  std::string name;
  std::shared_ptr<HostType> type;
};
struct DescriptorSetMetadata {
  std::vector<DescriptorBindingMetadata> bindings;
};
} // namespace shbind