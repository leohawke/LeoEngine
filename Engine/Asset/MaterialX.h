/*! \file Engine\Asset\Mesh.h
\ingroup Engine
\brief Material IO ...
*/
#ifndef LE_ASSET_MATERIAL_X_H
#define LE_ASSET_MATERIAL_X_H 1

#include "EffectX.h"
#include "MaterialAsset.h"
namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		std::shared_ptr<asset::MaterailAsset> LoadMaterialAsset(path const& materialpath);
	}
}

#endif