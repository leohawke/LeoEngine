#include "IRayTracingShader.h"
#include "BuiltInRayTracingShader.h"

using namespace platform::Render;

RayTracingShader* Shader::BuiltInRayTracingShader::GetRayTracingShader()
{
	return pRayTracingShader.get();
}

void Shader::BuiltInRayTracingShader::SetRayTracingShader(RayTracingShader* pShader)
{
	pRayTracingShader = shared_raw_robject(pShader);
}
