#include "Light.hpp"
#include "Camera.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
#include "Vertex.hpp"
#include <RenderSystem\d3dx11.hpp>
#include <RenderSystem\ShaderMgr.h>
#include <leomathutility.hpp>
#include <Singleton.hpp>
#include <exception.hpp>
using namespace leo;

#ifdef near
#undef near
#undef far
#endif

ops::Rect leo::CalcScissorRect(const PointLight & wPointLight, const Camera & camera)
{
	//Create a bounding sphere for the light,based on the position
	//and range
	auto centerWS =float4( wPointLight.Position,1.f);
	auto radius = wPointLight.FallOff_Range.w;


	auto centerWSvector = load(centerWS);
	auto ViewMatrixmatrix = load(camera.View());

	float4 centerVS;
	//Transfrom the sphere center to view space
	save(centerVS,Multiply(centerWSvector, ViewMatrixmatrix));

	//Figure out the four points at the top,bottom,left, and
	//right of the sphere
	auto topVS = centerVS;
	topVS.y += radius;

	auto bottomVS = centerVS;
	bottomVS.y -= radius;

	auto leftVS = centerVS;
	leftVS.x -= radius;

	auto rightVS = centerVS;
	rightVS.y += radius;

	//Figure out whether we want to use the top and right from quad
	//tangent to the front of the sphere, or the back or the sphere
	leftVS.z = leftVS.x < 0.f ? leftVS.z - radius:leftVS.z+radius;
	rightVS.z = rightVS.x < 0.f ? rightVS.z + radius : rightVS.z - radius;
	topVS.z = topVS.x < 0.f ? topVS.z + radius : topVS.z - radius;
	bottomVS.z = bottomVS.x < 0.f ? bottomVS.z - radius : bottomVS.z + radius;

	//Clamp the z coordinate to the clip planes
	auto near = camera.mNear;
	auto far = camera.mFar;
	leftVS.z = clamp( near, far, leftVS.z);
	rightVS.z = clamp( near, far, rightVS.z);
	topVS.z = clamp(near, far, topVS.z);
	bottomVS.z = clamp( near, far, bottomVS.z);

	//Figure out the rectangle in clip-space by applying the
	//perspective transfrom. We assume that the perspective
	//transfrom is symmetrical with respect to X and Y
	auto ProjMatrix = camera.Proj();

	auto rectLeftCS = leftVS.x*ProjMatrix(0, 0) / leftVS.z;
	auto rectRightCS = rightVS.x*ProjMatrix(0, 0)/ rightVS.z;
	auto rectTopCS = topVS.y*ProjMatrix(1, 1) / topVS.z;
	auto rectBottomCS = bottomVS.y*ProjMatrix(1, 1) / bottomVS.z;

	//Clamp the rectangle to the screen extents
	rectLeftCS = clamp(-1.f, 1.f, rectLeftCS);
	rectRightCS = clamp(-1.f, 1.f, rectRightCS);
	rectTopCS = clamp(-1.f, 1.f, rectTopCS);
	rectBottomCS = clamp(-1.f, 1.f, rectBottomCS);

	//Now we convert to screen coordinates by applying the
	//viewport transfrom
	auto rectTopSS = rectTopCS*0.5f + 0.5f;
	auto rectRightSS = rectRightCS*0.5f + 0.5f;
	auto rectBottomSS = rectBottomCS*0.5f + 0.5f;
	auto rectLeftSS = rectLeftCS*0.5f + 0.5f;

	rectTopSS = 1.f - rectTopSS;
	rectBottomSS = 1.f - rectBottomSS;

	return ops::Rect(float4(rectTopSS,rectLeftSS,rectBottomSS,rectRightSS));
}

leo::LightSource::LightSource(light_type type)
	:_type(type)
{
}

leo::LightSource::light_type leo::LightSource::Type() const
{
	return _type;
}

const float3 & leo::LightSource::Position() const
{
	return mPos;
}

float leo::LightSource::Range() const
{
	return mRange;
}

