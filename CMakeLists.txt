set(SPIRV_CROSS_CLI OFF)
set(SPIRV_CROSS_ENABLE_C_API OFF)
set(SPIRV_CROSS_ENABLE_HLSL OFF)
set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_ENABLE_REFLECT OFF)

add_subdirectory(SPIRV-Cross)

add_executable(shader-binder main.cc extractor.cc host-types.cc)
target_include_directories(shader-binder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(shader-binder PROPERTIES
                      CXX_STANDARD 20)
target_link_libraries(shader-binder spirv-cross-cpp)