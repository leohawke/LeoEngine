// CopyRight 2014. LeoHawke. All wrongs reserved.

#ifndef Core_Mesh_Hpp
#define Core_Mesh_Hpp


#include <vector>

#include "..\d3dx11.hpp"
#include "..\COM.hpp"
#include "Vertex.hpp"
#include "..\LightHelper.h"
#include "..\LightBuffer.h"
#include "..\debug.hpp"
#include "CoreObject.hpp"
#include "..\IndePlatform\BaseMacro.h"
#include "..\IndePlatform\leoint.hpp"


namespace leo
{
	class Camera;

	class Mesh : public SQTObject
	{
	public:
		struct subset
		{
			Material m_mat;
			ID3D11ShaderResourceView* m_texdiff;
			ID3D11ShaderResourceView* m_texnormalmap;
			win::uint m_indexoffset;
			win::uint	m_indexcount;
		};

		using vertex_type = leo::Vertex::NormalMap;
		friend class SkeletonModel;
	private:
		std::vector<subset> m_subsets;

		ID3D11Buffer* m_vertexbuff{ nullptr };
		ID3D11Buffer* m_indexbuff{ nullptr };
	public:
		//;临时变动,改用shader dynamice link,需重写shaderMgr
		Mesh(const SQT& sqt = {})
			: SQTObject(sqt)
		{
		}
	public:
		bool Load(const std::wstring& file,ID3D11Device* device);
		virtual ~Mesh()
		{
			
			leo::win::ReleaseCOM(m_vertexbuff);
			//leo::win::ReleaseCOM(m_vertexshader);
			leo::win::ReleaseCOM(m_indexbuff);
			//leo::win::ReleaseCOM(m_pixelshader);
		}
		//变动
		void Render(ID3D11DeviceContext* context,const leo::Camera&);
		void CastShadow(ID3D11DeviceContext* context);
	protected:

		DefGetter(const _NOEXCEPT, ID3D11Buffer*, VertexBuffer, m_vertexbuff);
		DefGetter(const _NOEXCEPT, ID3D11Buffer*, IndexBuffer, m_indexbuff);

		
	};
}
#endif