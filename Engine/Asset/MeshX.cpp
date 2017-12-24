#include "MeshX.h"
#include "Loader.hpp"
#include<LBase/typeinfo.h>

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

	struct GeometrySection {
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