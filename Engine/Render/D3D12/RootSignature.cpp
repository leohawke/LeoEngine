#include "RootSignature.h"
#include "../../Core/Hash/CityHash.h"

using namespace platform_ex::Windows::D3D12;

void RootSignature::Init(const ShaderBlob& blob)
{
}

void platform_ex::Windows::D3D12::RootSignature::Init(const QuantizedBoundShaderState& QBSS)
{
}

leo::uint32 QuantizedBoundShaderState::GetHashCode() const
{
	return CityHash64((const char*)this,sizeof(*this));
}

RootSignature* platform_ex::Windows::D3D12::RootSignatureMap::GetRootSignature(const QuantizedBoundShaderState& QBSS)
{
	std::lock_guard lock{ mutex };

	auto iter = Map.find(QBSS);
	if (iter == Map.end())
	{
		return CreateRootSignature(QBSS);
	}

	return iter->second.get();
}

RootSignature* platform_ex::Windows::D3D12::RootSignatureMap::CreateRootSignature(const QuantizedBoundShaderState& QBSS)
{
	auto pair = Map.emplace(QBSS, std::make_unique<RootSignature>(QBSS));

	return pair.first->second.get();
}
