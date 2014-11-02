#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <functional>

#include "..\leomath.hpp"
namespace leo
{
	class MeshFile
	{
	private:
	public:
		struct MeshFileHeader
		{
			std::uint8_t numsubset;
			std::uint32_t numvertice;
			std::uint32_t numindex;
		};
		struct MeshMaterial
		{
			DirectX::XMFLOAT3 ambient;
			DirectX::XMFLOAT3 diffuse;
			DirectX::XMFLOAT3 specular;
			DirectX::XMFLOAT3 reflect;
			float specPow;
			float alphaclip;
			wchar_t diffusefile[260];
			wchar_t normalmapfile[260];
		};
		struct MeshSubSet
		{
			std::uint32_t indexoffset;
			std::uint32_t indexcount;
		};
		struct MeshVertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT2 tex;
			DirectX::XMFLOAT3 tangent;
		};

		struct SkeletonHeader{
			std::uint32_t numjoint;
			std::uint32_t numanima;
			bool loop;
			//吗,时间默认为1帧糊弄着再说
		};

		struct SkeletonAdjInfo{
			std::uint32_t indices;
			float weights[3];
		};

		struct Joint{

			float data[16];
			wchar_t name[260];
			std::uint8_t parent;

			float& operator()(uint8 row, uint8 col){
				return data[row * 4 + col];
			}
		};

		struct JointAnimaSample{
			//一个动画中的一个关节的所有采样信息
			std::unique_ptr<SeqSQT[]> data;
		};

		struct AnimatClip{
			wchar_t name[260];
			std::uint32_t size;
			std::unique_ptr<float[]> timedata;
			std::unique_ptr<JointAnimaSample[]> data;//size = numjoints
		};
		static void m3dTol3d(const std::wstring& m3dfilename, const std::wstring& l3dfilename);

		static void sdkmeshTol3d(const std::wstring& sdkmeshfilename, const std::wstring& l3dfilename);


		//1.........n
		//.
		//.
		//.
		//.
		//m
		static void terrainTol3d(const std::function<float(float, float)>& f, std::pair<std::uint32_t, std::uint32_t> size, std::pair<std::uint16_t, std::uint16_t> mn, const std::wstring& l3dfilename);

		static void w3eTol3d(const std::wstring& w3efilename, const std::wstring& l3dfilename);
	};
}