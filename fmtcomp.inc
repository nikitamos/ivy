if (
( vk::Format::eR8Uint < fmt && fmt < vk::Format::eR8Sint ) ||
( vk::Format::eR16Uint < fmt && fmt < vk::Format::eR16Sfloat ) ||
( vk::Format::eR32Uint < fmt && fmt < vk::Format::eR32Sfloat ) ||
( vk::Format::eR64Uint < fmt && fmt < vk::Format::eR64Sfloat ) ||
false)
  return 1;
if (
( vk::Format::eR8G8Uint < fmt && fmt < vk::Format::eR8G8Sint ) ||
( vk::Format::eR16G16Uint < fmt && fmt < vk::Format::eR16G16Sfloat ) ||
( vk::Format::eR32G32Uint < fmt && fmt < vk::Format::eR32G32Sfloat ) ||
( vk::Format::eR64G64Uint < fmt && fmt < vk::Format::eR64G64Sfloat ) ||
false)
  return 2;
if (
( vk::Format::eR8G8B8Uint < fmt && fmt < vk::Format::eR8G8B8Sint ) ||
( vk::Format::eR16G16B16Uint < fmt && fmt < vk::Format::eR16G16B16Sfloat ) ||
( vk::Format::eR32G32B32Uint < fmt && fmt < vk::Format::eR32G32B32Sfloat ) ||
( vk::Format::eR64G64B64Uint < fmt && fmt < vk::Format::eR64G64B64Sfloat ) ||
false)
  return 3;
if (
( vk::Format::eR8G8B8A8Uint < fmt && fmt < vk::Format::eR8G8B8A8Sint ) ||
( vk::Format::eR16G16B16A16Uint < fmt && fmt < vk::Format::eR16G16B16A16Sfloat ) ||
( vk::Format::eR32G32B32A32Uint < fmt && fmt < vk::Format::eR32G32B32A32Sfloat ) ||
( vk::Format::eR64G64B64A64Uint < fmt && fmt < vk::Format::eR64G64B64A64Sfloat ) ||
false)
  return 4;
