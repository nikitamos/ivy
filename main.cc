#include <cctype>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "argparse.hpp"

#include "cxx-writer.hpp"
#include "extractor.hpp"
#include "host-types.hpp"

#include "options.hpp"

namespace shbind {
std::vector<char> ReadShader(const std::string_view path) {
  int a = 3;
  std::ifstream shader_file(path.data(),
                            std::ios_base::ate | std::ios_base::binary);
  if (!shader_file.is_open()) {
    throw std::runtime_error("failed to open shader file");
  }
  std::vector<char> shader_src(shader_file.tellg());
  shader_file.seekg(0, std::ios::beg);
  shader_file.read(shader_src.data(), shader_src.size());
  return shader_src;
}
bool GenerationOptions::Validate() const {
  if (!module_name.empty()) {
    if (!(std::isalpha(module_name[0]) || module_name[0] == '_')) {
      // throw
      return false;
    }
    for (size_t i = 1; i < module_name.size(); ++i) {
      if (!std::isalnum(module_name[i])) {
        // throw
        return false;
      }
    }
  }
  return true;
}
} // namespace shbind

int main(int argc, char **argv) {
  std::string input_path, output_path;
  shbind::GenerationOptions opts;
  argparse::ArgumentParser argparser{"shader-binder", "0.1"};
  argparser.add_argument("input")
      .help("input SPIR-V file")
      .store_into(input_path);
  argparser.add_argument("-o").help("output file").store_into(output_path);
  argparser.add_argument("--module,-m")
      .help("module (namespace) where to place the bindings, separated by ::")
      .store_into(opts.module_name);
  argparser.parse_args(argc, argv);

  if (!opts.Validate()) {
    std::cout << "Error: invalid option passed\n";
    return 1;
  }

  std::ostream *out_stream = &std::cout;
  std::ofstream out_file;
  if (!output_path.empty()) {
    out_file.open(output_path);
    if (out_file.fail()) {
      std::cout << "Failed to open file " << output_path << std::endl;
      return 1;
    }
    out_stream = &out_file;
  }

  auto src = shbind::ReadShader(input_path);
  spirv_cross::Compiler core((uint32_t *)src.data(), src.size() / 4);
  if (src.size() % 4 != 0) {
    throw std::runtime_error("SPIR-V size is invalid");
  }
  shbind::HostTypeFactory factory;
  shbind::CxxWriter writer(opts);
  shbind::BindingsExtractor extractor(core, factory);

  shbind::GraphicsPipelineProvider pr({});
  extractor.ExtractBindings(pr);
  extractor.WriteToStream(*out_stream, writer);
}