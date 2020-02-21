#include <LFramework/LCLib/Platform.h>
#include <LFramework/Win32/LCLib/COM.h>
#include <LFramework/Core/LString.h>
#include "D3DShaderCompiler.h"
#include "../Render/IContext.h"
#include "../emacro.h"

#include <algorithm>

using namespace platform::Render::Shader;
using namespace platform_ex;
using namespace D3DFlags;

#define SHADER_OPTIMIZATION_LEVEL_MASK (D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_OPTIMIZATION_LEVEL1 | D3DCOMPILE_OPTIMIZATION_LEVEL2 | D3DCOMPILE_OPTIMIZATION_LEVEL3)
#define RAY_TRACING_REGISTER_SPACE_LOCAL  0
#define RAY_TRACING_REGISTER_SPACE_GLOBAL 1
#define RAY_TRACING_REGISTER_SPACE_SYSTEM 2

namespace asset::X::Shader {
	std::vector<ShaderMacro> AppendCompileMacros(const std::vector<ShaderMacro>& macros, ShaderType type)
	{
		using namespace  platform::Render;

		auto append_macros = macros;
		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			append_macros.emplace_back("D3D12", "1");
			//TODO depend feature_level
			append_macros.emplace_back("SM_VERSION", "50");
			break;
		}
		switch (type) {
		case ShaderType::VertexShader:
			append_macros.emplace_back("VS", "1");
			break;
		case ShaderType::PixelShader:
			append_macros.emplace_back("PS", "1");
			break;
		}
		return append_macros;
	}

	std::string_view CompileProfile(ShaderType type)
	{
		using namespace  platform::Render;

		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			switch (type) {
			case ShaderType::VertexShader:
				return "vs_5_0";
			case ShaderType::PixelShader:
				return "ps_5_0";
			case ShaderType::RayGen:
			case ShaderType::RayMiss:
			case ShaderType::RayHitGroup:
			case ShaderType::RayCallable:
				return "lib_6_3";
			}
		}
		return "";
	}
}



#ifdef LFL_Win32

namespace platform_ex::Windows::D3D12 {
	platform::Render::ShaderInfo* ReflectDXBC(const platform::Render::ShaderBlob& blob, platform::Render::ShaderType type);
}

#include <UniversalDXSDK/d3dcompiler.h>
#ifdef LFL_Win64
#pragma comment(lib,"UniversalDXSDK/Lib/x64/d3dcompiler.lib")
#else
#pragma comment(lib,"UniversalDXSDK/Lib/x86/d3dcompiler.lib")
#endif
#include <dxc/Support/dxcapi.use.h>

