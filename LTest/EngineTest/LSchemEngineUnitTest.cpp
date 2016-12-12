#include "LSchemEngineUnitTest.h"
#include <LScheme/Configuration.h>
#include <sstream>
#include <LBase/Debug.h>

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

	TryExpr(ParseMacroNodev1(leo::AccessNode(node, "macro")))
	CatchExpr(std::out_of_range&, LAssert(false, R"(Invalid Format:Find "macro" failed)"))
}

void ParseAtNode(const leo::ValueNode& node)
{
	try {
		if (auto pname = leo::AccessNodePtr(node, "name"))
			pname->Value.Access<std::string>();
		if (auto ptype = leo::AccessNodePtr(node, "type"))
			ptype->Value.Access<std::string>();
	}
	CatchExpr(leo::bad_any_cast&,LAssert(false,R"(@ Invalid Format:Type Match failed)"))
}

void ParseMacroNodev1(const leo::ValueNode& node) {
	ParseAtNode(leo::AccessNode(node, "@"));
}


