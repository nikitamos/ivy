#include "host-types.hpp"
#include "spirv_common.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace shbind {
namespace spc = spirv_cross;
static const std::string kInt8 = "int8_t";
static const std::string kUInt8 = "uint8_t";
static const std::string kInt16 = "int16_t";
static const std::string kUInt16 = "uint16_t";
static const std::string kInt32 = "int32_t";
static const std::string kUInt32 = "uint32_t";
static const std::string kInt64 = "int64_t";
static const std::string kUInt64 = "int64_t";
static const std::string kFloat = "float";
static const std::string kDouble = "double";

std::shared_ptr<HostType>
HostTypeFactory::CreatePrimitiveType(spc::SPIRType::BaseType base, int vec_dim,
                                     int cols) {
  if (vec_dim == 1 && cols == 1) {
    auto name = GetPrimitiveTypeName(base);
    if (name.has_value()) {
      return std::make_shared<HostType>(name.value());
    }
  }
  return nullptr;
}

const std::optional<std::string>
HostTypeFactory::GetPrimitiveTypeName(spirv_cross::SPIRType::BaseType base) {
  switch (base) {
  case spc::SPIRType::SByte:
    return kInt8;
  case spc::SPIRType::Char:
    if constexpr (std::is_signed_v<char>) {
      return kInt8;
    } else {
      return kUInt8;
    }
  case spc::SPIRType::UByte:
    return kUInt8;
  case spc::SPIRType::Short:
    return kInt32;
  case spc::SPIRType::UShort:
    return kUInt16;
  case spc::SPIRType::Int:
    return kInt32;
  case spc::SPIRType::UInt:
    return kUInt32;
  case spc::SPIRType::Int64:
    return kInt64;
  case spc::SPIRType::UInt64:
    return kUInt64;
  case spc::SPIRType::Float:
    return kFloat;
  case spc::SPIRType::Double:
    return kDouble;
  default:
    std::cerr << "WARNING(GetPrimitiveTypeName): non-primitive type: " << base
              << std::endl;
    return std::nullopt;
  }
}

std::shared_ptr<HostType>
HostTypeFactory::CreateStruct(const spirv_cross::SPIRType &type,
                              const spirv_cross::Compiler &compiler) {
  assert(type.member_type_index_redirection.size() == 0 &&
         "member redirection is unsupported yet");
  std::vector<StructMem> members;
  members.reserve(type.member_types.size());

  for (int i = 0; i < type.member_types.size(); ++i) {
    // compiler_.type_struct_member_offset(const SPIRType &type, uint32_t index)
    const auto &mem_type = compiler.get_type(type.member_types[i]);
    auto mem_size = compiler.get_declared_struct_member_size(type, i);
    auto mem_oft = compiler.type_struct_member_offset(type, i);
    auto mem_name = compiler.get_member_name(type.self, i);
    if (mem_name.empty()) {
      std::cerr << "Warning: member " << i << "of type " << type.self
                << "has empty name\n";
      mem_name = "member" + std::to_string(i);
    }
    std::cout << compiler.get_member_name(type.self, i) << ": size=" << mem_size
              << " oft=" << mem_oft << std::endl;
    members.emplace_back(
        CreateType(compiler.get_type(type.member_types[i]), compiler), mem_name,
        mem_size, mem_oft);
  }
  std::string name = compiler.get_name(type.self);
  if (name.empty()) {
    std::cerr << "Warning: type " << type.self << " has empty name."
              << std::endl;
    name = "SpvType" + std::to_string(type.self);
  }
  return std::make_shared<HostStruct>(name, std::move(members));
}

std::shared_ptr<HostType>
HostTypeFactory::CreateType(const spc::SPIRType &type,
                            const spc::Compiler &compiler) {
  // TODO: Try to read the type from the cache
  auto host_type =
      CreatePrimitiveType(type.basetype, type.vecsize, type.columns);
  if (host_type == nullptr && type.basetype == spc::SPIRType::Struct) {
    host_type = CreateStruct(type, compiler);
  }
  if (host_type == nullptr) {
    std::cerr << "Warning: creating fallback type for the type-id " << type.self
              << std::endl;
    host_type = CreateFallbackType(type, compiler);
  }
  assert(host_type != nullptr && "failed to create host type");
  // TODO: Add host_type to the cache
  return host_type;
}
std::shared_ptr<HostType>
HostTypeFactory::CreateFallbackType(const spc::SPIRType &type,
                                    const spc::Compiler &compiler) {
  return std::make_shared<HostArray>(
      GetPrimitiveTypeName(spc::SPIRType::UByte).value(), type.width);
}
} // namespace shbind