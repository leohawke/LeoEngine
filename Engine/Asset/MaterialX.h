/*! \file Engine\Asset\Mesh.h
\ingroup Engine
\brief Material IO ...
*/
#ifndef LE_ASSET_MATERIAL_X_H
#define LE_ASSET_MATERIAL_X_H 1

#include "EffectX.h"
#include "MaterialAsset.h"
#include "../Core/Materail.h"
namespace platform {
	namespace X {
		using path = std::filesystem::path;

		std::shared_ptr<asset::MaterailAsset> LoadMaterialAsset(path const& materialpath);

		std::shared_ptr<Material> LoadMaterial(path const& materialpath, const std::string& name);
	}
}

#endif