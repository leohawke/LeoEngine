/*! \file Engine\Asset\EffectAsset.h
\ingroup Engine
\brief EFFECT Infomation ...
*/
#ifndef LE_ASSET_EFFECT_ASSET_H
#define LE_ASSET_EFFECT_ASSET_H 1

#include "ShaderAsset.h"

namespace asset {
	class TechniquePassAsset :public AssetName {
	public:
		using ShaderType = platform::Render::ShaderType;
		DefGetter(const lnothrow, const std::vector<ShaderMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<ShaderMacro>&, MacrosRef, macros)

			DefGetter(const lnothrow, const platform::Render::PipleState&, PipleState, piple_state)
			DefGetter(lnothrow, platform::Render::PipleState&, PipleStateRef, piple_state)

			void AssignOrInsertHash(ShaderType type, size_t  blobhash)
		{
			blobindexs.insert_or_assign(type, blobhash);
		}

		size_t GetBlobHash(ShaderType type) const {
			return blobindexs.find(type)->second;
		}

		const std::unordered_map<ShaderType, size_t>& GetBlobs() const lnothrow {
			return blobindexs;
		}
	private:
		platform::Render::PipleState piple_state;
		std::vector<ShaderMacro> macros;
		std::unordered_map<ShaderType, size_t> blobindexs;
	};

	class EffectTechniqueAsset : public AssetName {
	public:
		DefGetter(const lnothrow, const std::vector<ShaderMacro>&, Macros, macros)
			DefGetter(lnothrow, std::vector<ShaderMacro>&, MacrosRef, macros)

			DefGetter(const lnothrow, const std::vector<TechniquePassAsset>&, Passes, passes)
			DefGetter(lnothrow, std::vector<TechniquePassAsset>&, PassesRef, passes)
	private:
		std::vector<ShaderMacro> macros;
		std::vector<TechniquePassAsset> passes;
	};

	class EffectAsset :public ShadersAsset,public  AssetName, leo::noncopyable {
	public:
		EffectAsset() = default;

			DefGetter(const lnothrow, const std::vector<EffectTechniqueAsset>&, Techniques, techniques)
			DefGetter(lnothrow, std::vector<EffectTechniqueAsset>&, TechniquesRef, techniques)
	private:
		std::vector<EffectTechniqueAsset> techniques;
	};
}

#endif