# ivy - utility to generate Vulkan shader setup code
According to the Vulkan spec, the application must ensure that the
pipeline layout provided in the `Vk***PipelineCreateInfo` corresponds
to the layout declared in the SPIR-V shader code. Hence the information
required to create the pipeline can be inferred from the shader.

That's exactly what ShaderBinder does: it parses SPIR-V file,
extracts pipeline layout (i.e. descriptor bindings
and push constants) from there and generates C++ code
that allows to create and manage all descriptor sets.

# NOTE: the project is in very early alpha stage and is unusable yet.
