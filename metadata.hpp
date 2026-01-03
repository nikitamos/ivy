#pragma once
#include "vertex-attribs.hpp"
#include "vulkan/vulkan.hpp"
#include <string>

namespace shbind {
struct VertexAttributeMetadata {
  std::string name;
  uint32_t max_component;
  api::VertexAttribute attribute;
};
} // namespace shbind