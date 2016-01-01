#include "CopyProcess.hpp"
#include "RenderStates.hpp"
#include <Core\FileSearch.h>
class PointCopy :public leo::PostProcess {
public:
	PointCopy(ID3D11Device* create)
		:PostProcess(create)
	{
		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		point_sampler = leo::RenderStates().CreateSamplerState(L"point_sampler", mSampleDesc);

		BindProcess(create, leo::FileSearch::Search(L"Copy.cso"));
	}

	virtual void Apply(ID3D11DeviceContext* context) override {
		PostProcess::Apply(context);
		context->PSGetSamplers(0, 1, &point_sampler);
	}

private:
	ID3D11SamplerState* point_sampler;
};

class BilinearCopy :public leo::PostProcess {
public:
	BilinearCopy(ID3D11Device* create)
		:PostProcess(create)
	{
		CD3D11_SAMPLER_DESC mSampleDesc{ D3D11_DEFAULT };
		mSampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		bilinear_sampler = leo::RenderStates().CreateSamplerState(L"bilinear_sampler", mSampleDesc);

		BindProcess(create, leo::FileSearch::Search(L"BilinearCopy.cso"));
	}

	virtual void Apply(ID3D11DeviceContext* context) override {
		PostProcess::Apply(context);
		context->PSSetSamplers(0, 1, &bilinear_sampler);
	}
private:
	ID3D11SamplerState* bilinear_sampler;
};

class AddPointCopy :public PointCopy {
public:
	AddPointCopy(ID3D11Device* create)
		:PointCopy(create)
	{
		CD3D11_BLEND_DESC add_blendDesc(D3D11_DEFAULT);
		add_blendDesc.RenderTarget[0].BlendEnable = true;
		add_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		add_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blend_add = leo::RenderStates().CreateBlendState(L"add_blend",add_blendDesc);
	}

	void Apply(ID3D11DeviceContext* context) override
	{
		PointCopy::Apply(context);
		context->OMSetBlendState(blend_add, nullptr, 0XFFFFFFFF);
	}

	void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override {
		PointCopy::Draw(context, src, dst);
	}
private:
	ID3D11BlendState*  blend_add;
};

class AddBilinearCopy :public BilinearCopy {
public:
	AddBilinearCopy(ID3D11Device* create)
		:BilinearCopy(create)
	{
		CD3D11_BLEND_DESC add_blendDesc(D3D11_DEFAULT);
		add_blendDesc.RenderTarget[0].BlendEnable = true;
		add_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		add_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		add_blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blend_add = leo::RenderStates().CreateBlendState(L"add_blend", add_blendDesc);
	}
	void Apply(ID3D11DeviceContext* context) override
	{
		BilinearCopy::Apply(context);
		context->OMSetBlendState(blend_add, nullptr, 0XFFFFFFFF);
	}

	void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override {
		BilinearCopy::Draw(context, src, dst);
		context->OMSetBlendState(nullptr, nullptr, 0XFFFFFFFF);
	}
private:
	ID3D11BlendState*  blend_add;
};

std::shared_ptr<leo::PostProcess> leo::Make_CopyProcess(ID3D11Device * create, copyprocesss_type type)
{
	switch (type)
	{
	case leo::point_process:
		return std::make_shared<PointCopy>(create);
		break;
	case leo::bilinear_process:
		return std::make_shared<BilinearCopy>(create);
		break;
	case leo::point_addprocess:
		return std::make_shared<AddPointCopy>(create);
		break;
	case leo::bilinear_addprocess:
		return std::make_shared<AddBilinearCopy>(create);
		break;
	}

	return nullptr;
}
