/*! \file Core\Material.h
\ingroup Engine
\brief 提供渲染所需的Effect,Pass使用。
*/
#ifndef LE_Core_Mesh_H
#define LE_Core_Mesh_H 1

#include "Resource.h"
#include "../Asset/MaterialAsset.h"

namespace platform {

	class Material :Resources {
	public:
		Material(const asset::MaterailAsset& asset, const std::string& name);
	}

#endif