void FillD3D12Reflect(ID3D12ShaderReflection* pReflection, ShaderInfo* pInfo,ShaderType type)
{
	D3D12_SHADER_DESC desc;
	pReflection->GetDesc(&desc);

	for (UINT i = 0; i != desc.ConstantBuffers; ++i) {
		auto pReflectionConstantBuffer = pReflection->GetConstantBufferByIndex(i);

		D3D12_SHADER_BUFFER_DESC buffer_desc;
		pReflectionConstantBuffer->GetDesc(&buffer_desc);
		if ((D3D_CT_CBUFFER == buffer_desc.Type) || (D3D_CT_TBUFFER == buffer_desc.Type)) {
			ShaderInfo::ConstantBufferInfo  ConstantBufferInfo;
			ConstantBufferInfo.name = buffer_desc.Name;
			ConstantBufferInfo.name_hash = leo::constfn_hash(buffer_desc.Name);
			ConstantBufferInfo.size = buffer_desc.Size;

			for (UINT v = 0; v != buffer_desc.Variables; ++v) {
				auto pReflectionVar = pReflectionConstantBuffer->GetVariableByIndex(v);
				D3D12_SHADER_VARIABLE_DESC variable_desc;
				pReflectionVar->GetDesc(&variable_desc);

				D3D12_SHADER_TYPE_DESC type_desc;
				pReflectionVar->GetType()->GetDesc(&type_desc);

				ShaderInfo::ConstantBufferInfo::VariableInfo VariableInfo;
				VariableInfo.name = variable_desc.Name;
				VariableInfo.start_offset = variable_desc.StartOffset;
				VariableInfo.type = variable_desc.StartOffset;
				VariableInfo.rows = variable_desc.StartOffset;
				VariableInfo.columns = variable_desc.StartOffset;
				VariableInfo.elements = variable_desc.StartOffset;

				ConstantBufferInfo.var_desc.emplace_back(std::move(VariableInfo));
			}
			pInfo->ConstantBufferInfos.emplace_back(std::move(ConstantBufferInfo));
		}
	}

	for (UINT i = 0; i != desc.BoundResources; ++i) {
		D3D12_SHADER_INPUT_BIND_DESC input_bind_desc;
		pReflection->GetResourceBindingDesc(i, &input_bind_desc);

		auto BindPoint = static_cast<leo::uint16>(input_bind_desc.BindPoint + 1);
		switch (input_bind_desc.Type)
		{
		case D3D_SIT_SAMPLER:
			pInfo->ResourceCounts.NumSamplers = std::max(pInfo->ResourceCounts.NumSamplers, BindPoint);
			break;

		case D3D_SIT_TEXTURE:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			pInfo->ResourceCounts.NumSRVs = std::max(pInfo->ResourceCounts.NumSRVs, BindPoint);
			break;

		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			pInfo->ResourceCounts.NumUAVs = std::max(pInfo->ResourceCounts.NumUAVs, BindPoint);
			break;

		default:
			break;
		}

		switch (input_bind_desc.Type)
		{
		case D3D_SIT_TEXTURE:
		case D3D_SIT_SAMPLER:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		{
			ShaderInfo::BoundResourceInfo BoundResourceInfo;
			BoundResourceInfo.name = input_bind_desc.Name;
			BoundResourceInfo.type = static_cast<uint8_t>(input_bind_desc.Type);
			BoundResourceInfo.bind_point = static_cast<uint16_t>(input_bind_desc.BindPoint);
			pInfo->BoundResourceInfos.emplace_back(std::move(BoundResourceInfo));
		}
		break;

		default:
			break;
		}
	}

	if (type == ShaderType::VertexShader) {
		union {
			D3D12_SIGNATURE_PARAMETER_DESC signature_desc;
			stdex::byte signature_data[sizeof(D3D12_SIGNATURE_PARAMETER_DESC)];
		} s2d;

		size_t signature = 0;
		for (UINT i = 0; i != desc.InputParameters; ++i) {
			pReflection->GetInputParameterDesc(i, &s2d.signature_desc);
			auto seed = leo::hash(s2d.signature_data);
			leo::hash_combine(signature, seed);
		}

		pInfo->InputSignature = signature;
	}

	if (type == ShaderType::ComputeShader) {
		UINT x, y, z;
		pReflection->GetThreadGroupSize(&x, &y, &z);
		pInfo->CSBlockSize = leo::math::data_storage<leo::uint16, 3>(static_cast<leo::uint16>(x),
			static_cast<leo::uint16>(y), static_cast<leo::uint16>(z));
	}
}

