/*! \file Engine\Asset\EffectX.h
\ingroup Engine
\brief EFFECT IO/Infomation ...
*/
#ifndef LE_ASSET_EFFECT_X_H
#define LE_ASSET_EFFECT_X_H 1

#include <string>
#include <experimental/filesystem>

namespace asset {
	class EffectAsset {
	public:
#ifdef ENGINE_TOOL
	public:
		std::string GetHLSLShader() const;
	private:
		std::string hlsl_shader;
#endif
	};
}

namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		asset::EffectAsset LoadEffectAsset(path const& effectpath);
	}
}

#endif