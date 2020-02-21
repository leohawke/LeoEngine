#include <LFramework/LCLib/Platform.h>
#include <LFramework/Win32/LCLib/COM.h>
#include "D3DShaderCompiler.h"
#include "../Render/IContext.h"
#include "../emacro.h"

using namespace platform::Render::Shader;

#ifdef LFL_Win32

namespace platform_ex::Windows::D3D12 {
	platform::Render::ShaderInfo* ReflectDXBC(const platform::Render::ShaderBlob& blob, platform::Render::ShaderType type);
}

#include <UniversalDXSDK/d3dcompiler.h>
namespace platform::X::Shader {
	Render::ShaderBlob CompileToDXBC(Render::ShaderType type, std::string_view code,
		std::string_view entry_point, const std::vector<ShaderMacro>& macros,
		std::string_view profile, leo::uint32 flags, string_view SourceName) {
		std::vector<D3D_SHADER_MACRO> defines;
		for (auto& macro : macros) {
			D3D_SHADER_MACRO define;
			define.Name = macro.first.c_str();
			define.Definition = macro.second.c_str();
			defines.emplace_back(define);
		}
		D3D_SHADER_MACRO define_end = { nullptr, nullptr };
		defines.push_back(define_end);

		platform_ex::COMPtr<ID3DBlob> code_blob;
		platform_ex::COMPtr<ID3DBlob> error_blob;

		auto hr = D3DCompile(code.data(), code.size(), SourceName.data(), defines.data(), nullptr, entry_point.data(), profile.data(), flags, 0, &code_blob, &error_blob);
		platform_ex::CheckHResult(hr);
		if (code_blob) {
			Render::ShaderBlob blob;
			blob.first = std::make_unique<stdex::byte[]>(code_blob->GetBufferSize());
			blob.second = code_blob->GetBufferSize();
			std::memcpy(blob.first.get(), code_blob->GetBufferPointer(), blob.second);
			return std::move(blob);
		}
		auto error = reinterpret_cast<char*>(error_blob->GetBufferPointer());
		LE_LogError(error);

		return {};
	}

	Render::ShaderInfo* ReflectDXBC(const Render::ShaderBlob& blob, Render::ShaderType type)
	{
		using namespace Render;
		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			return platform_ex::Windows::D3D12::ReflectDXBC(blob, type);
		}

		return nullptr;
	}

	Render::ShaderBlob StripDXBC(const Render::ShaderBlob& code_blob, leo::uint32 flags) {
		platform_ex::COMPtr<ID3DBlob> stripped_blob;
		platform_ex::CheckHResult(D3DStripShader(code_blob.first.get(), code_blob.second, flags, &stripped_blob));
		Render::ShaderBlob blob;
		blob.first = std::make_unique<stdex::byte[]>(stripped_blob->GetBufferSize());
		blob.second = stripped_blob->GetBufferSize();
		std::memcpy(blob.first.get(), stripped_blob->GetBufferPointer(), blob.second);
		return std::move(blob);
	}
}

#else
//TODO CryEngine HLSLCross Compiler?
//Other Target Platfom Compiler [Tool...]
#endif