void FillD3D11Reflect(ID3D11ShaderReflection* pReflection, ShaderInfo* pInfo, ShaderType type)
{
	D3D11_SHADER_DESC desc;
	pReflection->GetDesc(&desc);

	for (UINT i = 0; i != desc.ConstantBuffers; ++i) {
		auto pReflectionConstantBuffer = pReflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC buffer_desc;
		pReflectionConstantBuffer->GetDesc(&buffer_desc);
		if ((D3D_CT_CBUFFER == buffer_desc.Type) || (D3D_CT_TBUFFER == buffer_desc.Type)) {
			ShaderInfo::ConstantBufferInfo  ConstantBufferInfo;
			ConstantBufferInfo.name = buffer_desc.Name;
			ConstantBufferInfo.name_hash = leo::constfn_hash(buffer_desc.Name);
			ConstantBufferInfo.size = buffer_desc.Size;

			for (UINT v = 0; v != buffer_desc.Variables; ++v) {
				auto pReflectionVar = pReflectionConstantBuffer->GetVariableByIndex(v);
				D3D11_SHADER_VARIABLE_DESC variable_desc;
				pReflectionVar->GetDesc(&variable_desc);

				D3D11_SHADER_TYPE_DESC type_desc;
				pReflectionVar->GetType()->GetDesc(&type_desc);

				ShaderInfo::ConstantBufferInfo::VariableInfo VariableInfo;
				VariableInfo.name = variable_desc.Name;
				VariableInfo.start_offset = variable_desc.StartOffset;
				VariableInfo.type = variable_desc.StartOffset;
				VariableInfo.rows = variable_desc.StartOffset;
				VariableInfo.columns = variable_desc.StartOffset;
				VariableInfo.elements = variable_desc.StartOffset;

				ConstantBufferInfo.var_desc.emplace_back(std::move(VariableInfo));
			}
			pInfo->ConstantBufferInfos.emplace_back(std::move(ConstantBufferInfo));
		}
	}

	for (UINT i = 0; i != desc.BoundResources; ++i) {
		D3D11_SHADER_INPUT_BIND_DESC input_bind_desc;
		pReflection->GetResourceBindingDesc(i, &input_bind_desc);

		auto BindPoint = static_cast<leo::uint16>(input_bind_desc.BindPoint + 1);
		switch (input_bind_desc.Type)
		{
		case D3D_SIT_SAMPLER:
			pInfo->ResourceCounts.NumSamplers = std::max(pInfo->ResourceCounts.NumSamplers, BindPoint);
			break;

		case D3D_SIT_TEXTURE:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			pInfo->ResourceCounts.NumSRVs = std::max(pInfo->ResourceCounts.NumSRVs, BindPoint);
			break;

		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			pInfo->ResourceCounts.NumUAVs = std::max(pInfo->ResourceCounts.NumUAVs, BindPoint);
			break;

		default:
			break;
		}

		switch (input_bind_desc.Type)
		{
		case D3D_SIT_TEXTURE:
		case D3D_SIT_SAMPLER:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
		case D3D_SIT_UAV_APPEND_STRUCTURED:
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
		{
			ShaderInfo::BoundResourceInfo BoundResourceInfo;
			BoundResourceInfo.name = input_bind_desc.Name;
			BoundResourceInfo.type = static_cast<uint8_t>(input_bind_desc.Type);
			BoundResourceInfo.bind_point = static_cast<uint16_t>(input_bind_desc.BindPoint);
			pInfo->BoundResourceInfos.emplace_back(std::move(BoundResourceInfo));
		}
		break;

		default:
			break;
		}
	}

	if (type == ShaderType::VertexShader) {
		union {
			D3D11_SIGNATURE_PARAMETER_DESC signature_desc;
			stdex::byte signature_data[sizeof(D3D12_SIGNATURE_PARAMETER_DESC)];
		} s2d;

		size_t signature = 0;
		for (UINT i = 0; i != desc.InputParameters; ++i) {
			pReflection->GetInputParameterDesc(i, &s2d.signature_desc);
			auto seed = leo::hash(s2d.signature_data);
			leo::hash_combine(signature, seed);
		}

		pInfo->InputSignature = signature;
	}

	if (type == ShaderType::ComputeShader) {
		UINT x, y, z;
		pReflection->GetThreadGroupSize(&x, &y, &z);
		pInfo->CSBlockSize = leo::math::data_storage<leo::uint16, 3>(static_cast<leo::uint16>(x),
			static_cast<leo::uint16>(y), static_cast<leo::uint16>(z));
	}
}


namespace asset::X::Shader::DXBC {
	ShaderBlob CompileToDXBC(ShaderType type, std::string_view code,
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
		if (error_blob)
		{
			auto error = reinterpret_cast<char*>(error_blob->GetBufferPointer());
			LE_LogError(error);
		}
		platform_ex::CheckHResult(hr);

		if (code_blob) {
			ShaderBlob blob;
			blob.first = std::make_unique<stdex::byte[]>(code_blob->GetBufferSize());
			blob.second = code_blob->GetBufferSize();
			std::memcpy(blob.first.get(), code_blob->GetBufferPointer(), blob.second);
			return std::move(blob);
		}

		return {};
	}

	ShaderInfo* ReflectDXBC(const ShaderBlob& blob, ShaderType type)
	{
		auto pInfo = std::make_unique<ShaderInfo>(type);
		platform_ex::COMPtr<ID3D12ShaderReflection> pReflection;
		if (SUCCEEDED(D3DReflect(blob.first.get(), blob.second, IID_ID3D12ShaderReflection, reinterpret_cast<void**>(&pReflection.GetRef()))))
		{
			FillD3D12Reflect(pReflection.Get(), pInfo.get(), type);
			return pInfo.release();
		}

		//fallback
		platform_ex::COMPtr<ID3D11ShaderReflection> pFallbackReflection;
		CheckHResult(D3DReflect(blob.first.get(), blob.second, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&pFallbackReflection.GetRef())));
		FillD3D11Reflect(pFallbackReflection.Get(), pInfo.get(), type);

