/*! \file Engine\Asset\EffectX.h
\ingroup Engine
\brief EFFECT IO ...
*/
#ifndef LE_ASSET_EFFECT_X_H
#define LE_ASSET_EFFECT_X_H 1


#include "EffectAsset.h"
#include "../Render/Effect/Effect.hpp"
#include <filesystem>
#include <string_view>
namespace platform {
	namespace X {
		using path = std::filesystem::path;

		std::shared_ptr<asset::EffectAsset> LoadEffectAsset(path const& effectpath);
		std::shared_ptr<Render::Effect::Effect> LoadEffect(std::string const& name);

		namespace Shader {
			Render::ShaderBlob CompileToDXBC(Render::ShaderType type, std::string_view Code,
				std::string_view entry_point,const std::vector<asset::EffectMacro>& macros,
				std::string_view profile, leo::uint32 flags,std::string_view SourceName);
			Render::ShaderInfo* ReflectDXBC(const Render::ShaderBlob& blob, Render::ShaderType type);
			Render::ShaderBlob StripDXBC(const Render::ShaderBlob& blob, leo::uint32 flags);
		}
	}
}

#endif