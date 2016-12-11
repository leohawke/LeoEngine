/*! \file Engine\Asset\EffectX.h
\ingroup Engine
\brief EFFECT IO ...
*/
#ifndef LE_ASSET_EFFECT_X_H
#define LE_ASSET_EFFECT_X_H 1


#include "EffectAsset.h"
#include <experimental/filesystem>

namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		asset::EffectAsset LoadEffectAsset(path const& effectpath);
	}
}

#endif