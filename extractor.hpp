#pragma once
#include "host-types.hpp"
#include "options.hpp"
#include "spirv_common.hpp"
#include "spirv_cross.hpp"
#include <memory>
#include <unordered_map>

namespace shbind {
struct CxxModule {};

class BindingsExtractor {
public:
  BindingsExtractor(const spirv_cross::Compiler &compiler,
                    HostTypeFactory &factory,
                    GenerationOptions &&opts = GenerationOptions())
      : compiler_(std::move(compiler)), opts_(std::move(opts)),
        type_factory_(factory) {}
  CxxModule ExtractBindings();

  BindingsExtractor(const BindingsExtractor &) = delete;
  BindingsExtractor(BindingsExtractor &&) = delete;

  BindingsExtractor &operator=(const BindingsExtractor &) = delete;
  BindingsExtractor &operator=(BindingsExtractor &&) = delete;

private:
  void ExtractPushConstants();
  void ExtractSpecializationConstants();
  std::shared_ptr<HostType> ExtractType(spirv_cross::TypeID id);
  void ExtractAllTypes();

  std::unordered_map<uint32_t, std::shared_ptr<HostType>> types_;
  const spirv_cross::Compiler &compiler_;
  GenerationOptions opts_;
  HostTypeFactory &type_factory_;
};
} // namespace shbind