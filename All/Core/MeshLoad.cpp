#include "..\IndePlatform\platform.h"

#include "MeshLoad.hpp"
#include "..\file.hpp"
#include "..\exception.hpp"
#include "..\IndePlatform\\string.hpp"
#include "Vertex.hpp"
#include <fstream>
#include <vector>
#include <type_traits>
namespace leo
{
	/*文件格式:
	文件头 MeshFileHeader
	材质0-------------------DirectX::XMFLOAT3 ambient;
	DirectX::XMFLOAT3 diffuse;
	DirectX::XMFLOAT3 specular;
	DirectX::XMFLOAT3 reflect;
	float specPow;
	float alphaclip;
	wchar_t diffusefile[260];
	wchar_t normalmapfile[260];
	材质1
	....
	顶点0 ------------------XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 tex;
	XMFLOAT3 tangent;
	顶点1
	顶点2
	...
	索引0 ------------------std::uint32_t
	索引1
	...
	*/
	//附带骨骼动画信息1.02
	/*
	动画头: SkeletonHeader           ------------add动画数量
	                                 ------------关节数量
									 ------------是否循环
	特殊hack,如果动画数量和关节数量都等于0xffffffff ,是否循环为0xff,则读入Lod信息
	第0成LOD在上面,接下来的第一个数据是LOD层次数,并存放n-1个std::uint32_t指出该LOD层次索引数
	接下来是LOD1的索引...LOD(n-1)的索引
	接下来LOD1的对应的SubSet Offset,Count LOD(n-1)
	接下来是动画头...........
	附加顶点数据---------------------索引 std::uint32_t
	--------------------权重	 float[3];
	关节信息0   --------------------float data[16];
	wchar_t name[260];
	std::uint8_t parent;
	关节信息n
	动画X:
	动画名字   ----------------wchar_t[260]
	动画采样数 ----------------std::uint32_t
	动画采样时间点 ------------float [动画采样数]
	关节0信息:
	采样1	 -----------------Sqt;
	.....
	采样n	 -----------------
	关节i信息
	.....
	关节n信息
	动画I:
	....
	动画Y:
	*/
	template<>
	struct Vertex::is_vertex<MeshFile::MeshVertex> : std::true_type
	{};

	template<>
	void Vertex::NormalMap::operator=(const MeshFile::MeshVertex& lvalue)
	{
		memcpy(normal, lvalue.normal);
		memcpy(pos, lvalue.pos);
		memcpy(tex, lvalue.tex);
		memcpy(tangent, lvalue.tangent);
	}

