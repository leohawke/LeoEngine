#include "PipelineStateUtility.h"
#include "IContext.h"
using namespace platform::Render;

void SetGraphicsPipelineState(CommandList& cmdlist, const GraphicsPipelineStateInitializer& initializer)
{
	//TODO Cache
	auto state = Context::Instance().GetDevice().CreateGraphicsPipelineState(initializer);

	cmdlist.SetGraphicsPipelineState(state);
}
