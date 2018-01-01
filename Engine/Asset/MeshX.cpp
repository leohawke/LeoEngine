#include "MeshX.h"
#include "Loader.hpp"
#include<LBase/typeinfo.h>
#include "../Core/LFile.h"
#include <LBase/lmathtype.hpp>
namespace platform {
	//Mesh文件格式
	struct MeshHeader
	{
		//TODO Asset Common Header?
		leo::uint32 Signature; //Magic Number == 'MESH'
		leo::uint16 Machine;//资源所属的平台格式
		leo::uint16 NumberOfSections;//主要的设计概念,SectionLoader将会加载出对应的资源,目前只实现一种Section
		union {
			leo::uint16 SizeOfOptional;//必须保证对齐 %16 == 0
			leo::uint16 FirstSectionOffset;//该偏移为相对文件的偏移,注意一定会进行对齐
		};
	};

	//每个Section的通用头
	struct SectionCommonHeader {
		leo::uint16 SectionIndex;//这是当前的第几节
		leo::uint32 NextSectionOffset;//下一节开始相对文件的偏移
		stdex::byte Name[8];//使用名字决定节加载器
		leo::uint32 Size;//本节的大小,意味着节是由最大大小限制的,注意是包含填充大小的
	};

	lconstexpr leo::uint32 GeomertyCurrentVersion = 0;

	struct GeomertySectionHeader :SectionCommonHeader {
		leo::uint32 FourCC; //'LEME'
		leo::uint32 Version;//保留支持
		leo::uint32 CompressVersion;//保留支持,可能会引入内置压缩
		union {
			leo::uint16 SizeOfOptional;//必须保证对齐
			leo::uint16 GeomertyOffset;//该偏移为相对文件的偏移,注意一定会进行对齐
		};//保留支持，值应该是0
	};

	template<typename type>
	type Read(FileRead& file) {
		type value;
		file.Read(&value, sizeof(type));
		return value;
	}


	class MeshSectionLoading {
	protected:
		using AssetType = asset::MeshAsset;
		std::shared_ptr<AssetType> mesh_asset;
		MeshSectionLoading(std::shared_ptr<AssetType> target)
			:mesh_asset(target) {
		}
	public:
		virtual std::experimental::generator<std::shared_ptr<AssetType>> Coroutine(FileRead& file) = 0;
		virtual std::array<byte, 8> Name() = 0;
	};

	class GemoertySection :public MeshSectionLoading {
	public:
		GemoertySection(std::shared_ptr<AssetType> target)
		:MeshSectionLoading(target){
		}
		std::array<byte, 8> Name() {
			static std::array<byte, 8> name = { 'G','E','M','O','E','R','T','Y' };
			return name;
		}

