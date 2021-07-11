#include "Shader.h"
#include "BuiltInRayTracingShader.h"
#include "IContext.h"
#include "IRayContext.h"
#include "../Asset/D3DShaderCompiler.h"
#include "LFramework/Helper/ShellHelper.h"
#include "../Core/Coroutine/Task.h"
#include "../Core/Coroutine/ThreadScheduler.h"
#include "../Core/Coroutine/SyncWait.h"
#include "../Core/Coroutine/WhenAllReady.h"
#include "../System/SystemEnvironment.h"
#include "../Asset/ShaderAsset.h"
#include "../Core/Hash/CityHash.h"
#include "BuiltInShader.h"
#include "ShaderParametersMetadata.h"
#include "ShaderDB.h"
#include "spdlog/spdlog.h"
#include "Core/Path.h"
#include "Core/Serialization/Archive.h"
#include "spdlog/stopwatch.h"
#include <format>

using namespace platform::Render;
using namespace LeoEngine;

namespace platform::Render::Shader
{
	std::list<ShaderMeta*>* GShaderTypeList;

	bool IsRayTracingShader(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::RayGen:
		case ShaderType::RayMiss:
		case ShaderType::RayHitGroup:
		case ShaderType::RayCallable:
			return true;
		default:
			return false;
		}
	}

	std::list<ShaderMeta*>& ShaderMeta::GetTypeList()
	{
		if (GShaderTypeList == nullptr)
			GShaderTypeList = new std::list<ShaderMeta*>();
		return *GShaderTypeList;
	}

	RenderShader* ShaderMeta::Construct() const
	{
		return (*ConstructRef)();
	}

	bool Shader::ShaderMeta::ShouldCompilePermutation(const FShaderPermutationParameters& Parameters) const
	{
		return (*ShouldCompilePermutationRef)(Parameters);
	}

	void ShaderMeta::ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) const
	{
		(*ModifyCompilationEnvironmentRef)(Parameters, OutEnvironment);
	}

	ShaderMeta::ShaderMeta(EShaderMetaForDownCast InShaderMetaForDownCast, const char* InName, const char* InSourceFileName, const char* InEntryPoint, platform::Render::ShaderType InFrequency, int32 InTotalPermutationCount,
		ConstructType InConstructRef, 
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCompilePermutationType InShouldCompilePermutationRef,
		uint32 InTypeSize,
		const ShaderParametersMetadata* InRootParametersMetadata
	)
		:
		ShaderMetaForDownCast(InShaderMetaForDownCast),
		TypeName(InName), 
		Hash(std::hash<std::string>()(TypeName)),
		SourceFileName(InSourceFileName),
		HashedShaderFilename(std::hash<std::string>()(SourceFileName)),
		EntryPoint(InEntryPoint), 
		Frequency(InFrequency), 
		TotalPermutationCount(InTotalPermutationCount),
		ConstructRef(InConstructRef),
		ModifyCompilationEnvironmentRef(InModifyCompilationEnvironmentRef),
		ShouldCompilePermutationRef(InShouldCompilePermutationRef),
		RootParametersMetadata(InRootParametersMetadata)
	{
		GetTypeList().emplace_front(this);
	}

	ImplDeCtor(RenderShader);
	ImplDeDtor(RenderShader);

	RenderShader::RenderShader(const CompiledShaderInitializer& initializer)
		:
		Shader(initializer.Shader),
		Meta(initializer.Meta)
	{
	}

	static void FillParameterMapByShaderInfo(ShaderParameterMap& target, const ShaderInfo& src);

	BuiltInShaderMap GGlobalBuiltInShaderMap;

	std::string ParseAndMoveShaderParametersToRootConstantBuffer(std::string code,const asset::X::Shader::ShaderCompilerInput& input);
	void PullRootShaderParametersLayout(asset::X::Shader::ShaderCompilerInput& Input, const ShaderParametersMetadata& ParameterMeta);

	leo::coroutine::Task<void> operator>>(AsyncArchive& archive, platform::Render::ShaderInitializer& initializer)
	{
		auto pBlob = const_cast<ShaderBlob*>(initializer.pBlob);

		co_await (archive >> pBlob->second);

		if (archive.IsLoading())
		{
			pBlob->first = std::make_unique<stdex::byte[]>(initializer.pBlob->second);
		}
		co_await archive.Serialize(pBlob->first.get(), pBlob->second);

		auto pInfo = const_cast<ShaderInfo*>(initializer.pInfo);
		co_await pInfo->Serialize(archive);
	}

	leo::coroutine::Task<bool> IsShaderCache(BuiltInShaderMeta* meta,const fs::path& path);

	leo::coroutine::Task<void> WriteShaderCache(BuiltInShaderMeta* meta);

	void InsertCompileOuput(BuiltInShaderMeta* meta, platform::Render::ShaderInitializer initializer, int32 PermutationId)
	{
		auto pBuiltInMeta = meta->GetBuiltInShaderType();
		if (!pBuiltInMeta)
			return;

		if (IsRayTracingShader(meta->GetShaderType()))
		{

			auto& RayDevice = Context::Instance().GetRayContext().GetDevice();
			auto pRayTracingShaderRHI = RayDevice.CreateRayTracingSahder(initializer);

			RenderShader::CompiledShaderInitializer compileOuput;
			compileOuput.Shader = pRayTracingShaderRHI;
			compileOuput.Meta = meta;
			FillParameterMapByShaderInfo(compileOuput.ParameterMap, *initializer.pInfo);

			auto pShader = static_cast<BuiltInRayTracingShader*>(pBuiltInMeta->Construct(compileOuput));

			GGlobalBuiltInShaderMap.FindOrAddShader(meta, PermutationId, pShader);
		}
		else {
			auto& Device = Context::Instance().GetDevice();

			auto pShaderRHI = Device.CreateShader(initializer);

			RenderShader::CompiledShaderInitializer compileOuput;
			compileOuput.Shader = pShaderRHI;
			compileOuput.Meta = meta;

			FillParameterMapByShaderInfo(compileOuput.ParameterMap, *initializer.pInfo);

			auto pShader = pBuiltInMeta->Construct(compileOuput);

			GGlobalBuiltInShaderMap.FindOrAddShader(meta, PermutationId, pShader);
		}
	}

	leo::coroutine::Task<void> CompileBuiltInShader(BuiltInShaderMeta* meta,int32 PermutationId,AsyncArchive& archive)
	{
		LFL_DEBUG_DECL_TIMER(Commpile, std::format("CompileBuiltInShader {} Entry:{} Permutation={} ", meta->GetSourceFileName(), meta->GetEntryPoint(), PermutationId));

		//shader build system
		asset::X::Shader::ShaderCompilerInput input;

		input.EntryPoint = meta->GetEntryPoint();
		input.Type = meta->GetShaderType();
		input.SourceName = meta->GetSourceFileName();

		auto last_write_time = std::filesystem::last_write_time(input.SourceName);

		//meta_write_time

		if (meta->GetRootParametersMetadata())
		{
			PullRootShaderParametersLayout(input, *meta->GetRootParametersMetadata());
			std::sort(input.RootParameterBindings.begin(),input.RootParameterBindings.end());
		}

		// Allow the shader type to modify the compile environment.
		meta->SetupCompileEnvironment(PermutationId, input.Environment);
		asset::X::Shader::AppendCompilerEnvironment(input.Environment, input.Type);

		auto Code = co_await platform::X::GenHlslShaderAsync(meta->GetSourceFileName());

		Code = asset::X::Shader::PreprocessShader(Code, input);

		if (IsRayTracingShader(meta->GetShaderType()))
			Code = ParseAndMoveShaderParametersToRootConstantBuffer(Code, input);

		input.Code = Code;

		ShaderInfo Info{ input.Type };

		auto blob = asset::X::Shader::CompileAndReflect(input,
#ifndef NDEBUG
			D3DFlags::D3DCOMPILE_DEBUG
#else
			D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
			, &Info
		);

		platform::Render::ShaderInitializer initializer{
			.pBlob = &blob,
			.pInfo = &Info
		};
		InsertCompileOuput(meta, initializer, PermutationId);

		co_await(archive >> PermutationId);
		co_await(archive >> Info);

		co_return;
	}

	fs::path FormatShaderSavePath(BuiltInShaderMeta* meta);

	void CompileShaderMap()
	{
		spdlog::stopwatch sw;

		std::vector< leo::coroutine::Task<void>> tasks;
		for (auto meta : ShaderMeta::GetTypeList())
		{
			//TODO:dispatch type
			if (auto pBuiltInMeta = meta->GetBuiltInShaderType())
			{
				tasks.emplace_back([](BuiltInShaderMeta* meta) -> leo::coroutine::Task<void>
				{
					std::vector< leo::coroutine::Task<void>> tasks;

					int32 PermutationCountToCompile = 0;

					auto localcachepath = FormatShaderSavePath(meta);

					if (co_await IsShaderCache(meta, localcachepath))
						co_return;

					auto pArchive = leo::unique_raw(LeoEngine::CreateFileWriter(localcachepath));

					auto hash = std::hash<std::string>()(meta->GetSourceFileName());
					//Serialize meta
					co_await pArchive->Serialize(&hash,sizeof(hash));

					for (int32 PermutationId = 0; PermutationId < meta->GetPermutationCount(); PermutationId++)
					{
						if (!meta->ShouldCompilePermutation(PermutationId))
							continue;

						co_await CompileBuiltInShader(meta, PermutationId,*pArchive);
					}

					co_await WriteShaderCache(meta);
				}(pBuiltInMeta));
			}
		}

		leo::coroutine::SyncWait(leo::coroutine::WhenAllReady(std::move(tasks)));
		spdlog::info("CompileShaderMap: {} seconds", sw);
	}

	void FillParameterMapByShaderInfo(ShaderParameterMap& target, const ShaderInfo& src)
	{
		for (auto& cb : src.ConstantBufferInfos)
		{
			auto& name = cb.name;

			bool bGlobalCB = name == "$Globals";
			if (bGlobalCB)
			{
				for (auto& var : cb.var_desc)
				{
					target.AddParameterAllocation(var.name, cb.bind_point,
						var.start_offset, var.size, ShaderParamClass::LooseData);
				}
			}
			else
			{
				target.AddParameterAllocation(name, 0,cb.bind_point,
					 cb.size, ShaderParamClass::UniformBuffer);
			}
		}
		for (auto& br : src.BoundResourceInfos)
		{
			ShaderParamClass Class = static_cast<ShaderParamClass>(br.type);

			target.AddParameterAllocation(br.name, 0, br.bind_point,
				 1, Class);
		}
	}

	BuiltInShaderMap* Shader::GetBuiltInShaderMap()
	{
		return &GGlobalBuiltInShaderMap;
	}

	namespace fs = std::filesystem;

	leo::coroutine::Task<bool> IsShaderCache(BuiltInShaderMeta* meta, const fs::path& path) {
		if (!fs::exists(path))
			co_return false;

		fs::path path_key = meta->GetSourceFileName();
		auto last_write_time = fs::last_write_time(path_key);

		auto db_time = LeoEngine::ShaderDB::QueryTime(meta->GetSourceFileName());
		if (!db_time.has_value() || db_time.value() != last_write_time)
			co_return false;

		//read manifest
		//for manifest check file time

		//load cache
		auto pArchive = leo::unique_raw(LeoEngine::CreateFileReader(path));

		auto hash = std::hash<std::string>()(meta->GetSourceFileName());
		//Serialize meta
		co_await pArchive->Serialize(&hash, sizeof(hash));

		for (int32 PermutationId = 0; PermutationId < meta->GetPermutationCount(); PermutationId++)
		{
			//cause error when impl change
			if (!meta->ShouldCompilePermutation(PermutationId))
				continue;

			ShaderInfo Info{ meta->GetShaderType() };
			ShaderBlob Blob;

			platform::Render::ShaderInitializer initializer{
				.pBlob = &Blob,
				.pInfo = &Info
			};

			co_await(*pArchive >> initializer);

			InsertCompileOuput(meta, initializer, PermutationId);
		}

		co_return true;
	}

	fs::path FormatShaderSavePath(BuiltInShaderMeta* meta) {
		static auto save_dir = [] {
			auto directory = (LeoEngine::PathSet::EngineIntermediateDir() / "Shaders");
			fs::create_directories(directory);
			return directory.string();
		}();

		std::string cache_filename;

		auto hash = std::hash<std::string>()(meta->GetSourceFileName());
		std::format_to(std::back_inserter(cache_filename), "{}/{:X}", save_dir, hash);

		std::array<char, 4> key = {};
		auto itr = meta->GetSourceFileName().rbegin();
		for (int i = 0; itr != meta->GetSourceFileName().rend() && i < 8; ++i, ++itr)
		{
			if (i > 3)
				key[i - 4] = *itr;
		}

		std::format_to(std::back_inserter(cache_filename), "_{:X}", *reinterpret_cast<unsigned*>(key.data()));

		return cache_filename;
	}

	leo::coroutine::Task<void> WriteShaderCache(BuiltInShaderMeta* meta) {
		fs::path path_key = meta->GetSourceFileName();
		auto last_write_time = fs::last_write_time(path_key);

		co_await LeoEngine::ShaderDB::UpdateTime(meta->GetSourceFileName(), last_write_time);

		co_return;
	}

}

