#pragma once
#include "spirv.hpp"

namespace shbind {
struct GraphicsPipelineSpec {
  std::string vertex;
  std::optional<std::string> tesselation_control;
  std::optional<std::string> tesselation_evaluation;
  std::optional<std::string> geometry;
  std::string fragment;
  static constexpr const std::array<spv::ExecutionModel, 5> kModels = {
      spv::ExecutionModelVertex, spv::ExecutionModelTessellationControl,
      spv::ExecutionModelTessellationEvaluation, spv::ExecutionModelGeometry,
      spv::ExecutionModelFragment};
};
struct MeshPipelineSpecNV {
  std::optional<std::string> task;
  std::string mesh;
  std::string fragment;
  static constexpr const std::array<spv::ExecutionModel, 3> kModels = {
      spv::ExecutionModelTaskNV, spv::ExecutionModelMeshNV,
      spv::ExecutionModelFragment};
};
struct MeshPipelineSpecEXT {
  std::string task;
  std::string mesh;
  std::string fragment;
  static constexpr const std::array<spv::ExecutionModel, 3> kModels = {
      spv::ExecutionModelTaskEXT, spv::ExecutionModelMeshEXT,
      spv::ExecutionModelFragment};
};

struct ComputePipelineSpec {
  std::string kernel;
  static constexpr const std::array<spv::ExecutionModel, 1> kModels = {
      spv::ExecutionModelGLCompute};
};

// TODO: ray tracing may require to be split into 2 groups
struct RaytracingPipelineSpec {
  std::string raygen;
  std::vector<std::string> miss;
  std::vector<std::string> hit;
  std::vector<std::string> callable;

  std::optional<std::string> intersection;
  std::optional<std::string> any_hit;
  std::optional<std::string> closest_hit;

  static constexpr const std::array<spv::ExecutionModel, 7> kModels = {
      spv::ExecutionModelRayGenerationKHR, spv::ExecutionModelMissKHR,
      spv::ExecutionModelCallableKHR,      spv::ExecutionModelIntersectionKHR,
      spv::ExecutionModelAnyHitKHR,        spv::ExecutionModelClosestHitKHR,
  };
};
} // namespace shbind