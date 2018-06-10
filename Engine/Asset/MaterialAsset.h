/*! \file Engine\Asset\MaterailAsset.h
\ingroup Engine
\brief Materail Infomation ...
*/
#ifndef LE_ASSET_MATERIAL_ASSET_H
#define LE_ASSET_MATERIAL_ASSET_H 1

#include "EffectAsset.h"

namespace asset {
	class MaterailAsset :leo::noncopyable {
	public:
		MaterailAsset() = default;

	public:
		struct PassInfo {
			std::pair<leo::uint8,leo::uint8> pass_used_range;
		};

		DefGetter(const lnothrow, const std::string&, EffectName, effect_name)
			DefGetter(lnothrow, std::string&, EffectName, effect_name)

	private:
		std::string effect_name;
		std::vector<std::pair<size_t, leo::any>> bind_values;
	};
}

#endif