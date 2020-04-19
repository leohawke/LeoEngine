#pragma once
#include "Shader.h"
#include "ShaderParametersMetadata.h"
#include <LBase/cformat.h>

using namespace platform::Render::Shader;
using namespace platform::Render;

struct ShaderParameterStructBinding
{
	const RenderShader* Shader;

	RenderShaderParameterBindings* Bindings;

	const ShaderParameterMap* ParametersMap;

	void Bind(const ShaderParametersMetadata& StructMetaData)
	{
		auto& StructMembers = StructMetaData.GetMembers();

		for (auto& Member : StructMembers)
		{
			auto ShaderBindingName = leo::sfmt("%s", Member.GetName());

			auto ByteOffset = static_cast<uint16>(Member.GetOffset());

			auto ShaderType = Member.GetShaderType();

			const bool bIsVariableNativeType = (
				ShaderType == SPT_float ||
				ShaderType == SPT_float2 ||
				ShaderType == SPT_float3 ||
				ShaderType == SPT_float4
				);

			uint16 BufferIndex, BaseIndex, BoundSize;
			if (!ParametersMap->FindParameterAllocation(ShaderBindingName, BufferIndex, BaseIndex, BoundSize))
			{
				continue;
			}

			if (bIsVariableNativeType)
			{
				auto ByteSize = Member.GetMemberSize();

				RenderShaderParameterBindings::Parameter Parameter;
				Parameter.BufferIndex = BufferIndex;
				Parameter.BaseIndex = BaseIndex;
				Parameter.ByteOffset = ByteOffset;
				Parameter.ByteSize = BoundSize;

				Bindings->Paramters.emplace_back(Parameter);
			}
		}
	}
};

void RenderShaderParameterBindings::BindForLegacyShaderParameters(const RenderShader* Shader, const ShaderParameterMap& ParameterMaps, const ShaderParametersMetadata& StructMetaData)
{
	ShaderParameterStructBinding Binding;
	Binding.Shader = Shader;
	Binding.Bindings = this;
	Binding.ParametersMap = &ParameterMaps;

	Binding.Bind(StructMetaData);
}