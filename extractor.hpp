#pragma once
#include "host-types.hpp"
#include "metadata.hpp"
#include "options.hpp"
#include "pipeline-provider.hpp"
#include "spirv.hpp"
#include "spirv_common.hpp"
#include "spirv_cross.hpp"
#include "spirv_cross_containers.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>

#include "api/vertex-attribs.hpp"

namespace shbind {
class BindingsExtractor {
public:
  BindingsExtractor(spirv_cross::Compiler &compiler, HostTypeFactory &factory,
                    GenerationOptions &&opts = GenerationOptions())
      : compiler_(compiler), opts_(std::move(opts)), type_factory_(factory) {}
  void ExtractBindingsFromResources(const spirv_cross::ShaderResources &res);
  void ExtractBindings(PipelineProvider &provider);
  void WriteToStream(std::ostream &out, IWriter &writer);

  BindingsExtractor(const BindingsExtractor &) = delete;
  BindingsExtractor(BindingsExtractor &&) = delete;

  BindingsExtractor &operator=(const BindingsExtractor &) = delete;
  BindingsExtractor &operator=(BindingsExtractor &&) = delete;

  vk::Format TypeToFormat(const spirv_cross::SPIRType &type,
                          uint32_t override_vecsize = 0);
  uint32_t GetLocationFormatComponentCount(vk::Format fmt) const;

  std::string &CanonicalizeName(std::string &name) const;
  [[nodiscard]]
  std::string CanonicalizeName(const std::string &name) const;

private:
  void ExtractPushConstants(const spirv_cross::ShaderResources &res);
  void ExtractDescriptorSets(const spirv_cross::ShaderResources &res);
  void ExtractSpecializationConstants(const spirv_cross::ShaderResources &res);
  void ExtractDescriptorBindingsOfType(
      const spirv_cross::SmallVector<spirv_cross::Resource> &resources,
      vk::DescriptorType type);
  void ExtractVertexAttributes(const std::string &entry_point_name = "");
  void ExtractVertexAttributes(const spirv_cross::SPIREntryPoint &entry_point);
  std::shared_ptr<HostType> ExtractType(spirv_cross::TypeID id);
  void ExtractAllTypes();

  // If name is empty, returns the first entry point with given execution model.
  // Otherwise returns the first entry point with given name and execution
  // model.
  // If such entry point does not exist, return nullopt.
  std::optional<spirv_cross::EntryPoint>
  GetEntryPointOrFirst(const std::string &name, spv::ExecutionModel model);

  spirv_cross::Compiler &compiler_;
  GenerationOptions opts_;
  HostTypeFactory &type_factory_;
  const spirv_cross::SPIREntryPoint *cur_stage_;
  std::map<uint32_t, VertexAttributeMetadata> vertex_attrs_;
  std::unordered_set<uint32_t> tail64_attrs_;
  std::map<uint32_t, DescriptorSetMetadata> descriptor_sets_;
};
} // namespace shbind