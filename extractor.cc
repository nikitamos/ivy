#include "extractor.hpp"
#include "host-types.hpp"
#include "spirv_common.hpp"
#include "writer.hpp"

#include <iostream>
#include <memory>

namespace shbind {
namespace spc = spirv_cross;
void BindingsExtractor::ExtractPushConstants() {
  auto resources = compiler_.get_shader_resources();
  auto constants = resources.push_constant_buffers;
  for (const auto push_const : constants) {
    std::cout << "dealing with push constant `" << push_const.name
              << "` (base id: " << push_const.base_type_id << " )" << std::endl;
    ExtractType(push_const.base_type_id);
  }
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
  // End file
  writer.EndWriting(out);
}
} // namespace shbind