#include "extractor.hpp"
#include "host-types.hpp"
#include "spirv_common.hpp"
#include <iostream>
#include <memory>

namespace shbind {
namespace internal {}
void BindingsExtractor::ExtractPushConstants() {
  auto resources = compiler_.get_shader_resources();
  auto constants = resources.push_constant_buffers;
  for (const auto push_const : constants) {
    std::cout << "dealing with push constant `" << push_const.name
              << "` (base id: " << push_const.base_type_id << " )" << std::endl;
    ExtractType(push_const.type_id);
    ExtractType(push_const.base_type_id);
  }
}
void BindingsExtractor::ExtractSpecializationConstants() {}
std::shared_ptr<HostType>
BindingsExtractor::ExtractType(spirv_cross::TypeID id) {
  const auto &type = compiler_.get_type(id);
  std::cout << "ID:" << id << " alias:" << type.type_alias
            << " width:" << type.width << " base:" << type.basetype
            << " mem-count:" << type.member_types.size()
            << " pointer:" << type.pointer << std::endl;
  if (type.basetype == spirv_cross::SPIRType::Struct) {
    return std::make_shared<HostStruct>(ExtractStruct(type));
  }
  return nullptr;
}
void BindingsExtractor::ExtractAllTypes() {}

shbind::CxxModule BindingsExtractor::ExtractBindings() {
  ExtractPushConstants();
  return CxxModule();
}
HostStruct BindingsExtractor::ExtractStruct(const spirv_cross::SPIRType &type) {
  assert(type.member_type_index_redirection.size() == 0 &&
         "member redirection is unsupported yet");
  for (int i = 0; i< type.member_types.size();++i) {
    // compiler_.type_struct_member_offset(const SPIRType &type, uint32_t index)
    const auto& mem_type = compiler_.get_type(type.member_types[i]);
    std::cout << compiler_.get_member_name(type.self, i) << ": size=" << compiler_.get_declared_struct_member_size(type, i) << std::endl;
  }
  return HostStruct();
}
} // namespace shbind


struct vec3 {
  float x, y, z;
};

struct Test {
  bool b;
  vec3 d;
};