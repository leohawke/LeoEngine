#pragma once

#include "../IDevice.h"
#include "Utility.h"

namespace platform_ex::Windows::D3D12 {
	class D3D12HardwareShader
	{
	public:
		D3D12HardwareShader(const platform::Render::ShaderInitializer& initializer);

		const ShaderBytecodeHash& GetHash() const { return Hash; }

		platform::Render::ShaderBlob ShaderByteCode;
		platform::Render::ShaderCodeResourceCounts ResourceCounts;
		bool bGlobalUniformBufferUsed = false;

	private:
		void HashShader()
		{
			if (ShaderByteCode.first && ShaderByteCode.second > 0)
			{
				// D3D shader bytecode contains a 128bit checksum in DWORD 1-4. We can just use that directly instead of hashing the whole shader bytecode ourselves.
				lconstraint(ShaderByteCode.second >= sizeof(uint32) + sizeof(Hash));
				std::memcpy(&Hash, ((uint32*)ShaderByteCode.first.get()) + 1, sizeof(Hash));
			}
			else
			{
				std::memset(&Hash,0,sizeof(Hash));
			}
		}
	private:
		ShaderBytecodeHash Hash;
	};

	class VertexHWShader :public platform::Render::VertexHWShader,public D3D12HardwareShader
	{
	public:
		using D3D12HardwareShader::D3D12HardwareShader;
	};

	class PixelHWShader :public platform::Render::PixelHWShader, public D3D12HardwareShader
	{
	public:
		using D3D12HardwareShader::D3D12HardwareShader;
	};

	class GeometryHWShader : public platform::Render::GeometryHWShader, public D3D12HardwareShader
	{
	public:
		using D3D12HardwareShader::D3D12HardwareShader;
	};

	class DomainHWShader : public platform::Render::HardwareShader, public D3D12HardwareShader
	{
	public:
		using D3D12HardwareShader::D3D12HardwareShader;
	};

	class HullHWShader : public platform::Render::HardwareShader, public D3D12HardwareShader
	{
	public:
		using D3D12HardwareShader::D3D12HardwareShader;
	};
}