	template<>
	inline XMFLOAT2 Subtract(const XMFLOAT2& lhs, const XMFLOAT2& rhs)
	{
		return XMFLOAT2(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	static void saveclips(std::unique_ptr<win::File>& fout, std::uint64_t& fileoffset, const std::vector<MeshFile::AnimatClip>& clips, std::uint32_t numJoint)
	{
		for (auto & clip : clips){
			fout->Write(fileoffset, clip.name, sizeof(clip.name));
			fileoffset += sizeof(clip.name);
			fout->Write(fileoffset, &clip.size, sizeof(std::uint32_t));
			fileoffset += sizeof(std::uint32_t);
			fout->Write(fileoffset, clip.timedata.get(), sizeof(float)*clip.size);
			fileoffset += sizeof(float)*clip.size;
			for (auto i = 0u; i != numJoint; ++i){
				fout->Write(fileoffset, clip.data[i].data.get(), sizeof(SeqSQT)* clip.size);
				fileoffset += sizeof(SeqSQT)* clip.size;
			}
		}
	}

	void MeshFile::m3dTol3d(const std::wstring& m3dfilename, const std::wstring& l3dfilename)
	{
		MeshFileHeader l3d_header;

		std::ifstream fin(m3dfilename);
		auto fout = win::File::OpenNoThrow(l3dfilename,win::File::TO_WRITE);
		UINT numSRVs = 0;
		UINT numVertices = 0;
		UINT numTriangles = 0;
		UINT numJoints = 0;
		UINT numAnimations = 0;

		std::string ignore;
		float fignore;
		std::uint64_t fileoffset = 0;

		//读取和写入头
		{
			if (fin)
			{
				fin >> ignore; // file header text
				fin >> ignore >> numSRVs;
				fin >> ignore >> numVertices;
				fin >> ignore >> numTriangles;
				fin >> ignore >> numJoints;
				fin >> ignore >> numAnimations;
			}
			l3d_header.numsubset = numSRVs;
			l3d_header.numvertice = numVertices;
			l3d_header.numindex = numTriangles * 3;
			fout->Write(fileoffset, &l3d_header, sizeof(l3d_header));
			fileoffset += sizeof(l3d_header);
		}

		std::string diffuseMapName;
		std::string normalMapName;

		MeshMaterial meshmat;
		//读取和写入材质信息
		{
			fin >> ignore; // materials header text

			std::wstring wstring;
			for (UINT i = 0; i < numSRVs; ++i)
			{
				fin >> ignore >> meshmat.ambient.x >> meshmat.ambient.y >> meshmat.ambient.z;//ambient
				fin >> ignore >> meshmat.diffuse.x >> meshmat.diffuse.y >> meshmat.diffuse.z;//diffuse
				fin >> ignore >> meshmat.specular.x >> meshmat.specular.y >> meshmat.specular.z;//specular
				fin >> ignore >> meshmat.specPow;//specPow
				fin >> ignore >> meshmat.reflect.x >> meshmat.reflect.y >> meshmat.reflect.z;//reflect
				fin >> ignore >> meshmat.alphaclip;//alphaclip
				fin >> ignore >> ignore;//effct
				fin >> ignore >> diffuseMapName;
				wstring = std::move(leo::to_wstring(diffuseMapName));
				wstring.resize(260, L'\0');
				wmemcpy(meshmat.diffusefile, wstring.c_str(), 260);//diffusefilename
				fin >> ignore >> normalMapName;
				wstring = std::move(leo::to_wstring(normalMapName));
				wstring.resize(260, L'\0');
				wmemcpy(meshmat.normalmapfile, wstring.c_str(), 260);//normalmapfilename
				fout->Write(fileoffset, &meshmat, sizeof(meshmat));
				fileoffset += sizeof(meshmat);
			}
		}

		UINT iignore;
		std::vector<MeshSubSet> subsets(numSRVs);
		//读取和写入网格索引
		{
			fin >> ignore; // subset header text

			for (UINT i = 0; i < numSRVs; ++i)
			{
				fin >> ignore >> iignore;//Id
				fin >> ignore >> iignore;//vertexstart
				fin >> ignore >> iignore;//vertexcount
				fin >> ignore >> subsets[i].indexoffset;//face start
				subsets[i].indexoffset *= 3;
				fin >> ignore >> subsets[i].indexcount;//face count
				subsets[i].indexcount *= 3;
			}
			fout->Write(fileoffset, &subsets[0], subsets.size()*sizeof(MeshSubSet));
			fileoffset += subsets.size()*sizeof(MeshSubSet);
		}
		std::vector<MeshVertex> vertices(numVertices);
		std::vector<SkeletonAdjInfo> veradjInfo(numVertices);
		//读取写入顶点信息
		{
			fin >> ignore; // vertices header text
			uint32 jointIndices[4];
			float weights[3];
			if (!numJoints)
				for (UINT i = 0; i < numVertices; ++i)
				{
				fin >> ignore >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
				fin >> ignore >> vertices[i].tangent.x >> vertices[i].tangent.y >> vertices[i].tangent.z >> fignore;
				fin >> ignore >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
				fin >> ignore >> vertices[i].tex.x >> vertices[i].tex.y;
				}
			else
				for (UINT i = 0; i < numVertices; ++i)
				{
				fin >> ignore >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
				fin >> ignore >> vertices[i].tangent.x >> vertices[i].tangent.y >> vertices[i].tangent.z >> fignore;
				fin >> ignore >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
				fin >> ignore >> vertices[i].tex.x >> vertices[i].tex.y;
				fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> fignore;
				fin >> ignore >> jointIndices[0] >> jointIndices[1] >> jointIndices[2] >> jointIndices[3];
				const uint32 indices = (jointIndices[0] << 24) | (jointIndices[1] << 16) | (jointIndices[2] << 8) | jointIndices[3];
				veradjInfo[i].indices = indices;
				std::memcpy(veradjInfo[i].weights, weights, sizeof(float) * 3);
				}
			fout->Write(fileoffset, &vertices[0], vertices.size()*sizeof(MeshVertex));
			fileoffset += vertices.size()*sizeof(MeshVertex);
		}

		std::vector<std::uint32_t> indices(l3d_header.numindex);
		//读取和写入索引信息
		{
			fin >> ignore; // triangles header text
			for (std::uint32_t i = 0; i < numTriangles; ++i)
			{
				fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
			}
			fout->Write(fileoffset, &indices[0], indices.size()*sizeof(std::uint32_t));
			fileoffset += indices.size()*sizeof(std::uint32_t);
		}

		if (!numJoints)
			return;

		SkeletonHeader ske_header{ numJoints, numAnimations, true };
		{
			fout->Write(fileoffset, &ske_header, sizeof(ske_header));
			fileoffset += sizeof(ske_header);
		}
		veradjInfo;
		{
			fout->Write(fileoffset, veradjInfo.data(), sizeof(SkeletonAdjInfo)*veradjInfo.size());
			fileoffset += sizeof(SkeletonAdjInfo)*veradjInfo.size();
		}



		std::wstring jointname{ L"Joint:    " };
		jointname += m3dfilename;
		jointname.resize(260);
		std::vector<Joint> joints(numJoints);
		{
			fin >> ignore; // BoneOffsets header text
			for (UINT i = 0; i < numJoints; ++i)
			{
				fin >> ignore >>
					joints[i](0, 0) >> joints[i](0, 1) >> joints[i](0, 2) >> joints[i](0, 3) >>
					joints[i](1, 0) >> joints[i](1, 1) >> joints[i](1, 2) >> joints[i](1, 3) >>
					joints[i](2, 0) >> joints[i](2, 1) >> joints[i](2, 2) >> joints[i](2, 3) >>
					joints[i](3, 0) >> joints[i](3, 1) >> joints[i](3, 2) >> joints[i](3, 3);
			}

			fin >> ignore; // BoneHierarchy header text
			int index;
			fin >> ignore >> index;
			joints[0].parent = 0xFFu;
			for (UINT i = 1; i < numJoints; ++i)
			{
				fin >> ignore >>iignore;
				joints[i].parent = iignore;
				jointname[7] = numJoints;
				std::memcpy(joints[i].name, jointname.data(), sizeof(wchar_t) * 260);
			}

			fout->Write(fileoffset, &joints[0], joints.size()*sizeof(Joint));
			fileoffset += joints.size()*sizeof(Joint);
		}

		std::vector<AnimatClip> clips(numAnimations);
		for (auto & clip : clips)
			clip.data = std::make_unique<JointAnimaSample[]>(numJoints);
		

		fin >> ignore;// AnimationClips header text
		std::string clipName;
		std::wstring wclipName;
		UINT numFrames = 0;
		for (auto & clip : clips)
		{
			fin >> ignore >> clipName;
			fin >> ignore;
			wclipName = to_wstring(clipName);
			wclipName.resize(259);
			std::wcscpy(clip.name, wclipName.c_str());

			bool alloc = false;
			for (auto j = 0u; j != numJoints; ++j){
				auto & jointSamples = clip.data[j];
				fin >> ignore >> ignore >> numFrames;
				fin >> ignore;

				if (!alloc){
					clip.size = numFrames;
					clip.timedata = std::make_unique<float[]>(clip.size);
					
					alloc = true;
				}
				jointSamples.data = std::make_unique<SeqSQT[]>(clip.size);

				for (auto f = 0u; f != numFrames; ++f){
					auto & sampleInfo = jointSamples.data[f];
					fin >> ignore >> clip.timedata[f];
					fin >> ignore >> sampleInfo.t[0] >> sampleInfo.t[1] >> sampleInfo.t[2];
					fin >> ignore >> sampleInfo.s >> sampleInfo.s >> sampleInfo.s;
					fin >> ignore >> sampleInfo.q[0] >> sampleInfo.q[1] >> sampleInfo.q[2] >> sampleInfo.q[3];
				}
				fin >> ignore;
			}


			fin >> ignore;
			
		}

		saveclips(fout, fileoffset, clips, numJoints);
	}

	namespace helper
	{
		template<typename T, typename U>
		static void _calc_normal(const std::true_type&, std::vector<T>& vertices, std::vector<U>& indices);
		template<typename T, typename U>
		static void _calc_normal(const std::false_type&, std::vector<T>& vertices, std::vector<U>& indices);

		template<typename T, typename U>
		static void CalcNormal(std::vector<T>& vertices, std::vector<U>& indices)
		{
			static_assert(Vertex::is_vertex<T>::value, "T must be a vertex type");
			static_assert(std::is_integral<U>::value, "U must be a integral type");
			static_assert(sizeof(U) <= 32, "Indices' size must be <= 32");
			_calc_normal(Vertex::is_ssevertex<T>(), vertices, indices);
		}


		template<typename T, typename U>
		static void _calc_tangent(const std::true_type&, std::vector<T>& vertices, std::vector<U>& indices);
		template<typename T, typename U>
		static void _calc_tangent(const std::false_type&, std::vector<T>& vertices, std::vector<U>& indices);

		template<typename T, typename U>
		static void CalcTangent(std::vector<T>& vertices, std::vector<U>& indices)
		{
			static_assert(Vertex::is_vertex<T>::value, "T must be a vertex type");
			static_assert(std::is_integral<U>::value, "U must be a integral type");
			static_assert(sizeof(U) <= 32, "Indices' size must be <= 32");
			_calc_tangent(Vertex::is_ssevertex<T>(), vertices, indices);
		}

		template<typename T, typename U>
		//SSE版本
		static void _calc_normal(const std::true_type&, std::vector<T>& vertices, std::vector<U>& indices)
		{
			static_assert(std::is_same<T, XMVECTOR>::value, "T must be XMVECTOR");
			for (auto i = 0; i != indices.size() / 3; ++i)
			{
				U ind[3];
				ind[0] = indices[i * 3 + 0];
				ind[1] = indices[i * 3 + 1];
				ind[2] = indices[i * 3 + 2];

				XMVECTOR v[3];
				v[0] = vertices[ind[0]];
				v[1] = vertices[ind[1]];
				v[2] = vertices[ind[2]];

				XMVECTOR e[2];
				e[0] = v[1].pos - v[0].pos;
				e[1] = v[2].pos - v[0].pos;
				XMVECTOR faceNormal = XMVector3Cross(e[0], e[1]);

				vertices[ind[0]].normal += faceNormal;
				vertices[ind[1]].normal += faceNormal;
				vertices[ind[2]].normal += faceNormal;
			}
			for (auto &v : vertices)
				v.normal = XMVector3Normalize(v.normal);
		}

		template<typename T, typename U>
		//CPU版本
		static void _calc_normal(const std::false_type&, std::vector<T>& vertices, std::vector<U>& indices)
		{
			for (auto i = 0; i != indices.size() / 3; ++i)
			{
				U ind[3];
				ind[0] = indices[i * 3 + 0];
				ind[1] = indices[i * 3 + 1];
				ind[2] = indices[i * 3 + 2];

				T v[3];
				v[0] = vertices[ind[0]];
				v[1] = vertices[ind[1]];
				v[2] = vertices[ind[2]];

				decltype(v[0].pos) e[2];
				e[0] = Subtract(v[1].pos, v[0].pos);
				e[1] = Subtract(v[2].pos, v[0].pos);

				/*
				decltype(v[0].tex) duv[2];
				duv[0] = Subtract(v[1].tex, v[0].tex);
				duv[1] = Subtract(v[2].tex, v[0].tex);
				auto inv = 1.f / (duv[0].x*duv[1].y - duv[0].y*duv[1].x);
				XMFLOAT3 tangent = XMFLOAT3(0.f, 0.f, 0.f);
				tangent.x = (duv[1].y*e[0].x - duv[0].y*e[1].x) / inv;
				tangent.y = (duv[1].y*e[0].y - duv[0].y*e[1].y) / inv;
				tangent.z = (duv[1].y*e[0].z - duv[0].y*e[1].z) / inv;
				*/
				decltype(v[0].pos) faceNormal = Cross(e[0], e[1]);

				vertices[ind[0]].normal = Add(vertices[ind[0]].normal, faceNormal);
				vertices[ind[1]].normal = Add(vertices[ind[1]].normal, faceNormal);
				vertices[ind[2]].normal = Add(vertices[ind[2]].normal, faceNormal);
			}
			for (auto &v : vertices)
				v.normal = Normalize(v.normal);
		}

		template<typename T, typename U>
		//SSE版本
		static void _calc_tangent(const std::true_type&, std::vector<T>& vertices, std::vector<U>& indices)
		{
			static_assert(std::is_same<T, XMVECTOR>::value, "T must be XMVECTOR");
			for (auto i = 0; i != indices.size() / 3; ++i)
			{
				U ind[3];
				ind[0] = indices[i * 3 + 0];
				ind[1] = indices[i * 3 + 1];
				ind[2] = indices[i * 3 + 2];

				XMVECTOR v[3];
				v[0] = vertices[ind[0]];
				v[1] = vertices[ind[1]];
				v[2] = vertices[ind[2]];

				XMVECTOR _duv[2];
				_duv[0] = v[1].tex - v[0].tex;
				_duv[1] = v[2].tex - v[0].tex;

				XMFLOAT2A duv[2];
				XMStoreFloat2A(&duv[0], _duv[0]);
				XMStoreFloat2A(&duv[1], _duv[1]);

				auto inv = 1.f / (duv[0].x*duv[1].y - duv[0].y*duv[1].x);
				XMFLOAT3A _tangent = XMFLOAT3A(0.f, 0.f, 0.f);
				_tangent.x = (duv[1].y*e[0].x - duv[0].y*e[1].x) / inv;
				_tangent.y = (duv[1].y*e[0].y - duv[0].y*e[1].y) / inv;
				_tangent.z = (duv[1].y*e[0].z - duv[0].y*e[1].z) / inv;

				XMVECTOR tangent = XMLoadFloat3A(&_tangent);
				vertices[ind[0]].tangent += tangent;
				vertices[ind[1]].tangent += tangent;
				vertices[ind[2]].tangent += tangent;
			}
			for (auto &v : vertices)
				v.tangent = XMVector3Normalize(v.tangent);
		}

		template<typename T, typename U>
		//CPU版本
		static void _calc_tangent(const std::false_type&, std::vector<T>& vertices, std::vector<U>& indices)
		{
			for (auto i = 0; i != indices.size() / 3; ++i)
			{
				U ind[3];
				ind[0] = indices[i * 3 + 0];
				ind[1] = indices[i * 3 + 1];
				ind[2] = indices[i * 3 + 2];

				T v[3];
				v[0] = vertices[ind[0]];
				v[1] = vertices[ind[1]];
				v[2] = vertices[ind[2]];

				decltype(v[0].pos) e[2];
				e[0] = Subtract(v[1].pos, v[0].pos);
				e[1] = Subtract(v[2].pos, v[0].pos);


				decltype(v[0].tex) duv[2];
				duv[0] = Subtract(v[1].tex, v[0].tex);
				duv[1] = Subtract(v[2].tex, v[0].tex);
				auto inv = 1.f / (duv[0].x*duv[1].y - duv[0].y*duv[1].x);
				XMFLOAT3 tangent = XMFLOAT3(0.f, 0.f, 0.f);
				tangent.x = (duv[1].y*e[0].x - duv[0].y*e[1].x) / inv;
				tangent.y = (duv[1].y*e[0].y - duv[0].y*e[1].y) / inv;
				tangent.z = (duv[1].y*e[0].z - duv[0].y*e[1].z) / inv;


				vertices[ind[0]].tangent = Add(vertices[ind[0]].tangent, tangent);
				vertices[ind[1]].tangent = Add(vertices[ind[1]].tangent, tangent);
				vertices[ind[2]].tangent = Add(vertices[ind[2]].tangent, tangent);
			}
			for (auto &v : vertices)
				v.tangent = Normalize(v.tangent);
		}

		template<typename T, typename U>
		static void ClacNormalTangent(std::vector<T>& vertices, std::vector<U>& indices)
		{
			CalcNormal(vertices, indices);
			CalcTangent(vertices, indices);
		}

	}

	void MeshFile::terrainTol3d(const std::function<float(float, float)>& f, std::pair<std::uint32_t, std::uint32_t> size, std::pair<std::uint16_t, std::uint16_t> mn, const std::wstring& l3dfilename)
	{
		auto m = mn.first;
		auto n = mn.second;
		auto width = size.first;
		auto height = size.second;
		MeshFileHeader l3d_header;
		l3d_header.numvertice = m*n;
		l3d_header.numindex = (m - 1)*(n - 1) * 6;
		l3d_header.numsubset = 1;

		auto fout = win::File::OpenNoThrow(l3dfilename, win::File::TO_WRITE);

		std::uint64_t offset = 0;
		fout->Write(offset, &l3d_header, sizeof(MeshFileHeader));
		offset += sizeof(MeshFileHeader);

		MeshMaterial meshmat;
		BZero(meshmat);
		meshmat.ambient = XMFLOAT3(0.3f, 0.3f, 0.3f);
		meshmat.diffuse = XMFLOAT3(0.4f, 0.6f, 0.4f);
		meshmat.reflect = XMFLOAT3(0.2f, 0.4f, 0.2f);
		meshmat.specular = XMFLOAT3(0.1f, 0.1f, 0.1f);
		meshmat.specPow = 2;
		std::memcpy(meshmat.normalmapfile, L"DefaultNormalMap.dds", sizeof(L"DefaultNormalMap.dds"));
		std::memcpy(meshmat.diffusefile, L"DefaultDiffuse.dds", sizeof(L"DefaultDiffuse.dds"));
		fout->Write(offset, &meshmat, sizeof(MeshMaterial));
		offset += sizeof(MeshMaterial);

		MeshSubSet meshsub;
		meshsub.indexcount = l3d_header.numindex;
		meshsub.indexoffset = 0;
		fout->Write(offset, &meshsub, sizeof(MeshSubSet));
		offset += sizeof(MeshSubSet);

		std::vector<MeshVertex> vertices(l3d_header.numvertice);

		auto halfWidht = 0.5f*width;
		auto halfDepth = 0.5f*height;
		auto dx = width / (n - 1);
		auto dz = height / (m - 1);
		float du = 1.f / (n - 1);
		float dv = 1.f / (m - 1);
		for (auto i = 0; i != m; ++i)
		{
			auto z = halfDepth - i*dz;
			for (auto j = 0; j != n; ++j)
			{
				float x = -halfWidht + j*dx;
				float y = f(x, z);
				BZero(vertices[i*n + j]);
				vertices[i*n + j].pos = XMFLOAT3(x, y, z);
				vertices[i*n + j].tex = XMFLOAT2(j*du, i*dv);
			}
		}

		std::vector<std::uint32_t> indices(l3d_header.numindex);
		auto k = 0;
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
				k += 6;
			}
		}



		helper::ClacNormalTangent(vertices, indices);

		for (auto &v : vertices)
		{
			auto & p = v;
			float y = p.pos.y;
			leo::clamp(-20.f, 30.f, y);
			p.tex.y = (y + 20.f) / 50.1f;
			p.tex.y = (static_cast<int>(p.tex.y * 10)) / 10.f;
		}

		fout->Write(offset, &vertices[0], vertices.size()*sizeof(MeshVertex));
		offset += vertices.size()*sizeof(MeshVertex);
		fout->Write(offset, &indices[0], indices.size()*sizeof(std::uint32_t));
	}

