#pragma once
#include "Render/Color_T.hpp"
#include "Render/ICommandList.h"
#include "Runtime/SceneInfo.h"


namespace LeoEngine
{
	struct GizmosElementVertex
	{
		lm::float4 Position;
		lm::float2 TextureCoordinate;
		LinearColor Color;

		static platform::Render::VertexDeclarationElements VertexDeclaration;

		GizmosElementVertex(lm::float3 pos,lm::float2 uv,LinearColor color)
			:Position(pos,1), TextureCoordinate(uv),Color(color)
		{}
	};
	
	class GizmosElements
	{
	public:
		/** Adds a line to the batch. Note only SE_BLEND_Opaque will be used for batched line rendering. */
		void AddLine(const lm::float3& Start, const lm::float3& End, const LinearColor& Color, float Thickness = 0.0f, float DepthBias = 0.0f, bool bScreenSpace = false);

		bool Draw(platform::Render::CommandList& CmdList, const LeoEngine::SceneInfo& Info);

	private:
		struct BatchPoint
		{
			lm::float3 Position;
			float Size;
			FColor Color;
		};

		std::vector< GizmosElementVertex> Lines;
	};
}