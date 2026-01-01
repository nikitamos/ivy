#pragma once

#include "spirv_common.hpp"
#include "spirv_cross.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace shbind {

struct HostType {
  std::string name;
  static const std::shared_ptr<HostType> kInt8;
  static const std::shared_ptr<HostType> kUInt8;
  static const std::shared_ptr<HostType> kInt16;
  static const std::shared_ptr<HostType> kUInt16;
  static const std::shared_ptr<HostType> kInt32;
  static const std::shared_ptr<HostType> kUInt32;
  static const std::shared_ptr<HostType> kInt64;
  static const std::shared_ptr<HostType> kUInt64;
  static const std::shared_ptr<HostType> kFloat;
  static const std::shared_ptr<HostType> kDouble;
  static inline std::shared_ptr<HostType> CharType() {
    if constexpr (std::is_signed_v<char>) {
      return kInt8;
    }
    return kUInt8;
  }
};

class HostTypeFactory {
public:
  HostTypeFactory() {};
  virtual std::shared_ptr<HostType>
  CreatePrimitiveType(spirv_cross::SPIRType::BaseType base, int vec_dim,
                      int cols);
  virtual std::shared_ptr<HostType>
  CreateStruct(const spirv_cross::SPIRType &type,
               const spirv_cross::Compiler &compiler);

  virtual std::shared_ptr<HostType>
  CreateFallbackType(const spirv_cross::SPIRType &type,
                     const spirv_cross::Compiler &compiler);

  std::shared_ptr<HostType> CreateType(const spirv_cross::SPIRType &type,
                                       const spirv_cross::Compiler &compiler);

protected:
  virtual std::string GetTypeName(spirv_cross::SPIRType &type) {
    return "<NOT-IMPL>";
  }
  const std::optional<std::string>
  GetPrimitiveTypeName(spirv_cross::SPIRType::BaseType base);
};

struct StructMem {
  StructMem(std::shared_ptr<HostType> type, const std::string &name,
            uint32_t size, uint32_t offset)
      : type(type), name(name), size(size), offset(offset) {}
  std::shared_ptr<HostType> type;
  std::string name;
  uint32_t size;
  uint32_t offset;
};

struct HostArray : HostType {
  HostArray(std::string name, uint32_t len) : HostType{name}, array_len(len) {}
  uint32_t array_len;
};

struct HostStruct : HostType {
  HostStruct(std::string name, std::vector<StructMem> &&mems)
      : HostType{name}, members_(std::move(mems)) {
    std::cout << "CREATING STRUCT " << name << " {\n";
    for (auto m : members_) {
      std::cout << "\t" << m.type->name << " " << m.name << "; // (" << m.offset
                << ">>" << m.size << ")\n";
    }
    std::cout << "};" << std::endl;
  }
  std::vector<StructMem> members_;
};
} // namespace shbind
