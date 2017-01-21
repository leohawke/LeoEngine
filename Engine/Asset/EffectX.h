/*! \file Engine\Asset\EffectX.h
\ingroup Engine
\brief EFFECT IO ...
*/
#ifndef LE_ASSET_EFFECT_X_H
#define LE_ASSET_EFFECT_X_H 1


#include "EffectAsset.h"
#include <experimental/filesystem>
#include <string_view>
namespace platform {
	namespace X {
		using path = std::experimental::filesystem::path;

		asset::EffectAsset LoadEffectAsset(path const& effectpath);

		namespace Shader {
			Render::ShaderCompose::ShaderBlob CompileToDXBC(Render::ShaderCompose::Type type, std::string_view Code,
				std::string_view entry_point,const std::vector<asset::EffectMacro>& macros,
				std::string_view profile, leo::uint32 flags);
			void ReflectDXBC(const Render::ShaderCompose::ShaderBlob& blob);
			Render::ShaderCompose::ShaderBlob StripDXBC(const Render::ShaderCompose::ShaderBlob& blob, leo::uint32 flags);
		}
	}
}

#endif