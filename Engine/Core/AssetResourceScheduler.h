/*! \file Core\Camera.h
\ingroup Engine
\brief �ṩ�ʲ�(Asset)�Ĺ�������Դ��ʵ����(Resource)��
*/

#ifndef LE_Core_AssetResourceScheduler_h
#define LE_Core_AssetResourceScheduler_h 1

#include <LBase/cache.hpp>
#include "../Asset/Loader.hpp"

namespace platform {
	/*! \brief ��Ҫְ���Ǹ���ַ�����TaskScheduler(TODO),Ҳ�����ӳ���Դ����������
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
			/*!\breif loaded_tick ��delay_tick����Loaded���֮��ˢ��
			   \warning SpawnResource ����ˢ�����״̬
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