const float3 & leo::LightSource::Diffuse() const
{
	return mDiffuse;
}

void leo::LightSource::Position(const float3 & pos)
{
	mPos = pos;
}

void leo::LightSource::Range(float range)
{
	mRange = range;
}

void leo::LightSource::Diffuse(const float3 & diffuse)
{
	mDiffuse = diffuse;
}

leo::PointLightSource::PointLightSource()
	:LightSource(point_light)
{
}

const float3& leo::PointLightSource::FallOff() const {
	return mFallOff;
}

void leo::PointLightSource::FallOff(const float3& falloff){
	 mFallOff = falloff;
}


leo::SpotLightSource::SpotLightSource()
	:LightSource(spot_light)
{
}

const float3& leo::SpotLightSource::FallOff() const {
	return mFallOff;
}

void leo::SpotLightSource::FallOff(const float3& falloff) {
	mFallOff = falloff;
}

const D3D11_INPUT_ELEMENT_DESC static mLightVolumeVertexElement_Desc[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

class PointLightVolumeImpl :public leo::Singleton<PointLightVolumeImpl,false>{
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

	void Apply(ID3D11DeviceContext * context, PointLightSource& light_source,const Camera& camera) {
		leo::SQT scale{};
		scale.s = light_source.Range();
		scale.t = light_source.Position();
		mVSCBParams.WorldView = scale.operator std::array<__m128, 4U>();

		ApplyLightVolumeCommon(context, camera);

		PointLight mPSCBParams;
		auto point =float4( light_source.Position(),1.f);
		save(mPSCBParams.Position,Multiply(load(point),load(camera.View())));
		mPSCBParams.Diffuse = light_source.Diffuse();
		mPSCBParams.FallOff_Range = float4(light_source.FallOff(), light_source.Range());

		context->UpdateSubresource(mPSCB, 0, nullptr, &mPSCBParams, 0, 0);

		UINT strides[] = { sizeof(leo::float3) };
		UINT offsets[] = { 0 };

		context->IASetVertexBuffers(0, 1, &mPointVolumeVB, strides, offsets);
		context->IASetIndexBuffer(mPointVolumeIB, DXGI_FORMAT_R32_UINT, 0);

		context->PSSetShader(mPointLightVolumePS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);

	}
	void Draw(ID3D11DeviceContext * context) {
		context->DrawIndexed(mIndexCount, 0, 0);
	}

	//vertex shader
	//input layout
	//update matrix
	//note mVSCBParams.WorldView == WorldMatrix
	void ApplyLightVolumeCommon(ID3D11DeviceContext * context, const Camera& camera) {
		mVSCBParams.WorldView = leo::Transpose(load(camera.View()));
		//leo::Multiply(mVSCBParams.WorldView, 
		mVSCBParams.Proj = leo::Transpose(load(camera.Proj()));


		context->UpdateSubresource(mVSCB, 0, nullptr, &mVSCBParams, 0, 0);

		context->IASetInputLayout(mLightVolumeVertexLayout);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetShader(mLightVolumeVS, nullptr, 0);
		context->VSSetConstantBuffers(0, 1, &mVSCB);
	}
public:
	static PointLightVolumeImpl& GetInstance(ID3D11Device* device =nullptr) {
		static PointLightVolumeImpl mInstance{device};
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

class SpotLightVolumeImpl :public leo::Singleton<PointLightVolumeImpl, false> {
public:
	SpotLightVolumeImpl(ID3D11Device* device) {
		leo::ShaderMgr sm;
		
		auto meshdata = leo::helper::CreateCone(100.f,100.f,12);
		mIndexCount = meshdata.Indices.size();
		CD3D11_BUFFER_DESC vbDesc{ meshdata.Vertices.size()*sizeof(decltype(meshdata.Vertices)::value_type),
			D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE };

		D3D11_SUBRESOURCE_DATA resDesc;
		resDesc.pSysMem = &meshdata.Vertices[0];
		leo::dxcall(device->CreateBuffer(&vbDesc, &resDesc, &mSpotVolumeVB));

		CD3D11_BUFFER_DESC ibDesc{ vbDesc };
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
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

		PointLightVolumeImpl::GetInstance().mVSCBParams.WorldView = CalcWorld(light_source);

		PointLightVolumeImpl::GetInstance().ApplyLightVolumeCommon(context, camera);

		SpotLight mPSCBParams;
		auto point = float4(light_source.Position(),1.f);
		save(mPSCBParams.Position_Inner, Multiply(load(point), load(camera.View())));
		mPSCBParams.Position_Inner.w = light_source.CosInnerAngle();
		mPSCBParams.Diffuse = light_source.Diffuse();
		mPSCBParams.FallOff_Range = float4(light_source.FallOff(), light_source.Range());
		auto dir = float4(light_source.Directional(), 1.f);
		//Todo: TransposeInverse view
		save(mPSCBParams.Directional_Outer, Multiply(load(dir), load(camera.View())));
		mPSCBParams.Directional_Outer.w = light_source.CosOuterAngle();

		context->UpdateSubresource(mPSCB, 0, nullptr, &mPSCBParams, 0, 0);

		UINT strides[] = { sizeof(leo::float3) };
		UINT offsets[] = { 0 };

		context->IASetVertexBuffers(0, 1, &mSpotVolumeVB, strides, offsets);
		context->IASetIndexBuffer(mSpotVolumeIB, DXGI_FORMAT_R32_UINT, 0);

		context->PSSetShader(mSpotLightVolumePS, nullptr, 0);
		context->PSSetConstantBuffers(0, 1, &mPSCB);

	}
	void Draw(ID3D11DeviceContext * context) {
		context->DrawIndexed(mIndexCount, 0, 0);
	}

private:
	std::array<__m128, 4U> CalcWorld(SpotLightSource& light_source) {
		auto xyscale = tan(acos(light_source.CosOuterAngle()))/tan(15*LM_RPD);
		auto zscale = light_source.Range() / 100.f;
		std::array<__m128, 4> scalematrix  {};
		scalematrix[0] = load(float4(xyscale, 0.f, 0.f, 0.f));
		scalematrix[1] = load(float4(0.f, xyscale, 0.f, 0.f));
		scalematrix[2] = load(float4(0.f, 0.f,zscale, 0.f));
		scalematrix[3] = details::SplatR3();

		auto axis = light_source.Directional();
		axis.x /= 2;
		axis.y /= 2;
		axis.z += 1;
		axis.z /= 2;

		SQTObject sqt;
		sqt.Rotation(axis, LM_PI);
		sqt.Translation(light_source.Position());
		return Multiply(scalematrix,sqt);
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

leo::LightSourcesRender::LightSourcesRender(ID3D11Device * device)
{
	PointLightVolumeImpl::GetInstance(device);
	SpotLightVolumeImpl::GetInstance(device);
}

leo::LightSourcesRender::~LightSourcesRender()
{
	PointLightVolumeImpl::GetInstance().~PointLightVolumeImpl();
}

void leo::LightSourcesRender::Draw(ID3D11DeviceContext * context,DeferredRender & pRender, const Camera & camera)
{
	pRender.ApplyLightPass(context);
	Apply(context);
	for (auto &light_source : mLightSourceList) {
		switch (light_source->Type())
		{
		case LightSource::point_light:
			PointLightVolumeImpl::GetInstance().Apply(context, dynamic_cast<PointLightSource&>(*light_source),camera);
			PointLightVolumeImpl::GetInstance().Draw(context);
			break;
		case LightSource::spot_light:
			SpotLightVolumeImpl::GetInstance().Apply(context, dynamic_cast<SpotLightSource&>(*light_source), camera);
			SpotLightVolumeImpl::GetInstance().Draw(context);
			break;
		default:
			break;
		}
	}
}

void leo::LightSourcesRender::AddLight(std::shared_ptr<LightSource> light_source)
{
	mLightSourceList.push_back(light_source);
}

void leo::LightSourcesRender::Apply(ID3D11DeviceContext * context)
{
}
