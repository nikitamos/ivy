#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vulkan/vulkan.hpp>

#include "spirv.hpp"
#include "spirv_cross.hpp"
#include "spirv_cross_error_handling.hpp"

namespace shbind {
class EntryPointNotFoundException : public std::runtime_error {
public:
  EntryPointNotFoundException(const std::string &name,
                              spv::ExecutionModel model)
      : std::runtime_error("can not find entry point `" + name + "`"),
        name(name), model(model) {}
  const std::string name;
  const spv::ExecutionModel model;
};

class PipelineProvider {
  virtual std::pair<const std::string &, spv::ExecutionModel> NextEntryPoint();
  const spirv_cross::SPIREntryPoint &
  FindNextEntryPoint(const spirv_cross::Compiler &compiler,
                     const std::string &name, spv::ExecutionModel model) {
    if (name.empty()) {
      // Try to find an entry point with the required model.
      // If the number of entry points found is != 1, then throw
    }
    try {
      const auto &ep = compiler.get_entry_point(name, model);
      return ep;
    } catch (spirv_cross::CompilerError &ce) {
      throw EntryPointNotFoundException(name, model);
    }
  }

protected:
  struct StageInfo {
    const std::string &name;
    spv::ExecutionModel model;
    bool last;
  };
  virtual StageInfo GetNextStage() = 0;
};

struct VertexPipelineSpec {
  std::string vertex;
  std::optional<std::string> tesselation_control;
  std::optional<std::string> tesselation_evaluation;
  std::optional<std::string> geometry;
  std::string fragment;
};

class VertexPipelineProvider {
public:
  VertexPipelineProvider(VertexPipelineSpec &&spec) : spec_(std::move(spec)) {}

private:
  VertexPipelineSpec spec_;
};

struct MeshPipelineSpecNV {
  std::optional<std::string> task;
  std::string mesh;
  std::string fragment;
};
struct MeshPipelineSpecEXT {
  std::string task;
  std::string mesh;
};

template <typename U, typename... T>
std::tuple<std::add_lvalue_reference<T>...> Unpack3(U& u) {
    return u;
}

void A(MeshPipelineSpecEXT ext) {
//   std::tuple aaa = ext;
}

class ComputePipelineSpec {
  std::string kernel;
};

class RaytracingPipelineSpec {};
} // namespace shbind