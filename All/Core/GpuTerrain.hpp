#pragma once
#include "..\d3dx11.hpp"
namespace leo
{
	class Camera;

	class GpuTerrain
	{
	public:
		ALIGN16
		struct CBChangeOnLod
		{
			//旋转矩阵用来减少重复性
			XMMATRIX octaveMat6;
			XMMATRIX octaveMat7;
			//有LOD层次决定 1.0 2.0 4.0
			float  wsChunkSize = 4.0;
		};
		ALIGN16
		struct CBChangerEveryFrame
		{
			XMMATRIX World;
		};
		using Vertex = XMFLOAT3;
	public:
		~GpuTerrain();
	public:
		void Init(ID3D11Device* device);

		void Draw(ID3D11DeviceContext* context, const Camera& camera);
	private:
		//好吧,我现在能大概认为生成密度空间成功...
		void BuildDensity(ID3D11DeviceContext* context, CXMMATRIX world);
		void GenVertices(ID3D11DeviceContext* context, CXMMATRIX world);
		void DrawVertices(ID3D11DeviceContext* context, CXMMATRIX viewproj);
	private:
		ID3D11VertexShader* m_bdvs = nullptr;
		ID3D11GeometryShader* m_bdgs = nullptr;
		ID3D11PixelShader* m_bdps = nullptr;
		ID3D11InputLayout* m_inputlayout = nullptr;

		ID3D11ShaderResourceView* Vol[8];
		ID3D11SamplerState* m_bdpsss[4];

		ID3D11Buffer* m_bdvb = nullptr;
		ID3D11Buffer* m_bdib = nullptr;
		ID3D11Buffer* m_bdvscb = nullptr;
		ID3D11Buffer* m_bdpscb = nullptr;

		ID3D11Texture3D* m_bdrtvbuffer = nullptr;
		ID3D11RenderTargetView* m_bdrtv = nullptr;

		ID3D11Texture3D* mDensityTex = nullptr;
		ID3D11ShaderResourceView* mDensityRes = nullptr;

		ID3D11Buffer* m_gvvb = nullptr;//1?
		ID3D11Buffer* m_gvsovb = nullptr;
		ID3D11Buffer* m_gvsodebugvb = nullptr;

		ID3D11VertexShader* m_gvvs = nullptr;
		ID3D11GeometryShader* m_gvgs = nullptr;

		ID3D11Query* StreamOutQuery = nullptr;

		ID3D11VertexShader* m_dvvs = nullptr;
		ID3D11PixelShader* m_dvps = nullptr;

		ID3D11RasterizerState* m_dvrs = nullptr;
		ID3D11DepthStencilState* m_dvdss = nullptr;
	};
}