#include "MeshX.h"
#include "Loader.hpp"
#include<LBase/typeinfo.h>
#include "../Core/LFile.h"
namespace platform {
	//Mesh�ļ���ʽ
	struct MeshHeader
	{
		//TODO Asset Common Header?
		leo::uint32 Signature; //Magic Number == 'MESH'
		leo::uint16 Machine;//��Դ������ƽ̨��ʽ
		leo::uint16 NumberOfSections;//��Ҫ����Ƹ���,SectionLoader������س���Ӧ����Դ,Ŀǰֻʵ��һ��Section
		union {
			leo::uint16 SizeOfOptional;//���뱣֤���� %16 == 0
			leo::uint16 FirstSectionOffset;//��ƫ��Ϊ����ļ���ƫ��,ע��һ������ж���
		};
	};

	//ÿ��Section��ͨ��ͷ
	struct SectionCommonHeader {
		leo::uint16 SectionIndex;//���ǵ�ǰ�ĵڼ���
		leo::uint32 NextSectionOffset;//��һ�ڿ�ʼ����ļ���ƫ��
		stdex::byte Name[8];//ʹ�����־����ڼ�����
		leo::uint32 Size;//���ڵĴ�С,��ζ�Ž���������С���Ƶ�,ע���ǰ�������С��
	};

	lconstexpr leo::uint32 GeomertyCurrentVersion = 0;

	struct GeomertySectionHeader :SectionCommonHeader {
		leo::uint32 FourCC; //'LEME'
		leo::uint32 Version;//����֧��
		leo::uint32 CompressVersion;//����֧��,���ܻ���������ѹ��
		union {
			leo::uint16 SizeOfOptional;//���뱣֤����
			leo::uint16 GeomertyOffset;//��ƫ��Ϊ����ļ���ƫ��,ע��һ������ж���
		};//����֧�֣�ֵӦ����0
	};


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

	class GemoertySection :MeshSectionLoading {
	public:
		using MeshSectionLoading::MeshSectionLoading;
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

			leo::uint8 VertexElmentsCount = 0;
			file.Read(&VertexElmentsCount, 1);
			auto & vertex_elements = mesh_asset->GetVertexElementsRef();
			vertex_elements.reserve(VertexElmentsCount);
			for (auto i = 0; i != VertexElmentsCount; ++i) {
				vertex_elements.emplace_back(file.Read<Render::Vertex::Element>());
			}
		}
	};

	class MeshLoadingDesc : public asset::AssetLoading<asset::MeshAsset> {
	private:
		struct MeshDesc {
			X::path mesh_path;
			std::shared_ptr<AssetType> mesh_asset;
		} mesh_desc;
	public:
		explicit MeshLoadingDesc(X::path const & meshpath)
		{
			mesh_desc.mesh_path = meshpath;
		}

		std::size_t Type() const override {
			return leo::type_id<MeshLoadingDesc>().hash_code();
		}

		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
			co_yield mesh_desc.mesh_asset;
		}
	};


	asset::MeshAsset X::LoadMeshAsset(path const& meshpath) {
		return  std::move(*asset::SyncLoad<MeshLoadingDesc>(meshpath));
	}
}