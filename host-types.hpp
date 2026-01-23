#pragma once
#include "spirv_common.hpp"
#include "spirv_cross.hpp"
#include "spirv_cross_containers.hpp"
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace shbind {
class IWriter;
struct HostType {
  HostType(std::string name) : name(name) {}
  std::string name;

protected:
  friend IWriter;
  virtual void AcceptFwdDeclare(IWriter &writer, std::ostream &out);
  virtual void AcceptDeclare(IWriter &writer, std::ostream &out);
  virtual void AcceptVarDeclare(IWriter &writer, const std::string &name,
                                std::ostream &out);
};

class HostTypeFactory {
public:
  HostTypeFactory() {};
  std::shared_ptr<HostType> GetType(const spirv_cross::SPIRType &type,
                                    const spirv_cross::Compiler &compiler);
  std::vector<std::shared_ptr<HostType>> GetAllKnownTypes() const;

protected:
  virtual std::shared_ptr<HostType>
  CreatePrimitiveType(spirv_cross::SPIRType::BaseType base, int vec_dim,
                      int cols, spirv_cross::SmallVector<uint32_t> array = {});
  virtual std::shared_ptr<HostType>
  CreateStruct(const spirv_cross::SPIRType &type,
               const spirv_cross::Compiler &compiler);

  virtual std::shared_ptr<HostType>
  CreateFallbackType(const spirv_cross::SPIRType &type,
                     const spirv_cross::Compiler &compiler);
  virtual std::string GetTypeName(spirv_cross::SPIRType &type);
  const std::optional<std::string>
  GetPrimitiveTypeName(spirv_cross::SPIRType::BaseType base);

  std::map<spirv_cross::ID, std::shared_ptr<HostType>> type_map_;
  std::vector<spirv_cross::ID> order_;
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

struct HostArray : public HostType {
  [[deprecated]]
  HostArray(std::string name, uint32_t len)
      : HostType{name}, dimensions{len} {}
  [[deprecated]]
  HostArray(std::shared_ptr<HostType> subtype, uint32_t len)
      : HostArray(subtype, spirv_cross::SmallVector<uint32_t>{len}) {}

  HostArray(std::shared_ptr<HostType> element,
            spirv_cross::SmallVector<uint32_t> &&dim)
      : HostType(element->name), dimensions(std::move(dim)) {}
  spirv_cross::SmallVector<uint32_t> dimensions;

protected:
  virtual void AcceptFwdDeclare(IWriter &writer, std::ostream &out) override;
  virtual void AcceptDeclare(IWriter &writer, std::ostream &out) override;
  virtual void AcceptVarDeclare(IWriter &writer, const std::string &name,
                                std::ostream &out) override;
};

struct HostStruct : public HostType {
  HostStruct(std::string name, std::vector<StructMem> &&mems)
      : HostType{name}, members(std::move(mems)) {}

  std::vector<StructMem> members;

protected:
  virtual void AcceptFwdDeclare(IWriter &writer, std::ostream &out) override;
  virtual void AcceptDeclare(IWriter &writer, std::ostream &out) override;
  virtual void AcceptVarDeclare(IWriter &writer, const std::string &name,
                                std::ostream &out) override;
};
} // namespace shbind