	void MeshFile::w3eTol3d(const std::wstring& w3efilename, const std::wstring& l3dfilename)
	{
		//类型定义
		enum tileset_type :char
		{
			Ashenvale = 'A',//灰谷
			Barrens = 'B',//贫瘠之地
			Felwood = 'C',//费尔伍德
			Dungeon = 'D',//地下城
			Lordaeron_Fall = 'F',//洛丹伦的秋
			Underground = 'G',//秘密之地
			Lordaeron_Summer = 'L',//洛丹伦的夏
			Northread = 'N',//诺森德
			Village_Fall = 'Q',//乡村的瀑布
			Village = 'V',//乡村
			Lordaeron_Winter = 'W',//洛丹伦的冬
			Dalaran = 'X',//达拉然
			Cityscape = 'Y'//城市
		};
		struct W3eHeader
		{
			char id[4];//"W3E!"
			char version[4];//0X0000000B
			tileset_type maintileset;
		};

		auto fin = win::File::OpenNoThrow(w3efilename, win::File::TO_READ);

		std::uint64_t offset = 0;

		W3eHeader w3e_header;
		fin->Read(&w3e_header, sizeof(W3eHeader), offset);
		offset += sizeof(W3eHeader);

		if (w3e_header.id[0] != 'W' || w3e_header.id[1] != '3' || w3e_header.id[2] != 'E' || w3e_header.id[3] != '!' || w3e_header.version[0] != 11)
			Raise_Error_Exception(ERROR_INVALID_PARAMETER, "不是一个W3E文件");

		std::int32_t custom_tilesets_flag;
		fin->Read(&custom_tilesets_flag, 4, offset);
		offset += 4;
		std::int32_t tiesetsnum;
		fin->Read(&tiesetsnum, 4, offset);
		offset += 4;

		auto tilesetsIds = std::make_unique<char[]>(4 * tiesetsnum);
		fin->Read(tilesetsIds.get(), 4 * tiesetsnum, offset);
		offset += 4 * tiesetsnum;

		std::int32_t clifftilesetsnum;
		fin->Read(&clifftilesetsnum, 4, offset);
		offset += 4;

		auto clifftilesetsIds = std::make_unique<char[]>(4 * clifftilesetsnum);
		fin->Read(clifftilesetsIds.get(), 4 * clifftilesetsnum, offset);
		offset += 4 * clifftilesetsnum;

		std::int32_t mx;
		std::int32_t my;
		fin->Read(&mx, 4, offset);
		offset += 4;
		fin->Read(&my, 4, offset);
		offset += 4;
		auto n = mx; auto m = my;
		auto width = (n - 1) * 16; auto height = (m - 1) * 16;

		float xoffset, yoffset;
		fin->Read(&xoffset, 4, offset);
		offset += 4;
		fin->Read(&yoffset, 4, offset);
		offset += 4;

		struct _TilePoint
		{
			union
			{
				struct{
					std::uint8_t a;
					std::uint8_t b;
				};
				//std::int16_t height;
			};
			union
			{
				struct{
					std::uint8_t c;
					std::uint8_t d;
				};
				//std::int16_t water_flag;
			};
			std::uint8_t flag_groundtype;
			std::uint8_t detailtex;
			std::uint8_t clifftype_cellheight;
		};

		struct TilePoint
		{
			std::int16_t height;
			std::int16_t water_flag;
			std::uint8_t flag_groundtype;
			std::uint8_t detailtex;
			std::uint8_t clifftype_celllayer;

			TilePoint(const _TilePoint& lvalue)
			{
				height = (lvalue.b << 8) | lvalue.a;
				water_flag = (lvalue.d << 8) | lvalue.c;
				flag_groundtype = lvalue.flag_groundtype;
				detailtex = lvalue.detailtex;
				clifftype_celllayer = lvalue.clifftype_cellheight;
			}
		};
		static_assert(sizeof(_TilePoint) == 7, "7 bytes!");
		std::vector<_TilePoint> _tilepoints(m*n);
		fin->Read(&_tilepoints[0], sizeof(_TilePoint)*m*n, offset);
		offset += sizeof(_TilePoint)*m*n;

		std::vector<TilePoint> tilepoints;
		for (auto t : _tilepoints)
			tilepoints.emplace_back(t);

		_tilepoints.clear();

		MeshFileHeader l3d_header;
		l3d_header.numvertice = m*n;
		l3d_header.numindex = (m - 1)*(n - 1) * 6;
		l3d_header.numsubset = 1;

		auto fout = win::File::OpenNoThrow(l3dfilename, win::File::TO_WRITE);
		offset = 0;
		fout->Write(offset, &l3d_header, sizeof(MeshFileHeader));
		offset += sizeof(MeshFileHeader);

		MeshMaterial meshmat;
		BZero(meshmat);
		meshmat.ambient = XMFLOAT3(0.3f, 0.3f, 0.3f);
		meshmat.diffuse = XMFLOAT3(0.4f, 0.6f, 0.4f);
		meshmat.reflect = XMFLOAT3(0.2f, 0.4f, 0.2f);
		meshmat.specular = XMFLOAT3(0.1f, 0.1f, 0.1f);
		meshmat.specPow = 2;
		std::memcpy(meshmat.normalmapfile, L"DefaultNormalMap.dds", sizeof(L"DefaultNormalMap.dds"));
		std::memcpy(meshmat.diffusefile, L"DefaultDiffuse.dds", sizeof(L"DefaultDiffuse.dds"));
		fout->Write(offset, &meshmat, sizeof(MeshMaterial));
		offset += sizeof(MeshMaterial);

		MeshSubSet meshsub;
		meshsub.indexcount = l3d_header.numindex;
		meshsub.indexoffset = 0;
		fout->Write(offset, &meshsub, sizeof(MeshSubSet));
		offset += sizeof(MeshSubSet);

		std::vector<MeshVertex> vertices(l3d_header.numvertice);
		auto halfWidht = 0.5f*width;
		auto halfDepth = 0.5f*height;
		auto dx = width / (n - 1);
		auto dz = height / (m - 1);
		float du = 1.f / (n - 1);
		float dv = 1.f / (m - 1);
		for (auto i = 0; i != m; ++i)
		{
			auto z = -halfDepth + i*dz;
			for (auto j = 0; j != n; ++j)
			{
				float x = -halfWidht + j*dx;
				float y = (tilepoints[i*n + j].height - 0X2000 + (tilepoints[i*n + j].clifftype_celllayer & 0X0F - 2) * 0x0200) / 32.f;
				vertices[i*n + j].pos = XMFLOAT3(x, y, z);
				vertices[i*n + j].tex = XMFLOAT2(j*du, 1 - i*dv);
			}
		}

		std::vector<std::uint32_t> indices(l3d_header.numindex);
		auto k = 0;
		for (auto i = 0; i != m - 1; ++i)
		{
			for (auto j = 0; j != n - 1; ++j)
			{
				indices[k] = (i + 1)*n + j;
				indices[k + 1] = i*n + j + 1;
				indices[k + 2] = i*n + j;


				indices[k + 3] = (i + 1)*n + j + 1;
				indices[k + 4] = i*n + j + 1;
				indices[k + 5] = (i + 1)*n + j;
				k += 6;
			}
		}

		helper::ClacNormalTangent(vertices, indices);

		fout->Write(offset, &vertices[0], vertices.size()*sizeof(MeshVertex));
		offset += vertices.size()*sizeof(MeshVertex);
		fout->Write(offset, &indices[0], indices.size()*sizeof(std::uint32_t));
	}
}