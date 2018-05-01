#include "AssetResourceScheduler.h"

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
}