RenderShader* Shader::ShaderMapContent::GetShader(size_t TypeNameHash, int32 PermutationId) const
{
	auto Hash = (uint16)CityHash128to64({ TypeNameHash,(uint64)PermutationId });

	auto range = ShaderHash.equal_range(Hash);
	for (auto itr = range.first; itr != range.second; ++itr)
	{
		auto Index = itr->second;
		if (ShaderTypes[Index] == TypeNameHash && ShaderPermutations[Index] == PermutationId)
		{
			return Shaders[Index];
		}
	}
	return nullptr;
}

RenderShader* Shader::ShaderMapContent::FindOrAddShader(size_t TypeNameHash, int32 PermutationId, RenderShader* Shader)
{
	{
		std::shared_lock lock{ ShaderMutex };
		auto FindShader = GetShader(TypeNameHash, PermutationId);

		if (FindShader != nullptr)
			return FindShader;
	}

	std::unique_lock lock{ ShaderMutex };

	auto Hash = (uint16)CityHash128to64({ TypeNameHash,(uint64)PermutationId });

	const int32 Index = GetNumShaders();

	ShaderHash.emplace(Hash, Index);
	Shaders.emplace_back(Shader);
	ShaderTypes.emplace_back(TypeNameHash);
	ShaderPermutations.emplace_back(PermutationId);

	return Shader;
}

