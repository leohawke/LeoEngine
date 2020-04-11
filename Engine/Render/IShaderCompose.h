#pragma once

#include "ShaderCore.h"

namespace platform::Render {

	class ShaderCompose {
	public:
		virtual ~ShaderCompose();

		static const leo::uint8 NumTypes = (leo::uint8)ShaderType::ComputeShader + 1;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;
	};

	class GraphicsBuffer;
}