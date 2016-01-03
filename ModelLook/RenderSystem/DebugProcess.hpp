#ifndef ShaderSystem_DebugProcess_Hpp
#define ShaderSystem_DebugProcess_Hpp

#include  "RenderSystem/PostProcess.hpp"
#include "RenderSystem/RenderStates.hpp"
#include <Core\FileSearch.h>

namespace leo 
{
	class NormalDebug :public PostProcess {
	public:
		NormalDebug(ID3D11Device* create)
			:PostProcess(create)
		{
			CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
			mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			point_sampler = leo::RenderStates().CreateSamplerState(L"point_sampler", mSampleDesc);

			BindProcess(create, leo::FileSearch::Search(L"NormalDebug.cso"));
		}

		void Apply(ID3D11DeviceContext* context) override {
			PostProcess::Apply(context);
			context->PSSetSamplers(0, 1, &point_sampler);
		}

	private:
		ID3D11SamplerState* point_sampler;
	};
}

#endif
