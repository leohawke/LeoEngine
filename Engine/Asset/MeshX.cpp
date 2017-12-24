#include "MeshX.h"
#include "Loader.hpp"
#include<LBase/typeinfo.h>

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