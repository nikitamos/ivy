#pragma once

#include <algorithm>
#include <boost/pfr/core.hpp>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

#include <boost/pfr.hpp>

#include "spirv.hpp"
#include "spirv_common.hpp"
#include "spirv_cross.hpp"
#include <vulkan/vulkan.hpp>

#include "pipeline-spec.hpp"

namespace ivy {
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
public:
  PipelineProvider(size_t max_stages) : max_stage_count_(max_stages) {}

  const spirv_cross::SPIREntryPoint *
  TryGetNextEntryPoint(const spirv_cross::Compiler &compiler);
  size_t MaxStages() const { return max_stage_count_; }
  bool MayHaveUnhandledStages() const { return cur_stage < max_stage_count_; }

protected:
  struct StageInfo {
    const std::string &name;
    spv::ExecutionModel model;
  };
  virtual std::optional<StageInfo> GetNextStage() = 0;
  size_t cur_stage = 0;

private:
  const spirv_cross::SPIREntryPoint &
  FindEntryPoint(const spirv_cross::Compiler &compiler, StageInfo stage);
  size_t max_stage_count_;
};

// using TSpec = ComputePipelineSpec;
// #define VModelCount 1
// using VModels = ComputePipelineSpec::kModels;
template <typename TSpec, size_t VModelCount,
          std::array<spv::ExecutionModel, VModelCount> VModels>
class PipelineProviderFromSpec : public PipelineProvider {
public:
  static_assert(boost::pfr::tuple_size_v<TSpec> == VModelCount);
  PipelineProviderFromSpec() : PipelineProvider(VModelCount) {}
  static inline constexpr const size_t kModelCount = VModelCount;

protected:
  std::optional<StageInfo> GetNextStage() override {
    if (cur_stage >= VModelCount) {
      return std::nullopt;
    }
    return kGetterProviders.res[cur_stage++](spec_);
  }

private:
  using GetSpecImplFn =
      std::function<std::optional<StageInfo>(const TSpec &spec)>;
  static bool HasValue(const std::string &s) { return true; }
  static bool HasValue(const std::optional<std::string> &opt) {
    return opt.has_value();
  }
  static const std::string &GetValue(const std::string &s) { return s; }
  static const std::string &GetValue(const std::optional<std::string> &opt) {
    return opt.value();
  }
  template <size_t N>
  static std::optional<StageInfo> GetStageName(const TSpec &spec) {
    const auto &name = boost::pfr::get<N>(spec);
    if (HasValue(name)) {
      return StageInfo{GetValue(name), VModels[N]};
    }
    return std::nullopt;
  }

  template <size_t count> struct SpecGetterProvider {
    SpecGetterProvider() {
      SpecGetterProvider<count - 1> prev;
      std::ranges::copy(prev.res.begin(), prev.res.end(), res.begin());
      res[count - 1] = GetStageName<count - 1>;
    }
    std::array<GetSpecImplFn, count> res;
  };
  template <> struct SpecGetterProvider<1> {
    SpecGetterProvider() : res{GetStageName<0>} {}
    std::array<GetSpecImplFn, 1> res;
  };

  // TODO: make static?
  const SpecGetterProvider<VModelCount> kGetterProviders;
  TSpec spec_;
};

template <typename T>
using PipelineProviderForSpec =
    PipelineProviderFromSpec<T, T::kModels.size(), T::kModels>;

using GraphicsPipelineProvider = PipelineProviderForSpec<GraphicsPipelineSpec>;
using MeshPipelineProviderEXT = PipelineProviderForSpec<MeshPipelineSpecEXT>;
using MeshPipelineProviderNV = PipelineProviderForSpec<MeshPipelineSpecNV>;
using RaytracingPipelineProvider = PipelineProviderForSpec<ComputePipelineSpec>;
using ComputePipelineProvider = PipelineProviderForSpec<ComputePipelineSpec>;
} // namespace ivy