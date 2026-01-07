#include "extractor.hpp"
#include "api/descriptor-sets.hpp"
#include "api/vertex-attribs.hpp"
#include "host-types.hpp"
#include "metadata.hpp"
#include "spirv.hpp"
#include "spirv_common.hpp"
#include "vulkan/vulkan.hpp"
#include "writer.hpp"

#include <algorithm>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan_to_string.hpp>

// ceil(a/b)
template <std::unsigned_integral T>
static inline constexpr T ceildiv(T a, T b) {
  return (a + b - 1) / b;
}

static inline void Throw64BitLocationUnshareble(uint32_t location) {
  throw std::runtime_error(
      "We do not support the same attribute location (" +
      std::to_string(location) +
      ") to be shared by tail of 64bit vector and another variable. "
      "Sufferest thyself.");
}

namespace shbind {
namespace spc = spirv_cross;
void BindingsExtractor::ExtractPushConstants() {
  auto resources = compiler_.get_shader_resources();
  auto constants = resources.push_constant_buffers;

  // "There must be no more than one push constant block statically used per
  // shader entry point"
  //   -- Vulkan Spec

  if (constants.size() > 1) {
    throw std::runtime_error(
        "more than 1 push constant block violate the Vulkan spec");
  }

  if (constants.size() == 0) {
    return;
  }
  auto &push_const = constants[0];
  std::cout << "Push constant `" << push_const.name
            << "` (id: " << push_const.id
            << " base-type-id: " << push_const.base_type_id << " )"
            << std::endl;
  ExtractType(push_const.base_type_id);
  auto ranges = compiler_.get_active_buffer_ranges(push_const.id);
  const auto stages = compiler_.get_entry_points_and_stages();
  for (auto rng : ranges) {
    std::cout << "RANGES: ";
    std::cout << rng.index << "(" << rng.offset << ".."
              << rng.offset + rng.range << ") ";
  }
  std::cout << '\n';
  // What about push constant ranges?
}
void BindingsExtractor::ExtractSpecializationConstants() {}
std::shared_ptr<HostType> BindingsExtractor::ExtractType(spc::TypeID id) {
  const auto &type = compiler_.get_type(id);
  auto res = type_factory_.GetType(type, compiler_);
  return res;
}
void BindingsExtractor::ExtractAllTypes() {}

shbind::CxxModule BindingsExtractor::ExtractBindings() {
  ExtractPushConstants();
  ExtractVertexAttributes();
  ExtractDescriptorSets();
  return CxxModule();
}
void BindingsExtractor::WriteToStream(std::ostream &out, IWriter &writer) {
  // Write prelude
  writer.WritePrelude(out);
  // Write forward decls
  auto types = type_factory_.GetAllKnownTypes();
  for (auto t : types)
    writer.FwdDeclareType(t, out);
  // Write all types
  for (auto t : types)
    writer.DeclareType(t, out);
  // Write classes for bindings
  std::vector<VertexAttributeMetadata> attrs(vertex_attrs_.size());
  std::transform(vertex_attrs_.begin(), vertex_attrs_.end(), attrs.begin(),
                 [](auto attr) { return attr.second; });
  writer.WriteVertexAttributeInterface(attrs, out);
  for (const auto &[idx, set] : descriptor_sets_) {
    writer.WriteDescriptorSetStruct(set, out, idx);
  }
  // End file
  writer.EndWriting(out);
}

void BindingsExtractor::ExtractVertexAttributes(
    const std::string &entry_point_name) {
  const auto point_desc =
      GetEntryPointOrFirst(entry_point_name, spv::ExecutionModelVertex);
  if (!point_desc.has_value()) {
    std::cerr << "No vertex stage found. Ignoring." << std::endl;
    return;
  }
  const auto &entry_point =
      compiler_.get_entry_point(point_desc->name, spv::ExecutionModelVertex);
  for (const auto &var : entry_point.interface_variables) {
    if (compiler_.get_storage_class(var) == spv::StorageClassInput &&
        compiler_.get_decoration(var, spv::DecorationBuiltIn) == 0) {

      uint32_t location =
          compiler_.get_decoration(var, spv::DecorationLocation);
      uint32_t initial_component =
          compiler_.get_decoration(var, spv::DecorationComponent);

      if (tail64_attrs_.contains(location)) {
        Throw64BitLocationUnshareble(location);
      }

      // FIXME: follow the Vulkan spec in unwrapping structs and array
      auto type = compiler_.get_type_from_variable(var);
      if (type.member_types.size() > 1) {
        throw std::runtime_error(
            "Frankenstein structs in shader inputs are unsupported.");
      } else if (type.member_types.size() == 1) {
        type = compiler_.get_type(type.member_types[0]);
      }

      vk::Format fmt = TypeToFormat(type);
      std::string name = CanonicalizeName(compiler_.get_name(var));
      if (name.empty()) {
        name = "inp" + std::to_string(location) + '_' +
               std::to_string(initial_component);
      }

      // Some stupid formats, like vectors of 64bit types and matrices, occupy
      // several consecutive locations. But still, at least matrices, need to be
      // passed as distinct VkVertexInputAttributeDescription's (i.e. one
      // attribute per location). See the following link for details.
      // https://docs.vulkan.org/spec/latest/chapters/fxvertex.html#fxvertex-attrib-location

      // Number of components occupied by the base type
      uint32_t width = type.width == 64 ? 2 : 1;
      uint32_t col_per_item =
          ceildiv(width * type.vecsize, 4u /* len of location */);
      // The number of components occupied in the `location` attribute. The tail
      // of 64bit vectors is ignored.
      uint32_t occupied_comps = std::max(4u, width * type.vecsize);

      // Matrices are passed as arrays, right?
      // Therefore type.columns must always be 1?
      if (type.columns != 1) {
        std::cerr << "A pathological case encountered: matrix in vertex "
                     "attribute binding (variable id "
                  << var
                  << "). Generally matrices should be "
                     "passed as arrays; generated output may be invalid."
                  << std::endl;
      }
      uint32_t occupied_locs =
          std::reduce(type.array.begin(), type.array.end(), type.columns,
                      [](auto x, auto y) { return x * y; });

      // Note: two variables in the same location CAN NOT overlap.
      for (uint32_t col = 0; col < occupied_locs; col += col_per_item) {
        // We declare just one attribute for each 64-bit vector (corresponding
        // to the first locations), but I am not sure that it is the correct
        // behaviour.
        uint32_t cur_loc = location + col;
        if (col_per_item == 2) {
          if (vertex_attrs_.contains(cur_loc + 1)) {
            Throw64BitLocationUnshareble(cur_loc + 1);
          }
          tail64_attrs_.insert(cur_loc + 1);
        }
        if (vertex_attrs_.contains(cur_loc)) {
          std::cerr << "Some variables share the same location. Such situation "
                       "is valid, but untested. Output may be invalid."
                    << std::endl;
          auto &cur_attr = vertex_attrs_[cur_loc];
          // TODO: Decide how to change the name
          cur_attr.max_component = std::max(
              cur_attr.max_component, initial_component + occupied_comps - 1);
          cur_attr.attribute.component =
              std::min(cur_attr.attribute.component, initial_component);
          uint32_t used_components =
              cur_attr.max_component - cur_attr.attribute.component + 1;
          // Note: according to the spec, only variables of same base types are
          // allowed to share the location.
          //
          // I'm not sure that extending format is the correct way to pass
          // multiple variable into the same component. I haven't found the
          // correct way in the spec. But it is definitely valid with
          // maintenance4
          cur_attr.attribute.format = TypeToFormat(type, used_components);

          // Note: if 64-bit vector occupies two locations, the max component
          // of the second location will be either occupied by its tail
          // (in this case there is no other variables in the 2nd location, and
          // we are shouldn't set its max_component) or by another variable
          // (max_component will be properly when handling that variable).
          // Thus we don't care about the second location.
        } else {
          vertex_attrs_.emplace(
              location + col,
              VertexAttributeMetadata{name + "_loc" +
                                          std::to_string(location + col),
                                      initial_component + occupied_comps,
                                      {location, initial_component, fmt}});
        }
      }
    }
  }
}
std::optional<spirv_cross::EntryPoint>
BindingsExtractor::GetEntryPointOrFirst(const std::string &name,
                                        spv::ExecutionModel model) {
  const auto points = compiler_.get_entry_points_and_stages();
  if (name.empty()) {
    for (const auto &point : points) {
      if (point.execution_model == model) {
        return point;
      }
    }
  }
  for (const auto &point : points) {
    if (point.name == name && point.execution_model == model) {
      return point;
    }
  }
  return std::nullopt;
}
vk::Format BindingsExtractor::TypeToFormat(const spirv_cross::SPIRType &type,
                                           uint32_t override_vecsize) {
  if (override_vecsize == 0) {
    override_vecsize = type.vecsize;
  }
#include "type2fmt.inc"
}
std::string &BindingsExtractor::CanonicalizeName(std::string &name) const {
  if (name.empty()) {
    return name;
  }
  if (std::isdigit(name[0])) {
    name = "m" + name;
  }
  for (char &chr : name) {
    if (!std::isalnum(chr)) {
      chr = '_';
    }
  }
  return name;
}
std::string BindingsExtractor::CanonicalizeName(const std::string &name) const {
  std::string copy(name);
  CanonicalizeName(copy);
  return copy;
}
uint32_t
BindingsExtractor::GetLocationFormatComponentCount(vk::Format fmt) const {
  // clang-format off
#include "fmtcomp.inc"
else return 0;
  // clang-format on
}
void BindingsExtractor::ExtractDescriptorSets() {
  auto resources = compiler_.get_shader_resources();
  // FIXME: The following resources are not extracted for now:
  // sample weight image;
  // block matching image; input attachment;
  // mutable

  // dynamic buffers and inline uniforms from the shader's perspective are
  // indistinguishable from normal buffers and uniforms

  // It turns out that OpTypeSampledImage in SPIR-V != 'sampled image' in
  // Vulkan.
  // Actual correspondence is summarized below:
  // SPIR-V Type                         Vulkan resource
  // OpTypeSampler                       sampler
  // OpTypeImage (Sampled=1)             sampled image
  // OpTypeSampledImage (...)            combined image sampler
  // OpTypeImage(Sampled=<1|2>,Dim=Buf)  <uniform|storage> texel buffer
  // See also:
  // https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-resources-descset
  ExtractDescriptorBindingsOfType(resources.separate_samplers,
                                  vk::DescriptorType::eSampler);
  ExtractDescriptorBindingsOfType(resources.sampled_images,
                                  vk::DescriptorType::eCombinedImageSampler);
  ExtractDescriptorBindingsOfType(resources.separate_images,
                                  vk::DescriptorType::eSampledImage);
  ExtractDescriptorBindingsOfType(resources.storage_images,
                                  vk::DescriptorType::eStorageImage);

  ExtractDescriptorBindingsOfType(resources.uniform_buffers,
                                  vk::DescriptorType::eUniformBuffer);
  ExtractDescriptorBindingsOfType(resources.storage_buffers,
                                  vk::DescriptorType::eStorageBuffer);
  ExtractDescriptorBindingsOfType( // TODO: KHR or NV?
      resources.acceleration_structures,
      vk::DescriptorType::eAccelerationStructureKHR);
  ExtractDescriptorBindingsOfType(resources.tensors,
                                  vk::DescriptorType::eTensorARM);
  for (const auto &[id, set] : descriptor_sets_) {
    std::cout << "DESCRIPTOR SET " << id << "\n";
    for (size_t i = 0; i < set.bindings.size(); ++i) {
      std::cout << "binding " << set.bindings[i].name << " (" << i << ") -> "
                << vk::to_string(set.bindings[i].binding.descriptorType)
                << std::endl;
    }
  }
}
void BindingsExtractor::ExtractDescriptorBindingsOfType(
    const spc::SmallVector<spc::Resource> &resources,
    vk::DescriptorType desc_type) {
  for (const auto &res : resources) {
    uint32_t set =
        compiler_.get_decoration(res.id, spv::DecorationDescriptorSet);
    uint32_t binding = compiler_.get_decoration(res.id, spv::DecorationBinding);
    auto &bindings = descriptor_sets_[set].bindings;
    auto name = compiler_.get_name(res.id);
    if (name.empty()) {
      name = compiler_.get_fallback_name(res.id);
    }
    // TODO: discard excess struct layer?
    // For images it is inapplicable
    auto type = ExtractType(res.base_type_id);
    bindings.emplace_back(
        vk::DescriptorSetLayoutBinding{
            binding, desc_type, 0 /* TODO: count */,
            vk::ShaderStageFlagBits::eAll /* TODO?: select stages */, nullptr},
        name, type);
  }
}
} // namespace shbind
