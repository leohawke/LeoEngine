#pragma once

namespace platform_ex::Windows::D3D12 {
	class ShaderType
	{

	};

#define EXPORTED_SHADER_TYPE(ShaderClass) \
public:\
	using ShaderMetaType = ShaderType;\
	static ShaderMetaType StaticType;

	class BuiltInRayTracingShader
	{
	public:
		using ShaderMetaType = ShaderType;
	};

	class DefaultCHS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultCHS);
	};

	class DefaultMS :public BuiltInRayTracingShader
	{
		EXPORTED_SHADER_TYPE(DefaultMS);
	};
}
