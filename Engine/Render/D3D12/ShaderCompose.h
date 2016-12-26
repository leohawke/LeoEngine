/*! \file Engine\Render\D3D12\ShaderCompose.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_ShaderCompose_h
#define LE_RENDER_D3D12_ShaderCompose_h 1

#include "../Effect/Effect.hpp"

namespace platform_ex::Windows::D3D12 {
	class ShaderCompose:public platform::Render::ShaderCompose
	{
	public:
		 void Bind() override;
		 void Unbind() override;
	};
}

#endif