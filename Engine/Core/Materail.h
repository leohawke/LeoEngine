/*! \file Core\Material.h
\ingroup Engine
\brief 提供渲染所需的Effect,Pass使用。
*/
#ifndef LE_Core_Mesh_H
#define LE_Core_Mesh_H 1

#include "LSLEvaluator.h"
#include "Resource.h"
#include "../Asset/MaterialAsset.h"

namespace platform {

	class Material :Resource {
	public:
		Material(const asset::MaterailAsset& asset, const std::string& name);
	};

	class MaterialEvaluator :public LSLEvaluator {
	public:
		MaterialEvaluator();

		std::pair<scheme::TermNode, scheme::ReductionStatus> Reduce(const scheme::TermNode& input) {
			auto term(input);
			context.Prepare(term);
			auto status(scheme::v1::Reduce(term, context.Root));

			return std::make_pair(term, status);
		}

		void RegisterMathDotLssFile();

		struct InstanceTag :scheme::LSLATag
		{};

		struct RenderTag : scheme::LSLATag
		{};

		using InstanceDelayedTerm = leo::derived_entity<scheme::TermNode, InstanceTag>;
		using RenderDelayedTerm = leo::derived_entity<scheme::TermNode, RenderTag>;
	private:
		static void MaterialEvalFunctions(REPLContext& context);
	};
}

#endif