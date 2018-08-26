#include "AssetResourceScheduler.h"
#include <LFramework/Core/LException.h>

namespace platform {

	ImplDeCtor(AssetResourceScheduler)

	AssetResourceScheduler::~AssetResourceScheduler()
	{
		//explicit clear
		asset_loaded_caches.clear();
	}

	AssetResourceScheduler & AssetResourceScheduler::Instance()
	{
		static AssetResourceScheduler instance;
		return instance;
	}
	const asset::path& AssetResourceScheduler::FindAssetPath(const void * pAsset)
	{
		for (auto &pair : asset_loaded_caches) {
			if (pair.second.loaded_asset.get() == pAsset)
				return pair.first->Path();
		}
		throw leo::GeneralEvent(leo::sfmt("Can't Find Asset's loader,so can't retrieve path pAsset=%p", pAsset));
	}
}
