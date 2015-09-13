#include <exception.hpp>
#include <Core\FileSearch.h>
#include <Core\EngineConfig.h>
#include <Core\Vertex.hpp>
#include <RenderSystem\d3dx11.hpp>
#include <RenderSystem\ShaderMgr.h>
#include "DeferredRender.hpp"

using namespace leo;

void leo::DeferredRender::AddLight(std::shared_ptr<LightSource> light_source)
{
	mLightSourceList.push_back(light_source);
}

const D3D11_INPUT_ELEMENT_DESC static mLightVolumeVertexElement_Desc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

class PointLightVolumeImpl :public leo::Singleton<PointLightVolumeImpl, false> {
public:
	PointLightVolumeImpl(ID3D11Device* device) {
		leo::ShaderMgr sm;
		//common
		{
			mLightVolumeVS = sm.CreateVertexShader(
				leo::FileSearch::Search(
					leo::EngineConfig::ShaderConfig::GetShaderFileName(L"pointlight", D3D11_VERTEX_SHADER)
					),
				nullptr,
				mLightVolumeVertexElement_Desc,
				leo::arrlen(mLightVolumeVertexElement_Desc),
				&mLightVolumeVertexLayout);

			CD3D11_BUFFER_DESC vscbDesc{ sizeof(TransfromMatrix),D3D11_BIND_CONSTANT_BUFFER };

			leo::dxcall(device->CreateBuffer(&vscbDesc, nullptr, &mVSCB));
		}

		auto meshdata = leo::helper::CreateSphere(16, 16);
		mIndexCount = meshdata.Indices.size();
		CD3D11_BUFFER_DESC vbDesc{ meshdata.Vertices.size()*sizeof(decltype(meshdata.Vertices)::value_type),
			D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE };

		D3D11_SUBRESOURCE_DATA resDesc;
		resDesc.pSysMem = &meshdata.Vertices[0];
		leo::dxcall(device->CreateBuffer(&vbDesc, &resDesc, &mPointVolumeVB));

		CD3D11_BUFFER_DESC ibDesc{ vbDesc };
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.ByteWidth = static_cast<leo::win::UINT> (sizeof(std::uint32_t)*meshdata.Indices.size());
		resDesc.pSysMem = &meshdata.Indices[0];
		leo::dxcall(device->CreateBuffer(&ibDesc, &resDesc, &mPointVolumeIB));

		CD3D11_BUFFER_DESC pscbDesc{ sizeof(leo::PointLight),D3D11_BIND_CONSTANT_BUFFER };

		leo::dxcall(device->CreateBuffer(&pscbDesc, nullptr, &mPSCB));

		mPointLightVolumePS = sm.CreatePixelShader(
			leo::FileSearch::Search(
				leo::EngineConfig::ShaderConfig::GetShaderFileName(L"pointlight", D3D11_PIXEL_SHADER)
				));
	}

	~PointLightVolumeImpl() {

	}

