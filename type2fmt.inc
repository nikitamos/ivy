switch (type.basetype) {
case spc::SPIRType::SByte:
switch (type.vecsize) {case 1: return vk::Format::eR8Uint;
case 2: return vk::Format::eR8G8Uint;
case 3: return vk::Format::eR8G8B8Uint;
case 4: return vk::Format::eR8G8B8A8Uint;
}
case spc::SPIRType::UByte:
switch (type.vecsize) {case 1: return vk::Format::eR8Uint;
case 2: return vk::Format::eR8G8Uint;
case 3: return vk::Format::eR8G8B8Uint;
case 4: return vk::Format::eR8G8B8A8Uint;
}
case spc::SPIRType::Short:
switch (type.vecsize) {case 1: return vk::Format::eR16Sint;
case 2: return vk::Format::eR16G16Sint;
case 3: return vk::Format::eR16G16B16Sint;
case 4: return vk::Format::eR16G16B16A16Sint;
}
case spc::SPIRType::UShort:
switch (type.vecsize) {case 1: return vk::Format::eR16Uint;
case 2: return vk::Format::eR16G16Uint;
case 3: return vk::Format::eR16G16B16Uint;
case 4: return vk::Format::eR16G16B16A16Uint;
}
case spc::SPIRType::Int:
switch (type.vecsize) {case 1: return vk::Format::eR32Sint;
case 2: return vk::Format::eR32G32Sint;
case 3: return vk::Format::eR32G32B32Sint;
case 4: return vk::Format::eR32G32B32A32Sint;
}
case spc::SPIRType::UInt:
switch (type.vecsize) {case 1: return vk::Format::eR32Uint;
case 2: return vk::Format::eR32G32Uint;
case 3: return vk::Format::eR32G32B32Uint;
case 4: return vk::Format::eR32G32B32A32Uint;
}
case spc::SPIRType::Int64:
switch (type.vecsize) {case 1: return vk::Format::eR64Sint;
case 2: return vk::Format::eR64G64Sint;
case 3: return vk::Format::eR64G64B64Sint;
case 4: return vk::Format::eR64G64B64A64Sint;
}
case spc::SPIRType::UInt64:
switch (type.vecsize) {case 1: return vk::Format::eR64Uint;
case 2: return vk::Format::eR64G64Uint;
case 3: return vk::Format::eR64G64B64Uint;
case 4: return vk::Format::eR64G64B64A64Uint;
}
case spc::SPIRType::Float:
switch (type.vecsize) {case 1: return vk::Format::eR32Sfloat;
case 2: return vk::Format::eR32G32Sfloat;
case 3: return vk::Format::eR32G32B32Sfloat;
case 4: return vk::Format::eR32G32B32A32Sfloat;
}
case spc::SPIRType::Double:
switch (type.vecsize) {case 1: return vk::Format::eR64Sfloat;
case 2: return vk::Format::eR64G64Sfloat;
case 3: return vk::Format::eR64G64B64Sfloat;
case 4: return vk::Format::eR64G64B64A64Sfloat;
}

default: throw std::runtime_error("don't know format for base type " + std::to_string(type.basetype));

}
