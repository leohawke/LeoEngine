#pragma once

#include <LBase/lmath.hpp>
#include "Render/ICommandList.h"
#include "Math/IntRect.h"

namespace LeoEngine
{
	namespace lr = platform::Render;
	namespace lm = leo::math;

	class ProjectedShadowInfo
	{
	public:
		lm::float3 PreShadowTranslation;

		lm::float4x4 SubjectAndReceiverMatrix;

		lr::RenderPassInfo* PassInfo;

		/**
		* X and Y position of the shadow in the appropriate depth buffer.  These are only initialized after the shadow has been allocated.
		 * The actual contents of the shadowmap are at X + BorderSize, Y + BorderSize.
		*/
		uint32 X;
		uint32 Y;

		/**
		 * Resolution of the shadow, excluding the border.
		 * The full size of the region allocated to this shadow is therefore ResolutionX + 2 * BorderSize, ResolutionY + 2 * BorderSize.
		 */
		uint32 ResolutionX;
		uint32 ResolutionY;

		/** Size of the border, if any, used to allow filtering without clamping for shadows stored in an atlas. */
		uint32 BorderSize;
	};

	lr::GraphicsPipelineStateInitializer SetupShadowDepthPass(const ProjectedShadowInfo& ShadowInfo, lr::CommandList& CmdList);
}