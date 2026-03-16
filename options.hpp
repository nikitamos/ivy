#pragma once
#include <string>

namespace ivy {
struct GenerationOptions {
  std::string module_name;
  bool Validate() const;
};
} // namespace ivy