	void Apply(ID3D11DeviceContext * context, PointLightSource& light_source, const Camera& camera) {
		leo::SQT scale{};
		scale.s = light_source.Range();
		scale.t = light_source.Position();
		//TODO : Join this
		mVSCBParams.WorldView = scale.operator std::array<__m128, 4U>();
		mVSCBParams.WorldView = leo::Transpose(leo::Multiply(mVSCBParams.WorldView, load(camera.View())));
		mVSCBParams.Proj = leo::Transpose(load(camera.Proj()));
		context->UpdateSubresource(mVSCB, 0, nullptr, &mVSCBParams, 0, 0);
		context->IASetInputLayout(mLightVolumeVertexLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetConstantBuffers(0, 1, &mVSCB);
		UINT strides[] = { sizeof(leo::float3) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &mPointVolumeVB, strides, offsets);
		context->IASetIndexBuffer(mPointVolumeIB, DXGI_FORMAT_R32_UINT, 0);
	}
	UINT GetIndexCount() const noexcept {
		return mIndexCount;
	}

	void Draw(ID3D11DeviceContext * context, PointLightSource& light_source, const Camera& camera) {
		PointLight mPSCBParams;
		auto point = float4(light_source.Position(), 1.f);
		save(mPSCBParams.Position, Multiply(load(point), load(camera.View())));
		mPSCBParams.Diffuse = light_source.Diffuse();
		mPSCBParams.FallOff_Range = float4(light_source.FallOff(), light_source.Range());
		context->UpdateSubresource(mPSCB, 0, nullptr, &mPSCBParams, 0, 0);
		context->VSSetShader(mLightVolumeVS, nullptr, 0);
		context->PSSetShader(mPointLightVolumePS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);
		context->DrawIndexed(mIndexCount, 0, 0);
	}
public:
	static PointLightVolumeImpl& GetInstance(ID3D11Device* device = nullptr) {
		static PointLightVolumeImpl mInstance{ device };
		return mInstance;
	}

	//common
	ID3D11InputLayout* mLightVolumeVertexLayout = nullptr;


	ID3D11VertexShader* mLightVolumeVS = nullptr;

	struct TransfromMatrix {
		std::array<__m128, 4> WorldView;
		std::array<__m128, 4> Proj;
	} mVSCBParams;
	leo::win::unique_com<ID3D11Buffer>  mVSCB = nullptr;
	//end-common

	ID3D11PixelShader* mPointLightVolumePS = nullptr;

	leo::win::unique_com<ID3D11Buffer>  mPSCB = nullptr;

	leo::win::unique_com<ID3D11Buffer> mPointVolumeVB = nullptr;
	leo::win::unique_com<ID3D11Buffer>  mPointVolumeIB = nullptr;

	UINT mIndexCount = 0;
};

class SpotLightVolumeImpl :public leo::Singleton<SpotLightVolumeImpl, false> {
public:
	SpotLightVolumeImpl(ID3D11Device* device) {
		leo::ShaderMgr sm;

		auto meshdata = leo::helper::CreateCone(100.f, 100.f, 12);
		mIndexCount = meshdata.Indices.size();
		CD3D11_BUFFER_DESC vbDesc{ meshdata.Vertices.size()*sizeof(decltype(meshdata.Vertices)::value_type),
			D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE };

		D3D11_SUBRESOURCE_DATA resDesc;
		resDesc.pSysMem = &meshdata.Vertices[0];
		leo::dxcall(device->CreateBuffer(&vbDesc, &resDesc, &mSpotVolumeVB));

		CD3D11_BUFFER_DESC ibDesc{ vbDesc };
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
		ibDesc.ByteWidth = static_cast<leo::win::UINT> (sizeof(std::uint32_t)*meshdata.Indices.size());
		resDesc.pSysMem = &meshdata.Indices[0];
		leo::dxcall(device->CreateBuffer(&ibDesc, &resDesc, &mSpotVolumeIB));

		CD3D11_BUFFER_DESC pscbDesc{ sizeof(leo::SpotLight),D3D11_BIND_CONSTANT_BUFFER };

		leo::dxcall(device->CreateBuffer(&pscbDesc, nullptr, &mPSCB));

		mSpotLightVolumePS = sm.CreatePixelShader(
			leo::FileSearch::Search(
				leo::EngineConfig::ShaderConfig::GetShaderFileName(L"spotlight", D3D11_PIXEL_SHADER)
				));
	}

	~SpotLightVolumeImpl() {

	}

