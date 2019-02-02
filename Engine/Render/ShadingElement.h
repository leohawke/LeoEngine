#ifndef LE_RENDER_ShadingElement_h
#define LE_RENDER_ShadingElement_h 1

#include <LBase/any.h>
#include "DataStructures.h"
#include "InputLayout.hpp"

namespace LeoEngine::Render {
	using InputLayout = platform::Render::InputLayout;
	namespace Vertex = platform::Render::Vertex;

	enum ShadingElementDataType {
		SED_Unknown,

		SED_Mesh,
	};

	class IShadingElement {
	public:
		IShadingElement() = default;

		virtual ~IShadingElement();
	};

	class ShadingElement {
	private:
		static uint32 ElementId;
	public:
		ShadingElementDataType type = SED_Unknown;

		uint32 id;

		leo::any custom_data;
	public:
		//!TOOD �ع�Mesh,ֻ�ṩ������[��InputLayout?]
		//!TODO �ṩMesh����һ��ĳ���,������ȡGeometryInfo
		//!TODO �ع�InputLayout,ֻ����InputLayout�Ĵ�������ά��������Ϣ,������Ϣ
		struct GeometryInfo {
			/*bonesRemapInfo*/

			InputLayout::TopologyType primitive_type = InputLayout::TopologyType::TriangleList;
			uint8 vertex_format = Vertex::InputLayoutFormat::Empty;
			uint8 num_streams = 0;//Convert From vertex_format 

			uint32 start_vertex_location = 0;
			uint32 start_index_location = 0;

			uint32 num_verteices = 0;
			uint32 num_indices = 0;

			//ptr Class InputLayout
			uintptr_t input_layout = 0;
		};
	public:
		//Multi-Type ShadingElement
		virtual ~ShadingElement();

		ShadingElement(ShadingElementDataType type);
	public:
		//ShadingElement LinkList
	};
}

#endif