void Shader::ShaderMapContent::Clear()
{
	for (auto Shader : Shaders)
	{
		delete Shader;
	}

	Shaders.clear();
	ShaderTypes.clear();
	ShaderPermutations.clear();
	ShaderHash.clear();
}

uint32 Shader::ShaderMapContent::GetNumShaders() const
{
	return static_cast<int32>(Shaders.size());
}


void Shader::PullRootShaderParametersLayout(asset::X::Shader::ShaderCompilerInput& Input, const ShaderParametersMetadata& ParameterMeta)
{
	for (auto& member : ParameterMeta.GetMembers())
	{
		auto ShaderType = member.GetShaderType();

		const bool bIsVariableNativeType = (
			ShaderType >= SPT_uint &&
			ShaderType <= SPT_float4x4
			);

		if (bIsVariableNativeType)
		{
			ShaderCompilerInput::RootParameterBinding RootParameterBinding;
			RootParameterBinding.Name = member.GetName();
			RootParameterBinding.ExpectedShaderType = asset::ShadersAsset::GetTypeName(member.GetShaderType());
			RootParameterBinding.ByteOffset = member.GetOffset();
			Input.RootParameterBindings.emplace_back(RootParameterBinding);
		}
	}
}

std::string Shader::ParseAndMoveShaderParametersToRootConstantBuffer(std::string code, const asset::X::Shader::ShaderCompilerInput& input)
{
	if (input.RootParameterBindings.empty())
		return code;

	struct ParsedShaderParameter
	{
		std::string Type;

		bool IsFound() const
		{
			return !Type.empty();
		}
	};
	std::map<std::string, ParsedShaderParameter> ParsedParameters;

	for (auto& Member : input.RootParameterBindings)
	{
		ParsedParameters.emplace(Member.Name, ParsedShaderParameter{});
	}


	// Browse the code for global shader parameter, Save their type and erase them white spaces.
	{
		enum class EState
		{
			// When to look for something to scan.
			Scanning,

			// When going to next ; in the global scope and reset.
			GoToNextSemicolonAndReset,

			// Parsing what might be a type of the parameter.
			ParsingPotentialType,
			FinishedPotentialType,

			// Parsing what might be a name of the parameter.
			ParsingPotentialName,
			FinishedPotentialName,

			// Parsing what looks like array of the parameter.
			ParsingPotentialArraySize,
			FinishedArraySize,

			// Found a parameter, just finish to it's semi colon.
			FoundParameter,
		};

		const int32 ShaderSourceLen =static_cast<int32>(code.size());

		int32 CurrentPragamLineoffset = -1;
		int32 CurrentLineoffset = 0;

		int32 TypeStartPos = -1;
		int32 TypeEndPos = -1;
		int32 NameStartPos = -1;
		int32 NameEndPos = -1;
		int32 ScopeIndent = 0;

		EState State = EState::Scanning;
		bool bGoToNextLine = false;

		auto ResetState = [&]()
		{
			TypeStartPos = -1;
			TypeEndPos = -1;
			NameStartPos = -1;
			NameEndPos = -1;
			State = EState::Scanning;
		};

		auto EmitError = [&](const std::string& ErrorMessage)
		{
			LE_LogError(ErrorMessage.c_str());
		};

		auto EmitUnpextectedHLSLSyntaxError = [&]()
		{
			EmitError("Unexpected syntax when parsing shader parameters from shader code.");
			State = EState::GoToNextSemicolonAndReset;
		};

		for (int32 Cursor = 0; Cursor < ShaderSourceLen; Cursor++)
		{
			const auto Char = code[Cursor];

			auto FoundShaderParameter = [&]()
			{
				lconstraint(Char == ';');
				lconstraint(TypeStartPos != -1);
				lconstraint(TypeEndPos != -1);
				lconstraint(NameStartPos != -1);
				lconstraint(NameEndPos != -1);

				auto Type = code.substr(TypeStartPos, TypeEndPos - TypeStartPos + 1);
				auto Name = code.substr(NameStartPos, NameEndPos - NameStartPos + 1);

				if (ParsedParameters.contains(Name))
				{
					if (ParsedParameters[Name].IsFound())
					{
						// If it has already been found, it means it is duplicated. Do nothing and let the shader compiler throw the error.
					}
					else
					{
						ParsedParameters[Name] = ParsedShaderParameter{.Type = Type};

						// Erases this shader parameter conserving the same line numbers.
						if (true)
						{
							for (int32 j = TypeStartPos; j <= Cursor; j++)
							{
								if (code[j] != '\r' && code[j] != '\n')
									code[j] = ' ';
							}
						}
					}
				}

				ResetState();
			};

			const bool bIsWhiteSpace = Char == ' ' || Char == '\t' || Char == '\r' || Char == '\n';
			const bool bIsLetter = (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z');
			const bool bIsNumber = Char >= '0' && Char <= '9';

			const auto UpComing = (code.c_str()) + Cursor;
			const int32 RemainingSize = ShaderSourceLen - Cursor;

			CurrentLineoffset += Char == '\n';

			// Go to the next line if this is a preprocessor macro.
			if (bGoToNextLine)
			{
				if (Char == '\n')
				{
					bGoToNextLine = false;
				}
				continue;
			}
			else if (Char == '#')
			{
				if (RemainingSize > 6 && std::strncmp(UpComing, "#line ", 6) == 0)
				{
					CurrentPragamLineoffset = Cursor;
					CurrentLineoffset = -1; // that will be incremented to 0 when reaching the \n at the end of the #line
				}

				bGoToNextLine = true;
				continue;
			}

			// If within a scope, just carry on until outside the scope.
			if (ScopeIndent > 0 || Char == '{')
			{
				if (Char == '{')
				{
					ScopeIndent++;
				}
				else if (Char == '}')
				{
					ScopeIndent--;
					if (ScopeIndent == 0)
					{
						ResetState();
					}
				}
				continue;
			}

			if (State == EState::Scanning)
			{
				if (bIsLetter)
				{
					static const char* KeywordTable[] = {
						"enum",
						"class",
						"const",
						"struct",
						"static",
					};
					static int32 KeywordTableSize[] = { 4, 5, 5, 6, 6 };

					int32 RecognisedKeywordId = -1;
					for (int32 KeywordId = 0; KeywordId < leo::arrlen(KeywordTable); KeywordId++)
					{
						const char* Keyword = KeywordTable[KeywordId];
						const int32 KeywordSize = KeywordTableSize[KeywordId];

						if (RemainingSize > KeywordSize)
						{
							char KeywordEndTestChar = UpComing[KeywordSize];

							if ((KeywordEndTestChar == ' ' || KeywordEndTestChar == '\r' || KeywordEndTestChar == '\n' || KeywordEndTestChar == '\t') &&
								std::strncmp(UpComing, Keyword, KeywordSize) == 0)
							{
								RecognisedKeywordId = KeywordId;
								break;
							}
						}
					}

					if (RecognisedKeywordId == -1)
					{
						// Might have found beginning of the type of a parameter.
						State = EState::ParsingPotentialType;
						TypeStartPos = Cursor;
					}
					else if (RecognisedKeywordId == 2)
					{
						// Ignore the const keywords, but still parse given it might still be a shader parameter.
						Cursor += KeywordTableSize[RecognisedKeywordId];
					}
					else
					{
						// Purposefully ignore enum, class, struct, static
						State = EState::GoToNextSemicolonAndReset;
					}
				}
				else if (bIsWhiteSpace)
				{
					// Keep parsing void.
				}
				else if (Char == ';')
				{
					// Looks like redundant semicolon, just ignore and keep scanning.
				}
				else
				{
					// No idea what this is, just go to next semi colon.
					State = EState::GoToNextSemicolonAndReset;
				}
			}
			else if (State == EState::GoToNextSemicolonAndReset)
			{
				// If need to go to next global semicolon and reach it. Resume browsing.
				if (Char == ';')
				{
					ResetState();
				}
			}
			else if (State == EState::ParsingPotentialType)
			{
				// Found character legal for a type...
				if (bIsLetter ||
					bIsNumber ||
					Char == '<' || Char == '>' || Char == '_')
				{
					// Keep browsing what might be type of the parameter.
				}
				else if (bIsWhiteSpace)
				{
					// Might have found a type.
					State = EState::FinishedPotentialType;
					TypeEndPos = Cursor - 1;
				}
				else
				{
					// Found unexpected character in the type.
					State = EState::GoToNextSemicolonAndReset;
				}
			}
			else if (State == EState::FinishedPotentialType)
			{
				if (bIsLetter)
				{
					// Might have found beginning of the name of a parameter.
					State = EState::ParsingPotentialName;
					NameStartPos = Cursor;
				}
				else if (bIsWhiteSpace)
				{
					// Keep parsing void.
				}
				else
				{
					// No idea what this is, just go to next semi colon.
					State = EState::GoToNextSemicolonAndReset;
				}
			}
			else if (State == EState::ParsingPotentialName)
			{
				// Found character legal for a name...
				if (bIsLetter ||
					bIsNumber ||
					Char == '_')
				{
					// Keep browsing what might be name of the parameter.
				}
				else if (Char == ':' || Char == '=')
				{
					// Found a parameter with syntax:
					// uint MyParameter : <whatever>;
					// uint MyParameter = <DefaultValue>;
					NameEndPos = Cursor - 1;
					State = EState::FoundParameter;
				}
				else if (Char == ';')
				{
					// Found a parameter with syntax:
					// uint MyParameter;
					NameEndPos = Cursor - 1;
					FoundShaderParameter();
				}
				else if (Char == '[')
				{
					// Syntax:
					//  uint MyArray[
					NameEndPos = Cursor - 1;
					State = EState::ParsingPotentialArraySize;
				}
				else if (bIsWhiteSpace)
				{
					// Might have found a name.
					// uint MyParameter <Still need to know what is after>;
					NameEndPos = Cursor - 1;
					State = EState::FinishedPotentialName;
				}
				else
				{
					// Found unexpected character in the name.
					// syntax:
					// uint MyFunction(<Don't care what is after>
					State = EState::GoToNextSemicolonAndReset;
				}
			}
			else if (
				State == EState::FinishedPotentialName ||
				State == EState::FinishedArraySize)
			{
				if (Char == ';')
				{
					// Found a parameter with syntax:
					// uint MyParameter <a bit of OK stuf>;
					FoundShaderParameter();
				}
				else if (Char == ':')
				{
					// Found a parameter with syntax:
					// uint MyParameter <a bit of OK stuf> : <Ignore all this crap>;
					State = EState::FoundParameter;
				}
				else if (Char == '=')
				{
					// Found syntax that doesn't make any sens:
					// uint MyParameter <a bit of OK stuf> = <Ignore all this crap>;
					State = EState::FoundParameter;
					// TDOO: should error out that this is useless.
				}
				else if (Char == '[')
				{
					if (State == EState::FinishedPotentialName)
					{
						// Syntax:
						//  uint MyArray [
						State = EState::ParsingPotentialArraySize;
					}
					else
					{
						EmitError("Shader parameters can only support one dimensional array");
					}
				}
				else if (bIsWhiteSpace)
				{
					// Keep parsing void.
				}
				else
				{
					// Found unexpected stuff.
					State = EState::GoToNextSemicolonAndReset;
				}
			}
			else if (State == EState::ParsingPotentialArraySize)
			{
				if (Char == ']')
				{
					State = EState::FinishedArraySize;
				}
				else if (Char == ';')
				{
					EmitUnpextectedHLSLSyntaxError();
				}
				else
				{
					// Keep going through the array size that might be a complex expression.
				}
			}
			else if (State == EState::FoundParameter)
			{
				if (Char == ';')
				{
					FoundShaderParameter();
				}
				else
				{
					// Cary on skipping all crap we don't care about shader parameter until we find it's semi colon.
				}
			}
			else
			{
				throw leo::unimplemented();
			}
		} // for (int32 Cursor = 0; Cursor < PreprocessedShaderSource.Len(); Cursor++)
	}

	std::string rootcbuffer;
	for (auto& Member : input.RootParameterBindings)
	{
		std::string register_alloc = leo::to_string(Member.ByteOffset/16);

		switch (Member.ByteOffset % 16)
		{
		case 0:
			break;
		case 4:
			register_alloc += ".y";
			break;
		case 8:
			register_alloc += ".z";
			break;
		case 12:
			register_alloc += ".w";
			break;
		default:
			lassume(false);//not alignment to 4 bytes
		}

		const auto& ParsedParameter = ParsedParameters[Member.Name];

		rootcbuffer += leo::sfmt("%s %s : packoffset(c%s);\r\n", ParsedParameter.Type.c_str(), Member.Name.c_str(), register_alloc.c_str());
	}

	std::string new_code = leo::sfmt(
		"%s %s\r\n"
		"{\r\n"
		"%s"
		"}\r\n\r\n%s",
		"cbuffer",
		ShaderParametersMetadata::kRootUniformBufferBindingName,
		rootcbuffer.c_str(),
		code.c_str()
	);

	return new_code;
}