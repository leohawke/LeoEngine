#include "Water.hpp"
#include "..\exception.hpp"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
namespace leo
{
	const static std::size_t strides[] = { sizeof(Water::Vertex) };
	const static std::size_t offsets[] = { 0 };

	const static D3D11_INPUT_ELEMENT_DESC inputdescs[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	void Water::Init(ID3D11Device* device, short m, short n, float dx, float dt, float speed, float damping)
	{
		this->m = m;
		this->n = n;
		auto vertexcount = m*n;
		auto trianglecount = (m - 1)*(n - 1) * 2;

		stepspatial = dx; steptime = dt;

		auto d = damping*dt + 2.f;
		auto e = (speed*speed)*(dt*dt) / (dx*dx);
		mk1 = (damping*dt - 2.f) / d;
		mk2 = (4.f - 8.f*e) / d;
		mk3 = (2.f*e) / d;

		m_Prev = std::make_unique<XMFLOAT3[]>(vertexcount);
		m_Curr = std::make_unique<XMFLOAT3[]>(vertexcount);

		float halfWidth = (n - 1)*dx*0.5f;
		float halfDepth = (m - 1)*dx*0.5f;
		for (auto i = 0; i != m; ++i)
		{
			auto z = halfDepth - i*dx;//0.5=>-0.5
			for (auto j = 0; j != n; ++j)
			{
				auto x = -halfWidth + j*dx;//-0.5=>0.5
				m_Prev[i*n + j] = m_Curr[i*n + j] = XMFLOAT3(x, 0.0f, z);
			}
		}

		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;
		Desc.StructureByteStride = 0;
		Desc.ByteWidth = sizeof(Vertex)*vertexcount;

		D3D11_SUBRESOURCE_DATA subSourceDesc;
		subSourceDesc.pSysMem = m_Curr.get();

		dxcall(device->CreateBuffer(&Desc, &subSourceDesc, &m_vertexbuffer));

		m_indexnums = 3 * trianglecount;
		auto indices = std::make_unique<std::uint16_t[]>(m_indexnums);
		int k = 0;
		for (auto i = 0; i != m - 1; ++i)
		{
			for (auto j = 0; j != n - 1; ++j)
			{
				indices[k] = i*n + j;
				indices[k + 1] = i*n + j + 1;
				indices[k + 2] = (i + 1)*n + j;

				indices[k + 3] = (i + 1)*n + j;
				indices[k + 4] = i*n + j + 1;
				indices[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}

		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		Desc.ByteWidth = sizeof(short)*m_indexnums;
		subSourceDesc.pSysMem = indices.get();
		dxcall(device->CreateBuffer(&Desc, &subSourceDesc, &m_indexbuffer));


		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.ByteWidth = sizeof(CBChangeEveryFrame);
		Desc.CPUAccessFlags = 0;
		dxcall(device->CreateBuffer(&Desc, nullptr, &m_CBChangeEveryFramebuffer));

		Desc.ByteWidth = sizeof(CBColor);
		CBColor cbcolor;
		cbcolor.color = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.6f);
		subSourceDesc.pSysMem = &cbcolor;
		dxcall(device->CreateBuffer(&Desc, &subSourceDesc, &m_CBColor));
		ShaderMgr shadermgr;

		m_vertexshader = shadermgr.CreateVertexShader(L"WaterVS.cso", nullptr, inputdescs, 1, &m_inputlayout);
		m_pixelsshader = shadermgr.CreatePixelShader(L"WaterPS.cso");
		
	}

	void Water::Update(ID3D11DeviceContext* context, float dt)
	{
		static float t = 0.f;
		t += dt;
		if (t >= steptime)
		{
			for (auto i = 1; i != m-1; ++i)
			{
				for (auto j = 1; j != n - 1; ++j)
				{
					m_Prev[i*n + j].y =
						mk1*m_Prev[i*n + j].y +
						mk2*m_Curr[i*n + j].y +
						mk3*(m_Curr[(i + 1)*n + j].y +
						m_Curr[(i - 1)*n + j].y +
						m_Curr[i*n + j + 1].y +
						m_Curr[i*n + j - 1].y);
				}
			}
			std::swap(m_Curr, m_Prev);
			context->UpdateSubresource(m_vertexbuffer, 0, nullptr, m_Curr.get(), 0, 0);
			t = 0.f;
		}
	}

	void Water::Distrub(short i,short j,float magnitude)
	{
		float halfMag = 0.5f*magnitude;
		m_Curr[i*n + j].y += magnitude;
		m_Curr[i*n + j + 1].y += halfMag;
		m_Curr[i*n + j - 1].y += halfMag;
		m_Curr[(i + 1)*n + j].y += halfMag;
		m_Curr[(i - 1)*n + j].y += halfMag;
	}

	void Water::Render(ID3D11DeviceContext* context, CXMMATRIX view, CXMMATRIX proj)
	{
		auto world = XMLoadFloat4x4(&m_world);

		auto worldviewproj = world*view*proj;

		CBChangeEveryFrame cb;
		cb.worldviewproj = XMMatrixTranspose(worldviewproj);

		context->UpdateSubresource(m_CBChangeEveryFramebuffer, 0, nullptr, &cb, 0, 0);

		context->IASetIndexBuffer(m_indexbuffer, DXGI_FORMAT_R16_UINT, 0);
		context->IASetInputLayout(m_inputlayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetVertexBuffers(0, 1, &m_vertexbuffer, strides, offsets);

		context->VSSetShader(m_vertexshader, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &m_CBChangeEveryFramebuffer);

		context->PSSetShader(m_pixelsshader, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &m_CBColor);

		const float rgba[4] {};

		RenderStates rss;
		static auto TransparentBS = rss.GetBlendState(L"TransparentBS");
		context->OMSetBlendState(TransparentBS,rgba , 0xFFFFFFFF);
		context->DrawIndexed(m_indexnums, 0, 0);

		context->OMSetBlendState(nullptr, rgba, 0xFFFFFFFF);
	}
}