	void Apply(ID3D11DeviceContext * context, SpotLightSource& light_source, const Camera& camera) {

		auto & mPointImpl = PointLightVolumeImpl::GetInstance();
		auto & mVSCBParams = mPointImpl.mVSCBParams;
		mVSCBParams.WorldView = CalcWorld(light_source);

		mVSCBParams.WorldView = leo::Transpose(leo::Multiply(mVSCBParams.WorldView, load(camera.View())));
		mVSCBParams.Proj = leo::Transpose(load(camera.Proj()));
		context->UpdateSubresource(mPointImpl.mVSCB, 0, nullptr, &mVSCBParams, 0, 0);
		context->IASetInputLayout(mPointImpl.mLightVolumeVertexLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetConstantBuffers(0, 1, &mPointImpl.mVSCB);

		UINT strides[] = { sizeof(leo::float3) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &mSpotVolumeVB, strides, offsets);
		context->IASetIndexBuffer(mSpotVolumeIB, DXGI_FORMAT_R32_UINT, 0);
	}
	void Draw(ID3D11DeviceContext * context, SpotLightSource& light_source, const Camera& camera) {
		SpotLight mPSCBParams;
		auto point = float4(light_source.Position(), 1.f);
		save(mPSCBParams.Position_Inner, Multiply(load(point), load(camera.View())));
		mPSCBParams.Position_Inner.w = light_source.CosInnerAngle();
		mPSCBParams.Diffuse = light_source.Diffuse();
		mPSCBParams.FallOff_Range = float4(light_source.FallOff(), light_source.Range());
		auto dir = float4(light_source.Directional(), 1.f);
		//Todo: TransposeInverse view
		save(mPSCBParams.Directional_Outer, Multiply(load(dir), load(camera.View())));
		mPSCBParams.Directional_Outer.w = light_source.CosOuterAngle();
		auto & mPointImpl = PointLightVolumeImpl::GetInstance();
		context->UpdateSubresource(mPSCB, 0, nullptr, &mPSCBParams, 0, 0);
		context->VSSetShader(mPointImpl.mLightVolumeVS, nullptr, 0);
		context->PSSetShader(mSpotLightVolumePS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);
		context->DrawIndexed(mIndexCount, 0, 0);
	}

	UINT GetIndexCount() const noexcept {
		return mIndexCount;
	}
private:
	std::array<__m128, 4U> CalcWorld(SpotLightSource& light_source) {
		auto range = light_source.Range();
		auto sinouter = light_source.SinOuterAngle();
		auto cosouter = light_source.CosOuterAngle();
		auto yscale = range*cosouter / 100.f;
		auto xzscale = range*sinouter / (100.f / cos(LM_PI / 12));
		std::array<__m128, 4> scalematrix{};
		scalematrix[0] = load(float4(xzscale, 0.f, 0.f, 0.f));
		scalematrix[1] = load(float4(0.f, yscale, 0.f, 0.f));
		scalematrix[2] = load(float4(0.f, 0.f, xzscale, 0.f));
		scalematrix[3] = details::SplatR3();

		auto axis = light_source.Directional();
		axis.x /= 2;
		axis.y /= 2;
		axis.z += 1;
		axis.z /= 2;

		SQTObject sqt;
		sqt.Rotation(axis, LM_PI);
		sqt.Translation(light_source.Position());
		return Multiply(scalematrix, sqt);
	}

public:
	static SpotLightVolumeImpl& GetInstance(ID3D11Device* device = nullptr) {
		static SpotLightVolumeImpl mInstance{ device };
		return mInstance;
	}

	ID3D11PixelShader* mSpotLightVolumePS = nullptr;

	leo::win::unique_com<ID3D11Buffer>  mPSCB = nullptr;

	leo::win::unique_com<ID3D11Buffer> mSpotVolumeVB = nullptr;
	leo::win::unique_com<ID3D11Buffer>  mSpotVolumeIB = nullptr;

	UINT mIndexCount = 0;
};

class DirectionalVolumeImpl :public leo::Singleton<DirectionalVolumeImpl, false> {
public:
	DirectionalVolumeImpl(ID3D11Device* device) {
		leo::ShaderMgr sm;

		mDirectionalLightQuadPS = sm.CreatePixelShader(
			leo::FileSearch::Search(L"DirectionalLightQuadPS.cso"));

		CD3D11_BUFFER_DESC pscbDesc{ sizeof(leo::DirectionalLight),D3D11_BIND_CONSTANT_BUFFER };

		leo::dxcall(device->CreateBuffer(&pscbDesc, nullptr, &mPSCB));
	}

