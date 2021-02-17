#include "ShaderParametersMetadata.h"

using namespace platform::Render;

ShaderParametersMetadata::ShaderParametersMetadata(
	uint32 InSize, 
	const std::vector<Member>& InMembers)
	:Size(InSize),
	Members(InMembers)
{
}
