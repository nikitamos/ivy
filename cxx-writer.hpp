#pragma once
#include "metadata.hpp"
#include "options.hpp"
#include "writer.hpp"
#include <ostream>
#include <tuple>
#include <type_traits>

namespace shbind {
template <typename T>
concept Enum = requires(T t) {
  { std::is_enum_v<T>() };
};
class CxxWriter : public IWriter {
public:
  CxxWriter(const GenerationOptions &opts, std::string &&indentation = "  ")
      : IWriter(std::move(indentation)), opts_(opts) {}
  virtual ~CxxWriter() {}
  void WritePrelude(std::ostream &out) override;
  void EndWriting(std::ostream &out) override;

  void WriteVertexAttributeInterface(
      const std::vector<VertexAttributeMetadata> &attrs,
      std::ostream &out) override;

  void DeclareHostType(HostType *type, std::ostream &out) override;
  void DeclareHostArray(HostArray *type, std::ostream &out) override;
  void DeclareHostStruct(HostStruct *type, std::ostream &out) override;

  void FwdDeclareArray(HostArray *host_type, std::ostream &out) override;
  void FwdDeclareStruct(HostStruct *host_type, std::ostream &out) override;
  void FwdDeclareHostType(HostType *host_type, std::ostream &out) override;

  void VarDeclareArray(HostArray *host_type, std::string name, std::ostream &out) override;
  void VarDeclareStruct(HostStruct *host_type, std::string name, std::ostream &out) override;
  void VarDeclareHostType(HostType *host_type, std::string name, std::ostream &out) override;
  void WriteDescriptorSetStruct(const DescriptorSetMetadata &bindings,
                                std::ostream &out, uint32_t idx) override;
  virtual void WritePushConstantRanges(std::span<vk::PushConstantRange> ranges,
                                       std::ostream &out) override;
  /// NOTE: Don't use this methods on structs that have sType and pNext
  template <typename... T>
  void DeclareInitializerList(std::tuple<const T &...> params,
                              std::ostream &out);

private:
  const GenerationOptions &opts_;
};
} // namespace shbind