	~DirectionalVolumeImpl() {
	}

	void Apply(ID3D11DeviceContext * context, DirectionalLightSource& light_source, const Camera& camera) {
		EffectQuad::GetInstance().Apply(context);
	}

	void Draw(ID3D11DeviceContext * context, DirectionalLightSource& light_source, const Camera& camera) {
		DirectionalLight mPSCBParams;
		mPSCBParams.Diffuse = light_source.Diffuse();
		mPSCBParams.Directional = -light_source.Directional();
		context->UpdateSubresource(mPSCB, 0, 0, &mPSCBParams, 0, 0);
		context->PSSetShader(mDirectionalLightQuadPS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);
		//Important ,Disable z-write
		EffectQuad::GetInstance().Draw(context);
	}

	static DirectionalVolumeImpl& GetInstance(ID3D11Device* device = nullptr) {
		static DirectionalVolumeImpl mInstance{ device };
		return mInstance;
	}

private:
	ID3D11PixelShader* mDirectionalLightQuadPS = nullptr;

	leo::win::unique_com<ID3D11Buffer>  mPSCB = nullptr;
};

void leo::DeferredRender::LightSourcesRender::Init(ID3D11Device * device)
{
	PointLightVolumeImpl::GetInstance(device);
	SpotLightVolumeImpl::GetInstance(device);
	DirectionalVolumeImpl::GetInstance(device);
}

void leo::DeferredRender::LightSourcesRender::Destroy()
{
	PointLightVolumeImpl::GetInstance().~PointLightVolumeImpl();
	SpotLightVolumeImpl::GetInstance().~SpotLightVolumeImpl();
	DirectionalVolumeImpl::GetInstance().~DirectionalVolumeImpl();
}

void leo::DeferredRender::LightPass(ID3D11DeviceContext * context, DepthStencil& depthstencil, const Camera & camera) noexcept
{
	const static float factor[] = { 0.f,0.f,0.f,0.f };

	context->ClearRenderTargetView(pResImpl->mLightRTV, factor);
	context->OMSetRenderTargets(1, &pResImpl->mLightRTV, depthstencil);
	ID3D11ShaderResourceView* srvs[] = { GetLinearDepthSRV(),GetNormalAlphaSRV() };
	context->PSSetSamplers(0, 1, &LinearizeDepthImpl::GetInstance().mSamPoint);
	context->PSSetShaderResources(0, 2, srvs);

	for (auto &light_source : mLightSourceList) {
		switch (light_source->Type())
		{
		case LightSource::point_light: {
			auto & point_light = dynamic_cast<PointLightSource&>(*light_source);
			auto & pointImpl = PointLightVolumeImpl::GetInstance();
			pointImpl.Apply(context, point_light, camera);
			LightVolumePass(context, pointImpl.GetIndexCount());
			pointImpl.Draw(context, point_light, camera);
		}
									   break;
		case LightSource::spot_light: {
			auto & spot_light = dynamic_cast<SpotLightSource&>(*light_source);
			auto & spotImpl = SpotLightVolumeImpl::GetInstance();
			spotImpl.Apply(context, spot_light, camera);
			LightVolumePass(context, spotImpl.GetIndexCount());
			spotImpl.Draw(context, spot_light, camera);
		}
									  break;
		case LightSource::directional_light: {
			auto & dir_light = dynamic_cast<DirectionalLightSource&>(*light_source);
			auto & dirImpl = DirectionalVolumeImpl::GetInstance();
			dirImpl.Apply(context, dir_light, camera);
			LightQuadPass(context);
			dirImpl.Draw(context, dir_light, camera);
		}
											 break;
		default:
			break;
		}
	}
}

