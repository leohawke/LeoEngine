#include "..\IndePlatform\platform.h"
#include "..\d3dx11.hpp"

#include "Skeleton.hpp"

#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"

#include "MeshLoad.hpp"
namespace leo{
	static void ReadSubSets(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::SubSet>&subsets, ID3D11Device* device)
	{
		auto size = subsets.size();
		std::vector<MeshFile::MeshMaterial> materials(size);
		min.Read(&materials[0], sizeof(MeshFile::MeshMaterial)*size, offset);
		offset += sizeof(MeshFile::MeshMaterial)*size;
		std::vector<MeshFile::MeshSubSet> filesubsets(size);
		min.Read(&filesubsets[0], sizeof(MeshFile::MeshSubSet)*size, offset);
		offset += sizeof(MeshFile::MeshSubSet)*size;
		leo::TextureMgr texmgr;
		for (UINT i = 0; i < size; ++i)
		{
			subsets[i].mTexSRV = texmgr.LoadTextureSRV(materials[i].diffusefile);
			subsets[i].mNormalSRV = texmgr.LoadTextureSRV(materials[i].normalmapfile);
			subsets[i].mLodIndices[0].mCount = filesubsets[i].indexcount;
			subsets[i].mLodIndices[0].mOffset = filesubsets[i].indexoffset;
			BZero(subsets[i].mMat);
			std::memcpy(&subsets[i].mMat.ambient, &materials[i].ambient, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].mMat.diffuse, &materials[i].diffuse, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].mMat.specular, &materials[i].specular, sizeof(XMFLOAT3));
			subsets[i].mMat.specular.w = materials[i].specPow;
			//subsets[i].m_mat.reflect = materials[i].reflect;
		}
	}

	inline  static void ReadVertices(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::vertex>& vertices)
	{
		auto size = vertices.size();
		std::vector<MeshFile::MeshVertex> _vertices(size);
		auto buffersize = _vertices.size() * sizeof(MeshFile::MeshVertex);
		min.Read(&_vertices[0], buffersize, offset);

		for (std::size_t i = 0; i != size; ++i)
		{
			vertices[i] = _vertices[i];
		}

		offset += buffersize;
	}

	inline  static void ReadIndices(const MemoryChunk& min, std::uint64_t& offset, std::vector<std::uint32_t>& indices)
	{
		auto size = indices.size() * sizeof(std::uint32_t);
		min.Read(&indices[0], size, offset);
		offset += size;
	}

	inline static void ReadLodIndices(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::SubSet>&subsets, std::vector<std::uint32_t>& indices){
		std::uint32_t lodCount = 0;
		min.Read(&lodCount, 4, offset);
		offset += 4;
		assert(lodCount < 5 && lodCount > 1);
		auto lodIndicesCount = leo::make_unique<std::uint32_t[]>(lodCount - 1);
		min.Read(lodIndicesCount.get(), 4 * (lodCount - 1), offset);
		offset += (4 * (lodCount - 1));

		auto IndicesCount = [&]{
			auto c = 0u;
			for (auto i = 0u; i != lodCount - 1; ++i)
				c += lodIndicesCount[lodCount];
			return c;
		}();
		auto baseIndex = indices.size();
		indices.resize(indices.size() + IndicesCount);
		min.Read(&indices[baseIndex], IndicesCount * 4, offset);
		offset += IndicesCount * 4;

		auto size = subsets.size();
		std::vector<MeshFile::MeshSubSet> filesubsets((lodCount - 1)*size);
		min.Read(&filesubsets[0], sizeof(MeshFile::MeshSubSet)*filesubsets.size(), offset);
		for (auto i = 1u; i != lodCount; ++i){
			for (auto s = 0u; s != size; ++s){
				subsets[s].mLodIndices[i].mCount = filesubsets[(i - 1)*size + s].indexcount;
				subsets[s].mLodIndices[i].mOffset = filesubsets[(i - 1)*size + s].indexoffset;
			}
		}
		offset += (sizeof(MeshFile::MeshSubSet)*filesubsets.size());
	}

	std::shared_ptr<SkeletonData> SkeletonData::Load(const wchar_t* fileName){

	}
	//从内存载入,未实现
	std::shared_ptr<SkeletonData> SkeletonData::Load(const MemoryChunk& memory){

	}


	

	

	


	bool Mesh::Load(const std::wstring& filename, ID3D11Device* device)
	{
		if (filename.size() == 0)
			return false;
		auto min = win::File::OpenNoThrow(filename, win::File::TO_READ);
		try{
			leo::MeshFile::MeshFileHeader l3d_header;
			std::uint64_t fileoffset = 0;
			min->Read(&l3d_header, sizeof(l3d_header), fileoffset);
			fileoffset += sizeof(l3d_header);
			m_subsets.resize(l3d_header.numsubset);
			auto idx = filename.rmind(L'\\');
			assert(idx != std::wstring::npos);
			auto dir = filename.substr(0, idx + 1);
			SetCurrentDirectory(dir.c_str());
			ReadSubSets(min, fileoffset, m_subsets, device);
			SetCurrentDirectory(L"..\\");
			dir.resize(260);
			GetCurrentDirectory(260, &dir[0]);
			std::vector<vertex_type> vertices(l3d_header.numvertice);
			ReadVertices(min, fileoffset, vertices);
			std::vector<std::uint32_t> indices(l3d_header.numindex);
			ReadIndexs(min, fileoffset, indices);

			D3D11_BUFFER_DESC Desc;
			Desc.Usage = D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;
			Desc.StructureByteStride = 0;
			Desc.ByteWidth = static_cast<win::uint> (sizeof(vertex_type)*vertices.size());

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
}