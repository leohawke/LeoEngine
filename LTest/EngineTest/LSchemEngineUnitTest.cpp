#include "LSchemEngineUnitTest.h"
#include <LScheme/Configuration.h>
#include <sstream>
#include <LBase/Debug.h>

using namespace platform::Descriptions;

void ParseEffectNodeV1(const leo::ValueNode&);

void unit_test::ExceuteLSchemEngineUnitTest()
{
	auto ClearRenderTargetv1 = R"(
		(effect
			(macro (@ (name "VERSION")) "v1")
			(cbuffer (@ (name "per_frame"))
			(parameter (@ (type "float4") (name "color")))
			)
			(shader
				"
				float4 ClearVS(float4 position : POSITION) :SV_POSITION
				{
					return position;
				}
				float4 ClearPS() : SV_TARGET
				{
					return color;
				}
				"
			)
			(technique (@ (name "ClearRT"))
				(pass (@ (name "p0"))
					(state (@ (name "vertex_shader") (value "ClearVS")))
					(state (@ (name "pixel_shader") (value "ClearPS")))
				)
			)
		)
	)";

	std::stringstream ss(ClearRenderTargetv1);
	scheme::Configuration config;

	ss >> config;

	LAssert(!config.GetNodeRRef().empty(), "Empty configuration found.");
	ParseEffectNodeV1(config.GetNodeRRef());
}

void ParseMacroNodev1(const leo::ValueNode& node);

void ParseEffectNodeV1(const leo::ValueNode& node)
{
	LAssert(node.GetName() == "effect", R"(Invalid Format:Not Begin With "effect")");

	auto macros = node.SelectChildren([](const leo::ValueNode& node) {
		return node == "macro";
	});
	for (auto& macro : macros) {
		TraceDe(Debug, "(macro\t");
		ParseMacroNodev1(macro);
		TraceDe(Debug, ")\t");
	}
}

void ParseAtNode(const leo::ValueNode& node)
{
	try {
		if (auto pname = leo::AccessChildPtr<std::string>(node, "name"))
			TraceDe(Debug, "(name %s)\t", pname->c_str());
		if (auto ptype = leo::AccessChildPtr<std::string>(node, "type"))
			TraceDe(Debug, "(type %s)\t", ptype->c_str());
		
	}
	CatchExpr(leo::bad_any_cast&,LAssert(false,R"(@ Invalid Format:Type Match failed)"))
}

void ParseMacroNodev1(const leo::ValueNode& node) {
	TraceDe(Debug, "(@\t");
	ParseAtNode(leo::AccessNode(node, "@"));
	TraceDe(Debug, ")\t");
	auto values = node.SelectChildren([](const leo::ValueNode& node) {
		return leo::IsPrefixedIndex(node.GetName());
	});
	std::for_each(values.begin(), values.end(), [](const leo::ValueNode& node) {
		if (auto pvalue = leo::AccessPtr<std::string>(node)) {
			TraceDe(Debug, "%s\t", pvalue->c_str());
		}
	});
}


