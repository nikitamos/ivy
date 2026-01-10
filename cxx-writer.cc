// #define VULKAN_HPP_USE_REFLECT 1
#include "cxx-writer.hpp"
#include "host-types.hpp"
#include "metadata.hpp"
#include <string>

namespace shbind {
template <typename T> struct PrintMember {
  PrintMember(T mem, std::ostream &out) {
    if constexpr (std::is_enum_v<T>) {
      out << "{" << static_cast<std::underlying_type_t<T>>(mem) << "},\n";
    } else {
      out << "{" << mem << "},\n";
    }
  }
};
template <typename F> struct PrintMember<vk::Flags<F>> {
  PrintMember(vk::Flags<F> mem, std::ostream &out) {
    out << "{" << static_cast<vk::Flags<F>::MaskType>(mem) << "},\n";
  }
};
template <typename... T>
inline void CxxWriter::DeclareInitializerList(std::tuple<const T &...> params,
                                              std::ostream &out) {
  out << "{ ";
  std::apply([&out](const T &...args) { ((PrintMember(args, out)), ...); },
             params);
  out << "}";
}

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
  Indent(out) << "struct [[gnu::packed]] " << type->name << " {\n";
  uint32_t cur_size = 0;
  static const std::string kPadPrefix = "_pad";

  IncreaseIndent();
  HostArray pad_array("uint8_t", 0);

  for (size_t i = 0; i < type->members.size(); ++i) {
    const auto &mem = type->members[i];
    if (cur_size != mem.offset) {
      pad_array.dimensions[0] = mem.offset - cur_size;
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
  // Indent(out) << "#pragma pack(pop)\n";
}
void CxxWriter::WritePrelude(std::ostream &out) {
  Indent(out) << "// NOTE: This file is auto-generated\n\n";
  Indent(out) << "#include <cstdint>\n";
  Indent(out) << "#include <vertex-attribs.hpp>\n";
  // Indent(out) << "#pragma pack(push, 1)\n";
}

void CxxWriter::VarDeclareArray(HostArray *host_type, std::string name,
                                std::ostream &out) {
  Indent(out) << host_type->name << ' ' << name;
  for (auto i = host_type->dimensions.begin(); i != host_type->dimensions.end();
       ++i) {
    out << '[' << *i << ']';
  }
  out << ";\n";
}
void CxxWriter::VarDeclareStruct(HostStruct *host_type, std::string name,
                                 std::ostream &out) {
  VarDeclareHostType(host_type, name, out);
}
void CxxWriter::VarDeclareHostType(HostType *host_type, std::string name,
                                   std::ostream &out) {
  Indent(out) << host_type->name << ' ' << name << ";\n";
}
void CxxWriter::WriteVertexAttributeInterface(
    const std::vector<VertexAttributeMetadata> &attrs, std::ostream &out) {
  static const std::string kStructName = "VertexShaderInputAttribute";
  Indent(out) << "struct " << kStructName
              << " : public "
                 "shbind::api::VertexShaderInputBase<"
              << attrs.size() << "> {\n";
  IncreaseIndent();
  for (auto &[name, max_component, attr] : attrs) {
    Indent(out) << "static constexpr inline const shbind::api::VertexAttribute "
                << name << "{ .location = " << attr.location
                << ", .component = " << attr.component
                << ", .format = vk::Format(" << (uint64_t)attr.format
                << ")};\n";
  }
  Indent(out) << "constexpr void SelfInit();\n";
  Indent(out) << "constexpr " << kStructName << "() { SelfInit(); }\n";
  DecreaseIndent();
  Indent(out) << "};" << std::endl;
}

void CxxWriter::WriteDescriptorSetStruct(const DescriptorSetMetadata &set,
                                         std::ostream &out, uint32_t idx) {
  std::string struct_name = "DescriptorSet" + std::to_string(idx) + "Layout";
  out << "struct " << struct_name << " {\n";
  IncreaseIndent();
  for (const auto &binding : set.bindings) {
    // Note: set name is unknown!
    Indent(out) << "vk::DescriptorSetLayoutBinding " << binding.name << "\n";
    // clang-format off
    Indent(out) << "{ "
                <<    ".binding = " << binding.binding.binding
                <<  ", .descriptorType = vk::DescriptorType(" << (uint64_t)binding.binding.descriptorType 
                << "), .descriptorCount = " << binding.binding.descriptorCount
                <<  ", .stageFlags = vk::ShaderStageFlags(" << static_cast<vk::ShaderStageFlags::MaskType>(binding.binding.stageFlags)
                << ") };\n";
    // clang-format on
  }
  // Conversion operator for some reason
  Indent(out)
      << "operator vk::ArrayProxy<vk::DescriptorSetLayoutBinding>() const \n";
  IncreaseIndent();
  Indent(out) << "{ return {" << set.bindings.size()
              << ", (vk::DescriptorSetLayoutBinding *)this}; }\n";
  DecreaseIndent();

  // DescriptorSetLayoutCreateInfo getter
  Indent(out) << "vk::DescriptorSetLayoutCreateInfo "
                 "CreateInfo(vk::DescriptorSetLayoutCreateFlags flags = {}) "
                 "{\n";
  IncreaseIndent();
  Indent(out) << "return {.flags = flags, .bindingCount = "
              << set.bindings.size()
              << ", .pBindings = "
                 "(vk::DescriptorSetLayoutBinding *)this};\n";
  DecreaseIndent();
  Indent(out) << "}\n";

  DecreaseIndent();
  Indent(out) << "};\n";
  Indent(out) << "static_assert(offsetof(" << struct_name << ", "
              << set.bindings[0].name << ") == 0);\n\n";
}
} // namespace shbind
