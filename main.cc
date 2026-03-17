#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "argparse.hpp"

#include "cxx-writer.hpp"
#include "extractor.hpp"
#include "host-types.hpp"

#include "options.hpp"
#include "pipeline-provider.hpp"
#include "pipeline-spec.hpp"
#include "spirv.hpp"
#include "spirv_cross.hpp"

namespace ivy {
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

enum class PipelineType {
  kGraphics,
  kMeshNV,
  kMeshEXT,
  kCompute,
  kRaytracingNV,
  kRaytracingKHR
};

PipelineType GetExecutionModelPipelineType(spv::ExecutionModel em) {
  switch (em) {
  case spv::ExecutionModelVertex:
  case spv::ExecutionModelTessellationControl:
  case spv::ExecutionModelTessellationEvaluation:
  case spv::ExecutionModelGeometry:
  case spv::ExecutionModelFragment:
    return PipelineType::kGraphics;
  case spv::ExecutionModelGLCompute:
  case spv::ExecutionModelKernel:
    return PipelineType::kCompute;
  case spv::ExecutionModelTaskNV:
  case spv::ExecutionModelMeshNV:
    return PipelineType::kMeshNV;
  case spv::ExecutionModelRayGenerationKHR:
  case spv::ExecutionModelIntersectionKHR:
  case spv::ExecutionModelAnyHitKHR:
  case spv::ExecutionModelClosestHitKHR:
  case spv::ExecutionModelMissKHR:
  case spv::ExecutionModelCallableKHR:
    return PipelineType::kRaytracingKHR; // TODO: or NV?
  case spv::ExecutionModelTaskEXT:
  case spv::ExecutionModelMeshEXT:
    return PipelineType::kMeshEXT;
  default:
  case spv::ExecutionModelMax:
    throw std::runtime_error("Invalid execution model");
  }
}

std::vector<std::unique_ptr<PipelineProvider>>
TryGuessPipeline(spirv_cross::Compiler &compiler) {
  auto entry_points = compiler.get_entry_points_and_stages();
  // Check that all execution models belong to the same pipeline type.
  // TODO: if there are multiple pipeline types detected, try construct each
  // one.
  if (entry_points.empty()) {
    return {};
  }
  auto model = GetExecutionModelPipelineType(entry_points[0].execution_model);
  bool all_same = std::ranges::all_of(
      entry_points | std::views::transform([](const auto &ep) {
        return GetExecutionModelPipelineType(ep.execution_model);
      }),
      [model](PipelineType pt) { return pt == model; });
  if (!all_same) {
    throw std::runtime_error("inconsistent shader module");
  }
  // std::unique_ptr<typename Tp>

  return {};
}

} // namespace ivy

int main(int argc, char **argv) {
  std::string input_path, output_path;
  ivy::GenerationOptions opts;
  ivy::GraphicsPipelineSpec graphics, mesh_ext, mesh_nv, compute;

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

  auto src = ivy::ReadShader(input_path);
  spirv_cross::Compiler core((uint32_t *)src.data(), src.size() / 4);
  if (src.size() % 4 != 0) {
    throw std::runtime_error("SPIR-V size is invalid");
  }
  ivy::HostTypeFactory factory;
  ivy::CxxWriter writer(opts);
  ivy::BindingsExtractor extractor(core, factory);

  ivy::GraphicsPipelineProvider pr(ivy::GraphicsPipelineSpec{});
  extractor.ExtractBindings(pr);
  extractor.WriteToStream(*out_stream, writer);
}