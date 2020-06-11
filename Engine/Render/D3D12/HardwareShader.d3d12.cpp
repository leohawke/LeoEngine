#include "HardwareShader.h"

using namespace platform_ex::Windows::D3D12;

D3D12HardwareShader::D3D12HardwareShader(const platform::Render::ShaderInitializer& initializer)
{
	auto& blob = *initializer.pBlob;
	ShaderByteCode.first = std::make_unique<byte[]>(blob.second);
	ShaderByteCode.second = blob.second;
	std::memcpy(ShaderByteCode.first.get(), blob.first.get(), blob.second);

	for (auto& cb : initializer.pInfo->ConstantBufferInfos)
	{
		if (cb.name == "$Globals")
		{
			bGlobalUniformBufferUsed = true;
			break;
		}
	}
}
