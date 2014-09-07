#include "IEnumProc.h"
#include "l3d_mesh.h"
#include <conio.h>
#include <vector>
#include <set>
#include <algorithm>

struct MaxVertex
{
	Point3 pos;
	Point3 normal;
	Point3 tex;
};

inline DirectX::XMFLOAT3 transform(const Point3& p)
{
	return DirectX::XMFLOAT3(p.x, p.y, -p.z);
}

inline void transform(DirectX::XMFLOAT3& p)
{
	p.z = -p.z;
}

namespace leo
{
	template<typename T>
	inline T Normalize(const T& val)
	{
		float len = std::sqrtf(val.x * val.x + val.y * val.y + val.z * val.z);
		T ret(val.x / len, val.y / len, val.z / len);
		return ret;
	}

	template<typename T>
	inline T Add(const T& lhs, const T& rhs)
	{
		return T(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	DirectX::XMFLOAT2 inline Subtract(const DirectX::XMFLOAT2& lhs, const DirectX::XMFLOAT2& rhs)
	{
		return DirectX::XMFLOAT2(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	template<typename T>
	inline T Subtract(const T& lhs, const T& rhs)
	{
		return T(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}



	template<typename T>
	inline T Cross(const T& lhs, const T& rhs)
	{
		return T(
			(lhs.y * rhs.z) - (lhs.z * rhs.y),
			(lhs.z * rhs.x) - (lhs.x * rhs.z),
			(lhs.x * rhs.y) - (lhs.y * rhs.x)
			);
	}
}

namespace Tree
{


	template<typename T, typename U>
	//CPU版本
	static void calc_normal(std::vector<T>& vertices, std::vector<U>& indices)
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
			e[0] = leo::Subtract(v[1].pos, v[0].pos);
			e[1] = leo::Subtract(v[2].pos, v[0].pos);

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
			decltype(v[0].pos) faceNormal = leo::Cross(e[0], e[1]);

			vertices[ind[0]].normal = leo::Add(vertices[ind[0]].normal, faceNormal);
			vertices[ind[1]].normal = leo::Add(vertices[ind[1]].normal, faceNormal);
			vertices[ind[2]].normal = leo::Add(vertices[ind[2]].normal, faceNormal);
		}
		for (auto i = 0; i != vertices.size(); ++i)
			vertices[i].normal = leo::Normalize(vertices[i].normal);
	}

	template<typename T, typename U>
	//CPU版本
	static void calc_tangent(std::vector<T>& vertices, std::vector<U>& indices)
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
			e[0] = leo::Subtract(v[1].pos, v[0].pos);
			e[1] = leo::Subtract(v[2].pos, v[0].pos);


			decltype(v[0].tex) duv[2];
			duv[0] = leo::Subtract(v[1].tex, v[0].tex);
			duv[1] = leo::Subtract(v[2].tex, v[0].tex);
			auto inv = 1.f / (duv[0].x*duv[1].y - duv[0].y*duv[1].x);
			DirectX::XMFLOAT3 tangent = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
			tangent.x = (duv[1].y*e[0].x - duv[0].y*e[1].x) / inv;
			tangent.y = (duv[1].y*e[0].y - duv[0].y*e[1].y) / inv;
			tangent.z = (duv[1].y*e[0].z - duv[0].y*e[1].z) / inv;


			vertices[ind[0]].tangent = leo::Add(vertices[ind[0]].tangent, tangent);
			vertices[ind[1]].tangent = leo::Add(vertices[ind[1]].tangent, tangent);
			vertices[ind[2]].tangent = leo::Add(vertices[ind[2]].tangent, tangent);
		}
		for (auto i = 0; i != vertices.size(); ++i)
			vertices[i].tangent = leo::Normalize(vertices[i].tangent);
	}

	std::vector<Mesh*> meshVec;

	void EnumProc::Read(INode * node, Object* obj)
	{
		Class_ID triId(TRIOBJ_CLASS_ID, 0);
		auto p = reinterpret_cast<TriObject*>(obj->ConvertToType(0, triId));
#if 1
		if (!p){
			cprintf("Error: the obj can't ConvertTo TRI\n");
			return;
		}

#endif
		meshVec.push_back(&p->mesh);

		auto& pMesh = p->mesh;
		auto verticesNum = pMesh.getNumVerts();
		auto faceNum = pMesh.getNumFaces();
#if 1
		cprintf("Mesh Info: NumVerts: %d,NumFaces: %d\n", verticesNum, faceNum);
#endif

		MeshVertex objVer;
		MaxVertex tempVer;

		std::uint32_t baseIndex = l3d_header.numvertice;

		for (int i = 0; i != verticesNum; ++i)
		{
			std::memcpy(&objVer.pos, &pMesh.getVert(i), sizeof(DirectX::XMFLOAT3));
			transform(objVer.pos);
			l3d_vertexs.push_back(objVer);
		}

		if (pMesh.faces)
		{
#if 1
			cprintf("Mesh Info: Has Faces\n");
#endif
			for (int i = 0; i != verticesNum; ++i)
			{
				std::memcpy(&l3d_vertexs[baseIndex + i].normal, &pMesh.getNormal(i), sizeof(DirectX::XMFLOAT3));
			}
		}

		if (pMesh.tVerts)
		{
#if 1
			cprintf("Mesh Info: Has TexureCoord\n");
#endif
			for (int i = 0; i != verticesNum; ++i)
			{
				std::memcpy(&l3d_vertexs[baseIndex + i].tex, &pMesh.getTVert(i), sizeof(DirectX::XMFLOAT2));
			}
		}

		l3d_header.numvertice += verticesNum;


		auto mats = node->GetMtl();

		if (mats)
		{
#if 1
			cprintf("Mesh Info: Has Material\n");
#endif
			for (std::size_t i = 0; i != faceNum; ++i)
			{
				//获取材质
				auto mtlIndex = pMesh.getFaceMtlIndex(i);
				auto ambient = mats->GetAmbient(mtlIndex);
				auto diffuse = mats->GetDiffuse(mtlIndex);
				auto spec = mats->GetSpecular(mtlIndex);
				auto diffusename = mats->GetSubTexmap(mtlIndex) ? mats->GetSubTexmap(mtlIndex)->GetName() : WStr(L"DefaultDiffuse.dds");

				using namespace DirectX;
				//构造材质
				MeshMaterial mat;
				mat.ambient = XMFLOAT3(ambient.r, ambient.g, ambient.b);
				mat.diffuse = XMFLOAT3(diffuse.r, diffuse.g, diffuse.b);
				mat.specular = XMFLOAT3(spec.r, spec.g, spec.b);
				mat.specPow = 2;
				diffusename.data() ? std::wcscpy(mat.diffusefile, diffusename.data()) : 0;
				std::memcpy(mat.normalmapfile, L"DefaultNormalMap.dds", sizeof(L"DefaultNormalMap.dds"));

				//查找材质
				std::size_t index = l3d_mats.size();
				auto npos = std::distance(l3d_mats.cbegin(), std::find(l3d_mats.cbegin(), l3d_mats.cend(), mat));
				if (npos != index)
				{
					index = npos;
				}
				else
				{
					cprintf("Ctor New Material\n");
					//如果没有默认材质,构造一个默认材质
					//取得默认材质下标
					l3d_mats.emplace_back(mat);
					l3d_indexss.emplace_back(std::vector<std::uint32_t>());
				}

				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[2]);
				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[1]);
				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[0]);
			}
		}
		else
		{
#if 1
			cprintf("Mesh Info: No Material\n");
#endif
			std::size_t index = l3d_mats.size();
			auto npos = std::find(l3d_mats.cbegin(), l3d_mats.cend(), MeshMaterial(MeshMaterial::Ctor())) - l3d_mats.cbegin();
			if (npos != index)
			{
				index = npos;
			}
			else
			{
				//如果没有默认材质,构造一个默认材质
				//取得默认材质下标
				l3d_mats.push_back(MeshMaterial::Ctor());
				l3d_indexss.push_back(std::vector<std::uint32_t>());
			}
			for (std::size_t i = 0; i != faceNum; ++i)
			{
				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[2]);
				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[1]);
				l3d_indexss[index].push_back(baseIndex + pMesh.faces[i].v[0]);
			}
		}

		//_cprintf("triangle %d : %d %d %d\n", i, baseIndex + pMesh.faces[i].v[0], baseIndex + pMesh.faces[i].v[1], baseIndex + pMesh.faces[i].v[2]);
	}

	int EnumProc::callback(INode* node)
	{
		auto os = node->EvalWorldState(ip->GetTime());
		if (os.obj)
		{
			switch (os.obj->SuperClassID())
			{
			case GEOMOBJECT_CLASS_ID:
			{
				_cprintf("Address: %p,Name: %s\n",node, node->GetName());
				Read(node, os.obj);
			}
				return TREE_CONTINUE;
			default:
				break;
			}
		}
		return TREE_CONTINUE;
	}

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
	void EnumProc::Save(const TCHAR*name)
	{
		_cwprintf(L"%s", name);
		if (l3d_indexss.size() > 255)
			printf("Error: 超出材质上限!\n");
		l3d_header.numsubset =static_cast<uint8_t>(l3d_indexss.size());
		if (l3d_header.numvertice != l3d_vertexs.size())
			_cwprintf(L"顶点数据错误\n");
		_cwprintf(L"\nTotal NumVertices: %d", l3d_header.numvertice);
		_cwprintf(L"Total NumSubSets: %d\n", l3d_header.numsubset);
		

		auto handle = CreateFileW(
			name,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
			);
		if (handle == INVALID_HANDLE_VALUE)
			_cwprintf(L"Error: Can't Create %s file", name);

		std::uint64_t u64Offset = 0;

		OVERLAPPED Overlapped;
		ZeroMemory(&Overlapped, sizeof(OVERLAPPED));
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);

		std::uint32_t indexoffset = 0;
		std::vector<uint32_t> l3d_indexs;
		std::vector<MeshSubSet> l3d_subsets;
		for (int i = 0; i != l3d_indexss.size(); ++i)
		{
			l3d_subsets.push_back(MeshSubSet(indexoffset, static_cast<std::uint32_t>(l3d_indexss[i].size())));
			l3d_indexs.insert(l3d_indexs.cend(), l3d_indexss[i].cbegin(), l3d_indexss[i].cend());
			cprintf("%d th SubSets Indexs Num %d\n", i, l3d_indexss[i].size());
			indexoffset += l3d_indexss[i].size();
		}
		if (l3d_indexs.size() != indexoffset)
			cprintf("Error : 索引数量错误\n");
		cprintf("Indexs Num %d\n", l3d_indexs.size());
		l3d_header.numindex = l3d_indexs.size();

		DWORD NumOfWritten;
		//头写入
		WriteFile(handle, &l3d_header, sizeof(l3d_header), &NumOfWritten, &Overlapped);

		u64Offset += sizeof(l3d_header);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);


		//材质写入
		WriteFile(handle, l3d_mats.data(), l3d_mats.size()* sizeof(MeshMaterial), &NumOfWritten, &Overlapped);
		u64Offset += l3d_mats.size()* sizeof(MeshMaterial);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);

		

		//分割信息写入
		WriteFile(handle, l3d_subsets.data(), sizeof(MeshSubSet)*l3d_subsets.size(), &NumOfWritten, &Overlapped);
		u64Offset += sizeof(MeshSubSet)*l3d_subsets.size();
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);

		

		if (l3d_vertexs[0].normal != DirectX::XMFLOAT3(0.f, 0.f, 0.f))
			calc_normal(l3d_vertexs, l3d_indexs);

		calc_tangent(l3d_vertexs, l3d_indexs);
		WriteFile(handle, l3d_vertexs.data(), l3d_vertexs.size()*sizeof(MeshVertex), &NumOfWritten, &Overlapped);
		u64Offset += l3d_vertexs.size()*sizeof(MeshVertex);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);


		WriteFile(handle, l3d_indexs.data(), l3d_indexs.size()*sizeof(std::uint32_t), &NumOfWritten, &Overlapped);
		u64Offset += l3d_indexs.size()*sizeof(std::uint32_t);
		Overlapped.Offset = (DWORD)u64Offset;
		Overlapped.OffsetHigh = (DWORD)(u64Offset >> 32);

		CloseHandle(handle);
	}
}