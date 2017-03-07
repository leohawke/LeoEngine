/*! \file Engine\Render\InputLayout.hpp
\ingroup Engine
\brief 输入描述相关封装。
*/
#ifndef LE_RENDER_D3D12_InputLayout_hpp
#define LE_RENDER_D3D12_InputLayout_hpp 1

#include "d3d12_dxgi.h"
#include "../InputLayout.hpp"

#include <vector>

namespace platform_ex::Windows::D3D12 {
	class InputLayout : public platform::Render::InputLayout {
	public:
		InputLayout();

		const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputDesc();

		void Active();
	private:
		 std::vector<D3D12_INPUT_ELEMENT_DESC> vertex_elems;
		 std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs;
		 D3D12_INDEX_BUFFER_VIEW ibv;
	};
}

#endif