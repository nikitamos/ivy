#pragma once
#include <string>

namespace shbind {
struct GenerationOptions {
  std::string module_name;
  bool Validate() const;
};
} // namespace shbind