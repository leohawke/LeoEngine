#pragma once

#include "ShaderCore.h"
#include <LBase/exception_type.h>

#define PR_NAMESPACE_BEGIN  namespace platform::Render {
#define PR_NAMESPACE_END }

PR_NAMESPACE_BEGIN
inline namespace Shader
{
	using leo::uint32;

	class ShaderParametersMetadata
	{
	public:
		class Member
		{
		public:
			Member(
				const char* InName,
				uint32 InOffset,
				ShaderParamType InType
				)
				:
				Name(InName),
				Offset(InOffset),
				Type(InType)
			{}

			const char* GetName() const { return Name; }

			uint32 GetOffset() const { return Offset; }

			ShaderParamType GetShaderType() const { return Type; }

			uint32 GetMemberSize() const { throw leo::unimplemented(); }
		private:
			const char* Name;
			uint32 Offset;
			ShaderParamType Type;
		};

		ShaderParametersMetadata(const std::vector<Member>& InMembers);

		const std::vector<Member>& GetMembers() const { return Members; }
	private:
		std::vector<Member> Members;
	};
}
PR_NAMESPACE_END

#undef PR_NAMESPACE_BEGIN
#undef PR_NAMESPACE_END
