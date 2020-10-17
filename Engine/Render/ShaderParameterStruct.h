#pragma once


namespace platform::Render {
	template<typename TCommandList,typename TShaderClass,typename THardwareShader>
	inline void SetShaderParameters(TCommandList& cmdlist, const TShaderClass* Shader, THardwareShader* ShaderRHI, const typename TShaderClass::Parameters& Parameters)
	{
		//ValidateShaderParameters

		lconstraint(ShaderRHI != nullptr);
		const auto& Bindings = Shader->Bindings;

		auto ParametersPtr = &Parameters;
		const uint8* Base = reinterpret_cast<const uint8*>(ParametersPtr);

		// Parameters
		for (auto& ParameterBinding : Bindings.Paramters)
		{
			const void* DataPtr = Base + ParameterBinding.ByteOffset;
			cmdlist.SetShaderParameter(ShaderRHI, ParameterBinding.BufferIndex, ParameterBinding.BaseIndex, ParameterBinding.ByteSize,DataPtr);
		}

		// Textures
		for (auto& TextureBinding : Bindings.Textures)
		{
			auto ShaderParameterRef = *(Texture**)(Base + TextureBinding.ByteOffset);

			cmdlist.SetShaderTexture(ShaderRHI, TextureBinding.BaseIndex, ShaderParameterRef);
		}

		//Samplers
		for (auto& SamplerBinding : Bindings.Samplers)
		{
			auto ShaderParameterRef = *(TextureSampleDesc*)(Base + SamplerBinding.ByteOffset);

			cmdlist.SetShaderSampler(ShaderRHI, SamplerBinding.BaseIndex, ShaderParameterRef);
		}

		if constexpr (std::is_same_v<THardwareShader, ComputeHWShader>)
		{
			//UAVS
			for (auto& UAVBinding : Bindings.UAVs)
			{
				auto ShaderParameterRef = *(UnorderedAccessView**)(Base + UAVBinding.ByteOffset);

				cmdlist.SetUAVParameter(ShaderRHI, UAVBinding.BaseIndex, ShaderParameterRef);
			}
		}
	}
}
