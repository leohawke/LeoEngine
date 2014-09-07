#include "..\d3dx11.hpp"
#include "..\leomath.hpp"
#include "..\COM.hpp"
#include <memory>
#include <cstddef>
namespace leo
{
	class Water
	{
	public:
		using Vertex = XMFLOAT3;

		struct CBChangeEveryFrame
		{
			XMMATRIX worldviewproj;
		};

		struct CBColor
		{
			XMFLOAT4 color;
		};

		explicit Water(const XMFLOAT4X4& world)
			:m_world(world)
		{
		}

		void Render(ID3D11DeviceContext* context, CXMMATRIX view, CXMMATRIX proj);

		~Water()
		{
			leo::win::ReleaseCOM(m_vertexbuffer);
			leo::win::ReleaseCOM(m_indexbuffer);
			leo::win::ReleaseCOM(m_CBChangeEveryFramebuffer);
		}

		void Init(ID3D11Device* device, short m, short n, float dx, float dt, float speed, float damping);
		void Update(ID3D11DeviceContext* context,float dt);
		void Distrub(short i, short j, float magnitude);
	private:
		ID3D11Buffer* m_vertexbuffer = nullptr;
		ID3D11Buffer* m_indexbuffer = nullptr;
		ID3D11Buffer * m_CBChangeEveryFramebuffer = nullptr;
		ID3D11Buffer * m_CBColor = nullptr;
		ID3D11InputLayout* m_inputlayout = nullptr;
		std::size_t m_indexnums = 0;
		ID3D11VertexShader* m_vertexshader = nullptr;
		ID3D11PixelShader* m_pixelsshader = nullptr;

		XMFLOAT4X4 m_world;

		//
		short m, n;
		float mk1, mk2, mk3;
		float steptime, stepspatial;
		std::unique_ptr<XMFLOAT3[]> m_Prev = nullptr;
		std::unique_ptr<XMFLOAT3[]> m_Curr = nullptr;
	};
}