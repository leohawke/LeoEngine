#pragma once

#include "Shader.h"

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
class RayTracingShader;

inline namespace Shader
{
	bool IsRayTracingShader(platform::Render::ShaderType type);


	class BuiltInRayTracingShader :public RenderShader
	{
	public:
		RayTracingShader* GetRayTracingShader();
		void SetRayTracingShader(RayTracingShader* pShader);
	private:
		std::shared_ptr<RayTracingShader> pRayTracingShader;
	};
}
PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END