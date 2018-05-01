/*! \file Core\Camera.h
\ingroup Engine
\brief 提供资产(Asset)的管理与资源的实例化(Resource)。
*/

#ifndef LE_Core_AssetResourceScheduler_h
#define LE_Core_AssetResourceScheduler_h 1

#include <LBase/cache.hpp>
#include "../Asset/Loader.hpp"

namespace platform {
	/*! \brief 主要职责是负责分发任务到TaskScheduler(TODO),也负责延迟资源的生命周期
	*/
	class AssetResourceScheduler :leo::nonmovable, leo::noncopyable {
	public:
		template<typename Loading, typename... _tParams>
		std::shared_ptr<typename Loading::AssetType> SyncLoad(_tParams&&... args) {
			auto loading = std::make_unique<Loading>(lforward(args)...);

			/*TODO 
			auto task = TaskScheduler::Instance().CreateTask(loading->Coroutine());
			auto ret = task.Wait();
			*/
			auto coroutine = loading->Coroutine();
			auto iter = coroutine.begin();
			while (!*iter)
				++iter;

			return *iter;
		}

		template<typename _type, typename... _tParams>
		std::shared_ptr<_type> SpawnResource(_tParams&&... args);

		static AssetResourceScheduler& Instance();
	private:
		AssetResourceScheduler();
		~AssetResourceScheduler();

	private:
		struct AssetLoadedDesc {
			//@{
			/*!\breif loaded_tick 和delay_tick会在Loaded完成之后被刷新
			   \warning SpawnResource 不会刷新这个状态
			*/
			leo::uint32 loaded_tick;
			leo::uint8 delay_tick;
			//@}
			std::shared_ptr<void> loaded_asset;
		};

		//todo thread safe
		leo::used_list_cache<std::unique_ptr<asset::IAssetLoading>, AssetLoadedDesc,std::hash<std::unique_ptr<asset::IAssetLoading>>> asset_loaded_caches;
	};
}

#endif