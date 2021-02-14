#include "IRayTracingShader.h"
#include "BuiltInRayTracingShader.h"

using namespace platform::Render;

RayTracingShader* Shader::BuiltInRayTracingShader::GetRayTracingShader()
{
	return GetHardwareShader<RayTracingShader>();
}

