/*! \file Core\AssetResourceScheduler.h
\ingroup Engine
\brief �ṩ�ʲ�(Asset)�Ĺ�������Դ��ʵ����(Resource)��
*/

#ifndef LE_Core_AssetResourceScheduler_h
#define LE_Core_AssetResourceScheduler_h 1

#include <LBase/cache.hpp>
#include "../Asset/Loader.hpp"

namespace platform {

	/*! \brief ��Ҫְ���Ǹ���ַ�����TaskScheduler(TODO),Ҳ�����ӳ��ʲ�����������
	*/
	class AssetResourceScheduler :leo::nonmovable, leo::noncopyable {
	public:
		template<typename Loading, typename... _tParams>
		std::shared_ptr<typename Loading::AssetType> SyncLoad(_tParams&&... args) {
			std::shared_ptr<asset::IAssetLoading> key{ new Loading(lforward(args)...) };

			auto itr = asset_loaded_caches.find(key);
			if (itr != asset_loaded_caches.end()) {
				//TODO Refresh Ticks
				return std::static_pointer_cast<typename Loading::AssetType>(itr->second.loaded_asset);
			}

			std::shared_ptr<Loading> loading = std::static_pointer_cast<Loading>(key);
			std::shared_ptr<typename Loading::AssetType> ret{};
			auto coroutine = loading->Coroutine();
			for (auto yiled : coroutine)
				ret = yiled;

			AssetLoadedDesc desc;
			desc.loaded_asset = std::static_pointer_cast<void>(ret);
			desc.loaded_tick = 0;
			desc.delay_tick = 0;
			asset_loaded_caches.emplace(key, desc);
			return ret;
		}

		template<typename _type, typename... _tParams>
		std::shared_ptr<_type> SyncSpawnResource(_tParams&&... args);

		static AssetResourceScheduler& Instance();
	private:
		AssetResourceScheduler();
		~AssetResourceScheduler();

	private:
		struct AssetLoadedDesc {
			//@{
			/*!\breif loaded_tick ��delay_tick����Loaded���֮��ˢ��
			   \warning SpawnResource ����ˢ�����״̬
			*/
			leo::uint32 loaded_tick;
			leo::uint8 delay_tick;
			//@}
			std::shared_ptr<void> loaded_asset;
		};

		struct IAssetLoadingHash {
			std::size_t operator()(const std::shared_ptr<asset::IAssetLoading>& iasset) const lnoexcept {
				return iasset->Hash();
			}
		};

		struct IAssetLoadingEqual {
			bool operator()(const std::shared_ptr<asset::IAssetLoading>& lhs, const std::shared_ptr<asset::IAssetLoading>& rhs) const lnoexcept {
				return lhs->Hash() == rhs->Hash();
			}
		};

		//todo thread safe
		leo::used_list_cache<std::shared_ptr<asset::IAssetLoading>, AssetLoadedDesc, IAssetLoadingEqual,IAssetLoadingHash> asset_loaded_caches;
	};
}

#endif