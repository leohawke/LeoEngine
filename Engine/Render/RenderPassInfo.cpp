#include "RenderPassInfo.h"

using namespace platform::Render;

RenderTargetsInfo::RenderTargetsInfo(const RenderPassInfo& Info)
{
	for (int32 Index = 0; Index < MaxSimultaneousRenderTargets; ++Index)
	{
		if (!Info.ColorRenderTargets[Index].RenderTarget)
		{
			break;
		}
		ColorRenderTarget[Index].Texture = Info.ColorRenderTargets[Index].RenderTarget;
		RenderTargetLoadAction LoadAction = GetLoadAction(Info.ColorRenderTargets[Index].Action);
		ColorRenderTarget[Index].LoadAction = LoadAction;
		ColorRenderTarget[Index].StoreAction = GetStoreAction(Info.ColorRenderTargets[Index].Action);
		ColorRenderTarget[Index].ArraySlice = Info.ColorRenderTargets[Index].ArraySlice;
		ColorRenderTarget[Index].MipIndex = Info.ColorRenderTargets[Index].MipIndex;
		++NumColorRenderTargets;

		bClearColor |= (LoadAction == RenderTargetLoadAction::Clear);
	}

	RenderTargetActions DepthActions = GetDepthActions(Info.DepthStencilRenderTarget.Action);
	RenderTargetActions StencilActions = GetStencilActions(Info.DepthStencilRenderTarget.Action);
	RenderTargetLoadAction DepthLoadAction = GetLoadAction(DepthActions);
	RenderTargetStoreAction DepthStoreAction = GetStoreAction(DepthActions);
	RenderTargetLoadAction StencilLoadAction = GetLoadAction(StencilActions);
	RenderTargetStoreAction StencilStoreAction = GetStoreAction(StencilActions);

	DepthStencilRenderTarget = DepthRenderTarget(
		Info.DepthStencilRenderTarget.DepthStencilTarget,
		DepthLoadAction,
		GetStoreAction(DepthActions),
		StencilLoadAction,
		GetStoreAction(StencilActions)
	);

	bClearDepth = (DepthLoadAction == RenderTargetLoadAction::Clear);
	bClearStencil = (StencilLoadAction == RenderTargetLoadAction::Clear);
}
