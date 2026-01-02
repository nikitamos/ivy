#include "extractor.hpp"
#include "api/vertex-attribs.hpp"
#include "host-types.hpp"
#include "spirv.hpp"
#include "spirv_common.hpp"
#include "vulkan/vulkan.hpp"
#include "writer.hpp"

#include <cctype>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

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
  writer.WriteVertexAttributeInterface(vertex_attrs_, out);
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
      uint32_t component =
          compiler_.get_decoration(var, spv::DecorationComponent);

      // TODO: move this logic to the TypeToFormat function
      auto type = compiler_.get_type_from_variable(var);
      if (type.member_types.size() > 1) {
        throw std::runtime_error(
            "Frankenstein structs in shader inputs are unsupported.");
      } else if (type.member_types.size() == 1) {
        type = compiler_.get_type(type.member_types[0]);
      }

      vk::Format fmt = TypeToFormat(type);
      std::cout << "format: " << (uint64_t)fmt << std::endl;
      std::string name = CanonicalizeName(compiler_.get_name(var));
      if (name.empty()) {
        name =
            "inp" + std::to_string(location) + '_' + std::to_string(component);
      }
      vertex_attrs_.emplace_back(
          name, api::VertexAttribute{location, component, fmt});
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
vk::Format BindingsExtractor::TypeToFormat(const spirv_cross::SPIRType &type) {
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
} // namespace shbind