		return  pInfo.release();
	}

	ShaderBlob StripDXBC(const ShaderBlob& code_blob, leo::uint32 flags) {
		platform_ex::COMPtr<ID3DBlob> stripped_blob;
		platform_ex::CheckHResult(D3DStripShader(code_blob.first.get(), code_blob.second, flags, &stripped_blob));
		ShaderBlob blob;
		blob.first = std::make_unique<stdex::byte[]>(stripped_blob->GetBufferSize());
		blob.second = stripped_blob->GetBufferSize();
		std::memcpy(blob.first.get(), stripped_blob->GetBufferPointer(), blob.second);
		return std::move(blob);
	}
}

bool IsRayTracingShader(ShaderType type)
{
	switch (type)
	{
	case platform::Render::Shader::RayGen:
	case platform::Render::Shader::RayMiss:
	case platform::Render::Shader::RayHitGroup:
	case platform::Render::Shader::RayCallable:
		return true;
	default:
		return false;
	}
}

// Parses ray tracing shader entry point specification string in one of the following formats:
// 1) Verbatim single entry point name, e.g. "MainRGS"
// 2) Complex entry point for ray tracing hit group shaders:
//      a) "closesthit=MainCHS"
//      b) "closesthit=MainCHS anyhit=MainAHS"
//      c) "closesthit=MainCHS anyhit=MainAHS intersection=MainIS"
//      d) "closesthit=MainCHS intersection=MainIS"
//    NOTE: closesthit attribute must always be provided for complex hit group entry points
static void ParseRayTracingEntryPoint(const std::string& Input, std::string& OutMain, std::string& OutAnyHit, std::string& OutIntersection)
{
	auto ParseEntry = [&Input](const char* Marker)
	{
		std::string Result;
		auto BeginIndex = Input.find(Marker);
		if (BeginIndex != std::string::npos)
		{
			auto EndIndex = Input.find(" ", BeginIndex);
			if (EndIndex == std::string::npos) EndIndex = Input.size() + 1;
			auto MarkerLen = std::strlen(Marker);
			auto Count = EndIndex - BeginIndex;
			Result = Input.substr(BeginIndex + MarkerLen, Count - MarkerLen);
		}
		return Result;
	};

	OutMain = ParseEntry("closesthit=");
	OutAnyHit = ParseEntry("anyhit=");
	OutIntersection = ParseEntry("intersection=");

	// If complex hit group entry is not specified, assume a single verbatim entry point
	if (OutMain.empty() && OutAnyHit.empty() && OutIntersection.empty())
	{
		OutMain = Input;
	}
}

namespace asset::X::Shader::DXIL {

	static dxc::DxcDllSupport& GetDxcDllHelper()
	{
		static dxc::DxcDllSupport DxcDllSupport;
		static bool DxcDllInitialized = false;
		if (!DxcDllInitialized)
		{
			CheckHResult(DxcDllSupport.Initialize());
			DxcDllInitialized = true;
		}
		return DxcDllSupport;
	}

