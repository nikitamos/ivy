#pragma once
#include "host-types.hpp"
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "api/vertex-attribs.hpp"
#include "metadata.hpp"

namespace shbind {
class IWriter {
public:
  IWriter(std::string &&indent="  ") : indent_(std::move(indent)) {}
  void FwdDeclareType(std::shared_ptr<HostType> type, std::ostream &out) {
    type->AcceptFwdDeclare(*this, out);
  }
  void DeclareType(std::shared_ptr<HostType> type, std::ostream &out) {
    type->AcceptDeclare(*this, out);
  }
  void DeclareVar(std::shared_ptr<HostType> type, const std::string &name, std::ostream &out) {
    type->AcceptVarDeclare(*this, name, out);
  }
  virtual void WriteVertexAttributeInterface(
      const std::vector<VertexAttributeMetadata> &attrs, std::ostream &out) = 0;
  virtual ~IWriter() {}

  virtual void WritePrelude(std::ostream &out) = 0;
  virtual void EndWriting(std::ostream &out) = 0;

  inline void IncreaseIndent() { ++indent_level_; }
  inline void DecreaseIndent() { --indent_level_; }
  inline std::ostream& Indent(std::ostream &out) {
    for (int i = 0; i < indent_level_; ++i)
      out << indent_;
    return out;
  }

protected:
  friend HostType;
  friend HostArray;
  friend HostStruct;

  virtual void FwdDeclareHostType(HostType *host_type, std::ostream &out) = 0;
  virtual void FwdDeclareArray(HostArray *host_type, std::ostream &out) = 0;
  virtual void FwdDeclareStruct(HostStruct *host_type, std::ostream &out) = 0;

  virtual void DeclareHostType(HostType *type, std::ostream &out) = 0;
  virtual void DeclareHostArray(HostArray *type, std::ostream &out) = 0;
  virtual void DeclareHostStruct(HostStruct *type, std::ostream &out) = 0;

  virtual void VarDeclareHostType(HostType *host_type, std::string name, std::ostream &out) = 0;
  virtual void VarDeclareArray(HostArray *host_type, std::string name, std::ostream &out) = 0;
  virtual void VarDeclareStruct(HostStruct *host_type, std::string name, std::ostream &out) = 0;

private:
  int indent_level_ = 0;
  std::string indent_;
};
} // namespace shbind