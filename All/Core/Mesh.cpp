#include "..\IndePlatform\platform.h"


#include "..\IndePlatform\memory.hpp"
#include "..\string.hpp"
#include "..\file.hpp"
#include "MeshLoad.hpp"
#include "..\exception.hpp"
#include "Mesh.hpp"
#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "FileSearch.h"
#include "Camera.hpp"
#include "EffectShadowMap.hpp"
#include "EffectGBuffer.hpp"

namespace leo
{
	void ReadSubSets(std::unique_ptr<win::File> & fin, std::uint64_t& offset, std::vector<Mesh::subset>&subsets, ID3D11Device* device)
	{
		auto size = subsets.size();
		std::vector<MeshFile::MeshMaterial> materials(size);
		fin->Read(&materials[0], sizeof(MeshFile::MeshMaterial)*size, offset);
		offset += sizeof(MeshFile::MeshMaterial)*size;
		std::vector<MeshFile::MeshSubSet> filesubsets(size);
		fin->Read(&filesubsets[0], sizeof(MeshFile::MeshSubSet)*size, offset);
		offset += sizeof(MeshFile::MeshSubSet)*size;
		leo::TextureMgr texmgr;
		for (UINT i = 0; i < size; ++i)
		{
			subsets[i].m_texdiff = texmgr.LoadTextureSRV(FileSearch::Search(materials[i].diffusefile));
			subsets[i].m_texnormalmap = texmgr.LoadTextureSRV(FileSearch::Search(materials[i].normalmapfile));
			subsets[i].m_indexcount = filesubsets[i].indexcount;
			subsets[i].m_indexoffset = filesubsets[i].indexoffset;
			BZero(subsets[i].m_mat);
			std::memcpy(&subsets[i].m_mat.ambient, &materials[i].ambient, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].m_mat.diffuse, &materials[i].diffuse, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].m_mat.specular, &materials[i].specular, sizeof(XMFLOAT3));
			subsets[i].m_mat.specular.w = materials[i].specPow;
			//subsets[i].m_mat.reflect = materials[i].reflect;
		}
	}

	void ReadVertices(std::unique_ptr<win::File> & fin, std::uint64_t& offset, std::vector<Mesh::vertex_type>& vertices)
	{
		auto size = vertices.size();
		std::vector<MeshFile::MeshVertex> _vertices(size);
		auto buffersize = _vertices.size() * sizeof(MeshFile::MeshVertex);
		fin->Read(&_vertices[0], buffersize, offset);

		for (std::size_t i = 0; i != size; ++i)
		{
			vertices[i] = _vertices[i].operator leo::Vertex::NormalMap();
		}

		offset += buffersize;
	}

	void ReadIndexs(std::unique_ptr<win::File> & fin, std::uint64_t& offset, std::vector<std::uint32_t>& indices)
	{
		auto size = indices.size() * sizeof(std::uint32_t);
		fin->Read(&indices[0], size, offset);
		offset += size;
	}


	bool Mesh::Load(const std::wstring& filename, ID3D11Device* device)
	{
		if (filename.size() == 0)
			return false;
		auto fin = win::File::OpenNoThrow(filename,win::File::TO_READ);
		try{
			leo::MeshFile::MeshFileHeader l3d_header;
			std::uint64_t fileoffset = 0;
			fin->Read(&l3d_header, sizeof(l3d_header), fileoffset);
			fileoffset += sizeof(l3d_header);
			m_subsets.resize(l3d_header.numsubset);

			ReadSubSets(fin, fileoffset, m_subsets, device);
			std::vector<vertex_type> vertices(l3d_header.numvertice);
			ReadVertices(fin, fileoffset, vertices);
			std::vector<std::uint32_t> indices(l3d_header.numindex);
			ReadIndexs(fin, fileoffset, indices);

			D3D11_BUFFER_DESC Desc;
			Desc.Usage = D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;
			Desc.StructureByteStride = 0;
			Desc.ByteWidth =static_cast<win::uint> (sizeof(vertex_type)*vertices.size());

			D3D11_SUBRESOURCE_DATA resDesc;
			resDesc.pSysMem = &vertices[0];
			dxcall(device->CreateBuffer(&Desc, &resDesc, &m_vertexbuff));

			Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			Desc.ByteWidth = static_cast<win::uint> (sizeof(std::uint32_t)*indices.size());
			resDesc.pSysMem = &indices[0];
			dxcall(device->CreateBuffer(&Desc, &resDesc, &m_indexbuff));
		}
		catch (leo::win::dx_exception & e)
		{
			OutputDebugStringA(e.what());
			return false;
		}
		catch (leo::win::win32_exception & e)
		{
			OutputDebugStringA(e.what());
			return false;
		}
		return true;
	}

	void Mesh::Render(ID3D11DeviceContext* context,const Camera& camera)
	{
		context->IASetIndexBuffer(m_indexbuff, DXGI_FORMAT_R32_UINT, 0);
		static UINT strides[] = { sizeof(vertex_type) };
		static UINT offsets[] = { 0 };
		ID3D11Buffer* vbs[] = { m_vertexbuff };
		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto layout = ShaderMgr().CreateInputLayout(InputLayoutDesc::NormalMap);
		context->IASetInputLayout(layout);

		auto & mEffect = EffectGBuffer::GetInstance();


		auto world =SQT::operator std::array<__m128, 4U>();
		

		mEffect->WorldViewMatrix(Multiply(world,load(camera.View())));
		mEffect->WorldViewProjMatrix(Multiply(world,load(camera.ViewProj())));

		__m128 det;
		world[3] = leo::set(0.f, 0.f, 0.f, 1.f);
		world = Transpose(Inverse(det, world));
		mEffect->WorldInvTransposeViewMatrix(Multiply(world, load(camera.View())));


		mEffect->Apply(context);

		for (auto it = m_subsets.cbegin(); it != m_subsets.cend();++it)
		{
			//mEffect->Mat(it->m_mat, context);
			mEffect->DiffuseSRV(it->m_texdiff);
			//mEffect->NormalMapSRV(it->m_texnormalmap, context);
			context->DrawIndexed(it->m_indexcount, it->m_indexoffset, 0);
		}

		if (EffectConfig::GetInstance()->NormalLine())
		{
			auto & mLineEffect = EffectNormalLine::GetInstance();
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			mLineEffect->World(world);
			mLineEffect->ViewProj(camera.ViewProj());
			mLineEffect->Apply(context);
			for (auto it = m_subsets.cbegin(); it != m_subsets.cend(); ++it)
			{
				context->DrawIndexed(it->m_indexcount, it->m_indexoffset, 0);
			}
		}
		
	}

	void Mesh::CastShadow(ID3D11DeviceContext* context) {
		context->IASetIndexBuffer(m_indexbuff, DXGI_FORMAT_R32_UINT, 0);
		static UINT strides[] = { sizeof(vertex_type) };
		static UINT offsets[] = { 0 };
		ID3D11Buffer* vbs[] = { m_vertexbuff };
		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::NormalMap));

		EffectShadowMap::GetInstance()->WorldMatrix(SQT::operator leo::float4x4(), context);
		for (auto it = m_subsets.cbegin(); it != m_subsets.cend(); ++it)
		{
			context->DrawIndexed(it->m_indexcount, it->m_indexoffset, 0);
		}
	}
}
