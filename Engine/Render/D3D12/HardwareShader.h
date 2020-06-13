#pragma once

#include "../IDevice.h"

namespace platform_ex::Windows::D3D12 {
	class D3D12HardwareShader
	{
	public:
		D3D12HardwareShader(const platform::Render::ShaderInitializer& initializer);

		platform::Render::ShaderBlob ShaderByteCode;
		platform::Render::ShaderCodeResourceCounts ResourceCounts;
		bool bGlobalUniformBufferUsed = false;
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
