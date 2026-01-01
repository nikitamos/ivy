#include "cxx-writer.hpp"
#include "host-types.hpp"
#include <string>

namespace shbind {
void CxxWriter::FwdDeclareHostType(HostType *host_type, std::ostream &out) {
  // nop
}
void CxxWriter::FwdDeclareStruct(HostStruct *host_type, std::ostream &out) {
  out << "struct " << host_type->name << ";\n";
}
void CxxWriter::FwdDeclareArray(HostArray *host_type, std::ostream &out) {
  // nop
}
void CxxWriter::DeclareHostStruct(HostStruct *type, std::ostream &out) {
  Indent(out) << "struct " << type->name << " {\n";
  uint32_t cur_size = 0;
  static const std::string kPadPrefix = "_pad";

  IncreaseIndent();
  HostArray pad_array("uint8_t", 0);

  for (size_t i = 0; i < type->members.size(); ++i) {
    const auto &mem = type->members[i];
    if (cur_size != mem.offset) {
      pad_array.array_len = mem.offset - cur_size;
      VarDeclareArray(&pad_array, kPadPrefix + std::to_string(i), out);
    }
    Indent(out) << "// oft=" << mem.offset << " size=" << mem.size << "\n";
    DeclareVar(mem.type, mem.name, out);
    cur_size += mem.size;
  }
//   if (cur_size != type->size) {
  
//   }
  DecreaseIndent();
  Indent(out) << "};\n";
}
void CxxWriter::DeclareHostArray(HostArray *type, std::ostream &out) {
  // nop
}
void CxxWriter::DeclareHostType(HostType *type, std::ostream &out) {
  // nop
}
void CxxWriter::EndWriting(std::ostream &out) {
  Indent(out) << "#pragma pack(pop)\n";
}
void CxxWriter::WritePrelude(std::ostream &out) {
  Indent(out) << "#include <cstdint>\n";
  Indent(out) << "#pragma pack(push, 1)\n";
}

void CxxWriter::VarDeclareArray(HostArray *host_type, std::string name,
                                std::ostream &out) {
  Indent(out) << host_type->name << ' ' << name << '[' << host_type->array_len
              << ']' << ";\n";
}
void CxxWriter::VarDeclareStruct(HostStruct *host_type, std::string name,
                                 std::ostream &out) {
  VarDeclareHostType(host_type, name, out);
}
void CxxWriter::VarDeclareHostType(HostType *host_type, std::string name,
                                   std::ostream &out) {
  Indent(out) << host_type->name << ' ' << name << ";\n";
}
} // namespace shbind