	static void D3DCreateDXCArguments(std::vector<const WCHAR*>& OutArgs, const WCHAR* Exports, leo::uint32 CompileFlags,leo::uint32 AutoBindingSpace = ~0u)
	{
		// Static digit strings are used here as they are returned in OutArgs
		static const WCHAR* DigitStrings[] =
		{
			L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9"
		};

		if (AutoBindingSpace < leo::size(DigitStrings))
		{
			OutArgs.push_back(L"/auto-binding-space");
			OutArgs.push_back(DigitStrings[AutoBindingSpace]);
		}
		else if (AutoBindingSpace != ~0u)
		{
			LE_LogError("Unsupported register binding space %d", AutoBindingSpace);
		}

		if (Exports && *Exports)
		{
			// Ensure that only the requested functions exists in the output DXIL.
			// All other functions and their used resources must be eliminated.
			OutArgs.push_back(L"/exports");
			OutArgs.push_back(Exports);
		}

		if (CompileFlags & D3DCOMPILE_PREFER_FLOW_CONTROL)
		{
			CompileFlags &= ~D3DCOMPILE_PREFER_FLOW_CONTROL;
			OutArgs.push_back(L"/Gfp");
		}

		if (CompileFlags & D3DCOMPILE_DEBUG)
		{
			CompileFlags &= ~D3DCOMPILE_DEBUG;
			OutArgs.push_back(L"/Zi");
		}

		if (CompileFlags & D3DCOMPILE_SKIP_OPTIMIZATION)
		{
			CompileFlags &= ~D3DCOMPILE_SKIP_OPTIMIZATION;
			OutArgs.push_back(L"/Od");
		}

		if (CompileFlags & D3DCOMPILE_SKIP_VALIDATION)
		{
			CompileFlags &= ~D3DCOMPILE_SKIP_VALIDATION;
			OutArgs.push_back(L"/Vd");
		}

		if (CompileFlags & D3DCOMPILE_AVOID_FLOW_CONTROL)
		{
			CompileFlags &= ~D3DCOMPILE_AVOID_FLOW_CONTROL;
			OutArgs.push_back(L"/Gfa");
		}

		if (CompileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR)
		{
			CompileFlags &= ~D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
			OutArgs.push_back(L"/Zpr");
		}

		if (CompileFlags & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)
		{
			CompileFlags &= ~D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
			OutArgs.push_back(L"/Gec");
		}

		switch (CompileFlags & SHADER_OPTIMIZATION_LEVEL_MASK)
		{
		case D3DCOMPILE_OPTIMIZATION_LEVEL0:
			CompileFlags &= ~D3DCOMPILE_OPTIMIZATION_LEVEL0;
			OutArgs.push_back(L"/O0");
			break;

		case D3DCOMPILE_OPTIMIZATION_LEVEL1:
			CompileFlags &= ~D3DCOMPILE_OPTIMIZATION_LEVEL1;
			OutArgs.push_back(L"/O1");
			break;

		case D3DCOMPILE_OPTIMIZATION_LEVEL2:
			CompileFlags &= ~D3DCOMPILE_OPTIMIZATION_LEVEL2;
			OutArgs.push_back(L"/O2");
			break;

		case D3DCOMPILE_OPTIMIZATION_LEVEL3:
			CompileFlags &= ~D3DCOMPILE_OPTIMIZATION_LEVEL3;
			OutArgs.push_back(L"/O3");
			break;

		default:
			LE_LogError("Unknown optimization level flag");
			break;
		}

		LAssert(CompileFlags == 0, "Unhandled shader compiler flag!");
	}

	static leo::uint32 GetAutoBindingSpace(const ShaderType& Target)
	{
		switch (Target)
		{
		case ShaderType::RayGen:
		case ShaderType::RayMiss:
			return RAY_TRACING_REGISTER_SPACE_GLOBAL;
		case ShaderType::RayHitGroup:
		case ShaderType::RayCallable:
			return RAY_TRACING_REGISTER_SPACE_LOCAL;
		default:
			return 0;
		}
	}

	ShaderBlob CompileToDXIL(ShaderType type, std::string_view code,
		std::string_view entry_point, const std::vector<ShaderMacro>& macros,
		std::string_view profile, leo::uint32 flags, string_view SourceName) {
		using String = leo::Text::String;

		bool bIsRayTracingShader = IsRayTracingShader(type);

		std::string RayEntryPoint;
		std::string RayAnyHitEntryPoint;
		std::string RayIntersectionEntryPoint;
		std::string RayTracingExports;
		if (bIsRayTracingShader)
		{
			ParseRayTracingEntryPoint(std::string(entry_point), RayEntryPoint, RayAnyHitEntryPoint, RayIntersectionEntryPoint);

			RayTracingExports = RayEntryPoint;

			if (!RayAnyHitEntryPoint.empty())
			{
				RayTracingExports += ";";
				RayTracingExports += RayAnyHitEntryPoint;
			}

			if (!RayIntersectionEntryPoint.empty())
			{
				RayTracingExports += ";";
				RayTracingExports += RayIntersectionEntryPoint;
			}
		}

		std::vector<std::pair<String, String>> def_holder;
		std::vector<DxcDefine> defs;
		for (auto& macro : macros) {
			def_holder.emplace_back(String(macro.first.c_str(), macro.first.size()), String(macro.second.c_str(), macro.second.size()));
			DxcDefine define;
			define.Name = (wchar_t*)def_holder.back().first.c_str();
			define.Value = (wchar_t*)def_holder.back().second.c_str();
		}

		std::vector<const wchar_t*> args;
		D3DCreateDXCArguments(args,(wchar_t*)String(RayTracingExports).data(), flags, GetAutoBindingSpace(type));

		dxc::DxcDllSupport& DxcDllHelper = GetDxcDllHelper();

		COMPtr<IDxcCompiler> Compiler;
		DxcDllHelper.CreateInstance(CLSID_DxcCompiler, &Compiler.GetRef());

		COMPtr<IDxcLibrary> Library;
		DxcDllHelper.CreateInstance(CLSID_DxcLibrary, &Library.GetRef());

		COMPtr<IDxcBlobEncoding> TextBlob;
		Library->CreateBlobWithEncodingFromPinned(code.data(), code.size(), CP_UTF8, &TextBlob.GetRef());

		leo::Text::String wSourceName(SourceName.data(), SourceName.size());

		COMPtr<IDxcOperationResult> CompileResult;
		CheckHResult(Compiler->Compile(
			TextBlob.Get(),
			(wchar_t*)wSourceName.data(),
			(wchar_t*)String(entry_point).data(),
			(wchar_t*)String(profile).data(),
			args.data(),
			args.size(),
			defs.data(),
			defs.size(),
			nullptr,
			&CompileResult.GetRef()
		));

		HRESULT CompileResultCode;
		CompileResult->GetStatus(&CompileResultCode);

		platform_ex::COMPtr<IDxcBlobEncoding> error_blob;
		CompileResult->GetErrorBuffer(&error_blob.GetRef());
		if (error_blob && error_blob->GetBufferSize())
		{
			auto error =reinterpret_cast<char*>(error_blob->GetBufferPointer());
			LE_LogError(error);
		}

		platform_ex::COMPtr<ID3DBlob> code_blob;
		if (SUCCEEDED(CompileResultCode))
		{
			CheckHResult(CompileResult->GetResult((IDxcBlob**)&code_blob.GetRef()));
		}

		ShaderBlob blob;
		blob.first = std::make_unique<stdex::byte[]>(code_blob->GetBufferSize());
		blob.second = code_blob->GetBufferSize();
		std::memcpy(blob.first.get(), code_blob->GetBufferPointer(), blob.second);
		return std::move(blob);

		return {};
	}

