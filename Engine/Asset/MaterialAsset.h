/*! \file Engine\Asset\MaterailAsset.h
\ingroup Engine
\brief Materail Infomation ...
*/
#ifndef LE_ASSET_MATERIAL_ASSET_H
#define LE_ASSET_MATERIAL_ASSET_H 1

#include "EffectAsset.h"
#include <LFramework/Core/LObject.h>

namespace asset {
	class MaterailAsset :leo::noncopyable {
	public:
		MaterailAsset() = default;

	public:
		struct PassInfo {
			std::pair<leo::uint8,leo::uint8> pass_used_range;
		};

		DefGetter(const lnothrow, const std::string&, EffectName, effect_name)
			DefGetter(lnothrow, std::string&, EffectNameRef, effect_name)

		DefGetter(const lnothrow, const std::vector<std::pair<size_t LPP_Comma leo::ValueObject>>&, BindValues, bind_values)
			DefGetter(lnothrow, std::vector<std::pair<size_t LPP_Comma leo::ValueObject>>&, BindValuesRef, bind_values)
	private:
		std::string effect_name;
		std::vector<std::pair<size_t, leo::ValueObject>> bind_values;

		std::string asset_path;
	};
}

#endif