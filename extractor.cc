#include "extractor.hpp"
#include "host-types.hpp"
#include "spirv_common.hpp"
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
    // ExtractType(push_const.type_id);
    ExtractType(push_const.base_type_id);
  }
}
void BindingsExtractor::ExtractSpecializationConstants() {}
std::shared_ptr<HostType> BindingsExtractor::ExtractType(spc::TypeID id) {
  const auto &type = compiler_.get_type(id);
  auto name = compiler_.get_name(id);
  std::cout << name << ":: ID:" << id << " alias:" << type.type_alias
            << " width:" << type.width << " base:" << type.basetype
            << " mem-count:" << type.member_types.size()
            << " pointer:" << type.pointer << "array-dim:" << type.array.size()
            << " vecXcols:" << type.vecsize << "x" << type.columns << std::endl;
  auto res = type_factory_.CreateType(type, compiler_);
  return res;
}
void BindingsExtractor::ExtractAllTypes() {}

shbind::CxxModule BindingsExtractor::ExtractBindings() {
  ExtractPushConstants();
  return CxxModule();
}
} // namespace shbind

struct vec3 {
  float x, y;
  char tt;
  float z;
};

struct Test {
  bool b;
  vec3 d;
};