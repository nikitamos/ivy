#pragma once
#include "writer.hpp"
#include <ostream>

namespace shbind {
class CxxWriter : public IWriter {
public:
  CxxWriter(std::string &&indentation = "  ") : IWriter(std::move(indentation)) {}
  virtual ~CxxWriter() {}
  void WritePrelude(std::ostream &out) override;
  void EndWriting(std::ostream &out) override;

  void WriteVertexAttributeInterface(
      const std::vector<std::pair<std::string, api::VertexAttribute>> &attrs,
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
};
} // namespace shbind