#include "vulkan/vulkan.hpp"
namespace shbind::api {
template <typename T> struct PushConstant {
  vk::PushConstantsInfo FullUpdateInfo(vk::PipelineLayout layout) const {
    return vk::PushConstantsInfo{
      .layout = layout, .offset = 0, .size = sizeof(T),
      .pValues = reinterpret_cast<const void *>(this)
    };
  }
  static constexpr vk::PushConstantRange FullUniversalRange() {
    return vk::PushConstantRange {
      .stageFlags = vk::ShaderStageFlagBits::eAll,
      .offset = 0, .size = sizeof(T),
    };
  }
};
} // namespace shbind::api