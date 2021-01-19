#include "BuiltInShader.h"

using namespace platform::Render;

ShaderRef<RenderShader> Shader::BuiltInShaderMapSection::GetShader(ShaderMeta* ShaderType, int32 PermutationId) const
{
	return Content.GetShader(ShaderType, PermutationId);
}

Shader::BuiltInShaderMap::~BuiltInShaderMap()
{
	for (auto& section : SectionMap)
	{
		delete section.second;
	}

	SectionMap.clear();
}

ShaderRef<RenderShader> Shader::BuiltInShaderMap::GetShader(ShaderMeta* ShaderType, int32 PermutationId) const
{
	auto section_itr = SectionMap.find(ShaderType->GetHashedShaderFilename());

	return section_itr != SectionMap.end() ? section_itr->second->GetShader(ShaderType, PermutationId):ShaderRef<RenderShader>();
}

RenderShader* Shader::BuiltInShaderMap::FindOrAddShader(const ShaderMeta* ShaderType, int32 PermutationId, RenderShader* Shader)
{
	MapMutex.lock_shared();
	auto key = ShaderType->GetHashedShaderFilename();
	auto section_itr = SectionMap.find(key);
	MapMutex.unlock_shared();
	if (section_itr == SectionMap.end())
	{
		MapMutex.lock();
		section_itr = SectionMap.emplace(key, new BuiltInShaderMapSection(key)).first;
		MapMutex.unlock();
	}

	return section_itr->second->Content.FindOrAddShader(ShaderType->GetHash(),PermutationId,Shader);
}
