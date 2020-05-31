#pragma once


namespace platform::Render {
	template<typename TCommandList,typename TShaderClass,typename THardwareShader>
	void SetShaderParameters(TCommandList& cmdlist, const TShaderClass* Shader, THardwareShader* ShaderRHI, const typename TShaderClass::Parameters& Parameters)
	{
		const auto& Bindings = Shader->Bindings;

		auto ParametersPtr = &Parameters;
		const uint8* Base = reinterpret_cast<const uint8*>(ParametersPtr);

		// Parameters
		for (auto& ParameterBinding : Bindings.Paramters)
		{
			const void* DataPtr = reinterpret_cast<const char*>(&Parameters) + ParameterBinding.ByteOffset;
			cmdlist.SetShaderParameter(ShaderRHI, ParameterBinding.BufferIndex, ParameterBinding.BaseIndex, ParameterBinding.ByteSize,DataPtr);
		}
	}
}
