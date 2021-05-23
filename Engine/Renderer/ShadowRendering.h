#pragma once

#include <LBase/lmath.hpp>
#include "Render/ICommandList.h"
#include "Math/IntRect.h"
#include "Math/Sphere.h"
#include "Runtime/SceneInfo.h"
#include "Runtime/SceneClasses.h"

namespace LeoEngine
{
	namespace lr = platform::Render;
	namespace lm = leo::math;

	class ProjectedShadowInfo
	{
	public:
		lm::float3 PreShadowTranslation;

		lm::float4x4 ShadowViewMatrix;

		lm::float4x4 SubjectAndReceiverMatrix;

		float InvMaxSubjectDepth = 0;

		lr::Texture* DepthTarget;

		float MaxSubjectZ = 0;
		float MinSubjectZ = 0;

		Sphere ShadowBounds;

		ShadowCascadeSettings CascadeSettings;


		/**
		* X and Y position of the shadow in the appropriate depth buffer.  These are only initialized after the shadow has been allocated.
		 * The actual contents of the shadowmap are at X + BorderSize, Y + BorderSize.
		*/
		uint32 X = 0;
		uint32 Y = 0;

		/**
		 * Resolution of the shadow, excluding the border.
		 * The full size of the region allocated to this shadow is therefore ResolutionX + 2 * BorderSize, ResolutionY + 2 * BorderSize.
		 */
		uint32 ResolutionX = 0;
		uint32 ResolutionY = 0;

		/** Size of the border, if any, used to allow filtering without clamping for shadows stored in an atlas. */
		uint32 BorderSize = 0;

		void SetupWholeSceneProjection(const SceneInfo& scne, const WholeSceneProjectedShadowInitializer& ShadowInfo, uint32 InResolutionX, uint32 InResoultionY, uint32 InBorderSize);

		lr::GraphicsPipelineStateInitializer SetupShadowDepthPass(lr::CommandList& CmdList);

	private:
		float ShaderDepthBias = 0;
		float ShaderSlopeDepthBias = 0;
		float ShaderMaxSlopeDepthBias = 0;
	};
}