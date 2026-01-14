#include "pipeline-provider.hpp"
#include "pipeline-spec.hpp"

namespace shbind {
const spirv_cross::SPIREntryPoint &
PipelineProvider::FindEntryPoint(const spirv_cross::Compiler &compiler,
                                 StageInfo stage) {
  if (stage.name.empty()) {
    const auto points = compiler.get_entry_points_and_stages();
    auto model_pred = [model = stage.model](const spirv_cross::EntryPoint &ep) {
      return ep.execution_model == model;
    };
    auto iter = std::find_if(points.begin(), points.end(), model_pred);
    if (iter == points.end()) {
      throw EntryPointNotFoundException("<unknown>", stage.model);
    }
    if (std::find_if(iter + 1, points.end(), model_pred) != points.end()) {
      throw std::runtime_error("Multiple entry points of execution model " +
                               std::to_string(stage.model) +
                               " detected. Specify the entry point name");
    }
    return compiler.get_entry_point(iter->name, iter->execution_model);
  }
  try {
    const auto &ep = compiler.get_entry_point(stage.name, stage.model);
    return ep;
  } catch (spirv_cross::CompilerError &ce) {
    throw EntryPointNotFoundException(stage.name, stage.model);
  }
}
const spirv_cross::SPIREntryPoint *
PipelineProvider::TryGetNextEntryPoint(const spirv_cross::Compiler &compiler) {
  auto stage = GetNextStage();
  if (stage.has_value()) {
    return &FindEntryPoint(compiler, stage.value());
  } else if (!MayHaveUnhandledStages()) {
    return nullptr;
  }
  return TryGetNextEntryPoint(compiler);
}
} // namespace shbind