	template <typename T>
	static HRESULT D3DCreateReflectionFromBlob(ID3DBlob* DxilBlob, COMPtr<T>& OutReflection)
	{
		dxc::DxcDllSupport& DxcDllHelper = GetDxcDllHelper();

		COMPtr<IDxcContainerReflection> ContainerReflection;
		CheckHResult(DxcDllHelper.CreateInstance(CLSID_DxcContainerReflection, &ContainerReflection.GetRef()));
		CheckHResult(ContainerReflection->Load((IDxcBlob*)DxilBlob));

		const uint32 DxilPartKind = DXIL_FOURCC('D', 'X', 'I', 'L');
		uint32 DxilPartIndex = ~0u;
		CheckHResult(ContainerReflection->FindFirstPartKind(DxilPartKind, &DxilPartIndex));

		HRESULT Result = ContainerReflection->GetPartReflection(DxilPartIndex, IID_PPV_ARGS(&OutReflection.GetRef()));

		return Result;
	}

	ShaderInfo* ReflectDXIL(const ShaderBlob& blob, ShaderType type)
	{
		bool bIsRayTracingShader = IsRayTracingShader(type);


		return nullptr;
	}

	ShaderBlob StripDXIL(const ShaderBlob& code_blob, leo::uint32 flags) {
		ShaderBlob blob;
		blob.first = std::make_unique<stdex::byte[]>(code_blob.second);
		blob.second = code_blob.second;
		std::memcpy(blob.first.get(), code_blob.first.get(), blob.second);
		return std::move(blob);
	}
}

namespace asset::X::Shader
{
	ShaderBlob Compile(ShaderType type, std::string_view Code,
		std::string_view entry_point, const std::vector<ShaderMacro>& macros,
		std::string_view profile, leo::uint32 flags, std::string_view SourceName)
	{
		bool use_dxc = IsRayTracingShader(type);

		auto compile_ptr = use_dxc ? DXIL::CompileToDXIL : DXBC::CompileToDXBC;

		return (*compile_ptr)(type, Code, entry_point, macros, profile, flags, SourceName);
	}
	ShaderInfo* Reflect(const ShaderBlob& blob, ShaderType type)
	{
		bool use_dxc = IsRayTracingShader(type);

		auto reflect_ptr = use_dxc ? DXIL::ReflectDXIL : DXBC::ReflectDXBC;

		return (*reflect_ptr)(blob,type);
	}

	ShaderBlob Strip(const ShaderBlob& code_blob, ShaderType type, leo::uint32 flags) {
		bool use_dxc = IsRayTracingShader(type);

		auto strip_ptr = use_dxc ? DXIL::StripDXIL : DXBC::StripDXBC;

		return (*strip_ptr)(code_blob, flags);
	}
}

#else
//TODO CryEngine HLSLCross Compiler?
//Other Target Platfom Compiler [Tool...]
#endif