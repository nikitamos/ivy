#include "cxx-writer.hpp"
#include "extractor.hpp"
#include "host-types.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace shbind {
std::vector<char> ReadShader(const std::string &path) {
  int a = 3;
  std::ifstream shader_file(path, std::ios_base::ate | std::ios_base::binary);
  if (!shader_file.is_open()) {
    throw std::runtime_error("failed to open shader file");
  }
  std::vector<char> shader_src(shader_file.tellg());
  shader_file.seekg(0, std::ios::beg);
  shader_file.read(shader_src.data(), shader_src.size());
  return shader_src;
}
} // namespace shbind

int main(int argc, char **argv) {
  if (argc < 1) {
    throw std::runtime_error("missing file name!");
  }
  auto src = shbind::ReadShader(argv[1]);
  spirv_cross::Compiler core((uint32_t *)src.data(), src.size() / 4);
  if (src.size() % 4 != 0) {
    throw std::runtime_error("SPIR-V size is invalid");
  }
  shbind::HostTypeFactory factory;
  shbind::CxxWriter writer;
  shbind::BindingsExtractor extractor(std::move(core), factory);

  extractor.ExtractBindings();
  extractor.WriteToStream(std::cout, writer);
}