		/*
		struct GeometrySection {
			leo::uint8 VertexElmentsCount;
			Vertex::Element VertexElments[VertexElmentsCount];
			EFromat IndexFormat;//R16UI | R32UI
			leo::uint16/32 VertexCount;
			leo::uint16/32 IndexCount;

			byte[VertexElments[0...n].Size()][VertexCount];
			byte[IndexFormat.Size()][IndexCount];

			leo::uint8 MeshesCount;
			struct SubMeshDescrption{
				leo::uint8 MaterialIndex;
				leo::uint8 LodsCount;
				AABB<float3> PosAABB;
				AABB<float2> TexAABB;
				strcut LodDescrption{
					leo::uint16/32 VertexNum;
					leo::uint16/32 VertexBase;
					leo::uint16/32 IndexNum;
					leo::uint16/32 IndexBase;
				}[LodsCount];
			}[MeshesCount]
		};
		*/
		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine(FileRead& file) override {
			//read header
			GeomertySectionHeader header;//common header had read
			file.Read(&header.FourCC, sizeof(header.FourCC));
			if (header.FourCC != asset::four_cc_v < 'L', 'E', 'M', 'E'>)
				co_return;

			file.Read(&header.Version, sizeof(header.Version));
			if (header.Version != GeomertyCurrentVersion)
				co_return;

			file.Read(&header.CompressVersion, sizeof(header.CompressVersion));
			if (header.CompressVersion != 0)
				co_return;

			file.Read(&header.SizeOfOptional, sizeof(header.SizeOfOptional));
			if (header.SizeOfOptional != 0)
				co_return;
			co_yield mesh_asset;

			leo::uint8 VertexElmentsCount = Read<leo::uint8>(file) ;
			auto & vertex_elements = mesh_asset->GetVertexElementsRef();
			vertex_elements.reserve(VertexElmentsCount);
			for (auto i = 0; i != VertexElmentsCount; ++i) {
				vertex_elements.emplace_back(Read<Render::Vertex::Element>(file));
			}
			auto index_format = Read<Render::EFormat>(file);
			mesh_asset->SetIndexFormat(index_format);

			uint32 vertex_count = index_format == Render::EFormat::EF_R16UI ? Read<uint16>(file) : Read<uint32>(file);
			uint32 index_count = index_format == Render::EFormat::EF_R16UI ? Read<uint16>(file) : Read<uint32>(file);

			auto & vertex_streams = mesh_asset->GetVertexStreamsRef();
			for (auto i = 0; i != VertexElmentsCount; ++i) {
				auto vertex_stream = std::make_unique<stdex::byte[]>(vertex_elements[i].GetElementSize()*vertex_count);
				file.Read(vertex_stream.get(), vertex_elements[i].GetElementSize()*vertex_count);
				vertex_streams.emplace_back(std::move(vertex_stream));
			}

			auto index_stream = std::make_unique<stdex::byte[]>(Render::NumFormatBytes(index_format)*index_count);
			file.Read(index_stream.get(), Render::NumFormatBytes(index_format)*index_count);
			mesh_asset->GetIndexStreamsRef() = std::move(index_stream);

			auto & sub_meshes = mesh_asset->GetSubMeshDescesRef();
			auto sub_mesh_count = Read<leo::uint8>(file);
			for (auto i = 0; i != sub_mesh_count; ++i) {
				asset::MeshAsset::SubMeshDescrption mesh_desc;
				mesh_desc.MaterialIndex = Read<leo::uint8>(file);
				auto lods_count = Read<leo::uint8>(file);

				auto pc = Read<leo::math::data_storage<float, 3>>(file);
				auto po = Read<leo::math::data_storage<float, 3>>(file);
				auto tc = Read<leo::math::data_storage<float, 2>>(file);
				auto to = Read<leo::math::data_storage<float, 2>>(file);

				for (auto lod_index = 0; lod_index != lods_count; ++lod_index) {
					asset::MeshAsset::SubMeshDescrption::LodDescription lod_desc;
					if (index_format == Render::EFormat::EF_R16UI) {
						lod_desc.VertexNum = Read<leo::uint16>(file);
						lod_desc.VertexBase = Read<leo::uint16>(file);
						lod_desc.IndexNum = Read<leo::uint16>(file);
						lod_desc.IndexBase = Read<leo::uint16>(file);
					}
					else {
						lod_desc.VertexNum = Read<leo::uint32>(file);
						lod_desc.VertexBase = Read<leo::uint32>(file);
						lod_desc.IndexNum = Read<leo::uint32>(file);
						lod_desc.IndexBase = Read<leo::uint32>(file);
					}
					mesh_desc.LodsDescription.emplace_back(lod_desc);
				}
				sub_meshes.emplace_back(std::move(mesh_desc));
			}
		}
	};

	template<typename... section_types>
	class MeshLoadingDesc : public asset::AssetLoading<asset::MeshAsset> {
	private:
		struct MeshDesc {
			X::path mesh_path;
			std::shared_ptr<AssetType> mesh_asset;
			std::vector<std::unique_ptr<MeshSectionLoading>> section_loaders;
		} mesh_desc;
	public:
		explicit MeshLoadingDesc(X::path const & meshpath)
		{
			mesh_desc.mesh_path = meshpath;
			mesh_desc.mesh_asset = std::make_shared<AssetType>();
			ForEachSectionTypeImpl<std::tuple<section_types...>>(leo::make_index_sequence<sizeof...(section_types)>());
		}

		template<typename tuple,size_t... indices>
		void ForEachSectionTypeImpl(leo::index_sequence<indices...>) {
			int ignore[] = { (static_cast<void>(
				mesh_desc.section_loaders
				.emplace_back(
					std::make_unique<std::tuple_element_t<indices,tuple>>(mesh_desc.mesh_asset))),0)
				...
			};
			(void)ignore;
		}

		std::size_t Type() const override {
			return leo::type_id<MeshLoadingDesc>().hash_code();
		}

		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
			co_yield mesh_desc.mesh_asset;
		}
	};


	asset::MeshAsset X::LoadMeshAsset(path const& meshpath) {
		return  std::move(*asset::SyncLoad<MeshLoadingDesc<GemoertySection>>(meshpath));
	}
}