#include "effect.h"
#include "RenderSystem\ShaderMgr.h"
#include "RenderSystem\RenderStates.hpp"
#include "..\DeviceMgr.h"
#include "Vertex.hpp"
#include "FileSearch.h"
#include "EngineConfig.h"
#include "..\math.hpp"
#include <d3dcompiler.h>

#include <atomic>
#pragma comment(lib,"d3dcompiler.lib")
namespace leo
{
	using vector = __m128;
	using matrix = std::array < __m128, 4 >;

	template<typename COM>
	void ReleaseCOM(COM* &com, std::false_type) {
		if (com)
		{
			com->Release();
			com = nullptr;
		}
	}


	class EffectConfig::Delegate
	{
	public:
		EffectLevel GetLevel() const lnothrow
		{
			return mLevel;
		}

		bool SetLevel(EffectLevel l) lnothrow
		{
			mLevel = l;
			return true;
		}

		bool NormalLine() const lnothrow
		{
			return mNormalLineOpen;
		}

		void NormalLine(bool nlstate) lnothrow
		{
			mNormalLineOpen = nlstate;
		}
	private:
		EffectLevel mLevel = EffectLevel::hign;
		std::atomic<bool> mNormalLineOpen = false;
	};
	EffectConfig::EffectConfig()
		:pDelegate(new Delegate())
	{}

	EffectConfig::~EffectConfig()
	{}

	EffectConfig::EffectLevel EffectConfig::GetLevel() const lnothrow
	{
		return pDelegate->GetLevel();
	}
	bool EffectConfig::SetLevel(EffectLevel l) lnothrow
	{
		return pDelegate->SetLevel(l);
	}

	bool EffectConfig::NormalLine() const lnothrow
	{
		return pDelegate->NormalLine();
	}

	void EffectConfig::NormalLine(bool nlstate) lnothrow
	{
		return pDelegate->NormalLine(nlstate);
	}

	Effect::Effect()
	{}

#pragma region EffectNormalMap
	class EffectNormalMapDelegate :CONCRETE(EffectNormalMap), public Singleton<EffectNormalMapDelegate>
	{
	public:
		EffectNormalMapDelegate(ID3D11Device* device)
			:mVertexShaderConstantBufferPerFrame(device), mPixelShaderConstantBufferPerFrame(device), mPixelShaderConstantBufferPerPrimitive(device), mPixelShaderConstantBufferPerView(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(
				FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"normalmap",D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::NormalMap, 4, &layout);

			device->CreateClassLinkage(&mPixelShaderClassLinkage);
			auto blob = sm.CreateBlob(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"normalmap", D3D11_PIXEL_SHADER)));
			mPixelShader = sm.CreatePixelShader(blob, mPixelShaderClassLinkage);

			ID3D11ShaderReflection* pRefector = nullptr;
			D3DReflect(blob.GetBufferPointer(), blob.GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pRefector);
			mNumofInstance = pRefector->GetNumInterfaceSlots();
			mInstances = std::make_unique<ID3D11ClassInstance*[]>(mNumofInstance);

			ID3D11ShaderReflectionVariable* pDirLightVar = pRefector->GetVariableByName("gAbsLight");
			_gAbsLight_mInstanceIndex = pDirLightVar->GetInterfaceSlot(0);
			win::ReleaseCOM(pRefector);

			mPixelShaderClassLinkage->GetClassInstance("gDirLight", 0, &mLightInstances[0]);
			mPixelShaderClassLinkage->GetClassInstance("gPoiLight", 0, &mLightInstances[1]);
			mPixelShaderClassLinkage->GetClassInstance("gSpoLight", 0, &mLightInstances[2]);

			mInstances[_gAbsLight_mInstanceIndex] = mLightInstances[static_cast<uint8>(mLightType)];

			RenderStates rss;
			mPixelShaderSampleState = rss.GetSamplerState(L"LinearRepeat");
			mPixelShadersamShadow = rss.GetSamplerState(L"samShadow");
		}

		~EffectNormalMapDelegate()
		{
			win::ReleaseCOM(mPixelShaderClassLinkage);
			for (auto & instance : mLightInstances)
				leo::win::ReleaseCOM(instance);
		}
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"normalmap");
			mVertexShaderConstantBufferPerFrame.Update(context);

			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.VSSetConstantBuffers(0, 1, &mVertexShaderConstantBufferPerFrame.mBuffer);


			mPixelShaderConstantBufferPerFrame.Update(context);
			mPixelShaderConstantBufferPerPrimitive.Update(context);
			mPixelShaderConstantBufferPerView.Update(context);

			ID3D11Buffer* mpscbs[] = {
				mPixelShaderConstantBufferPerFrame.mBuffer,
				mPixelShaderConstantBufferPerPrimitive.mBuffer,
				mPixelShaderConstantBufferPerView.mBuffer
			};

			ID3D11ShaderResourceView* mpssrvs[] = {
				mPixelShaderDiffuseSRV,
				mPixelShaderNormalMapSRV,
				mPixelShaderShadowMapSRV
			};

			ID3D11SamplerState* mpssss[] = {
				mPixelShaderSampleState,
				mPixelShadersamShadow
			};

			mInstances[_gAbsLight_mInstanceIndex] = mLightInstances[static_cast<uint8>(mLightType)];
			pContext.PSSetShader(mPixelShader, mInstances.get(), mNumofInstance);
			pContext.PSSetConstantBuffers(0, 3, mpscbs);
			pContext.PSSetShaderResources(0, 3, mpssrvs);
			pContext.PSSetSamplers(0, 2,mpssss);
		}

		void LM_VECTOR_CALL WorldViewMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.world = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}

		void LM_VECTOR_CALL WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context = nullptr) {
			mVertexShaderConstantBufferPerFrame.worldinvtranspose = Transpose(matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}


		void  LM_VECTOR_CALL WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.worldviewproj = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}
		void  LM_VECTOR_CALL ShadowViewProjTexMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.shadowviewprojtex = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}
		void EyePos(const float3& pos, ID3D11DeviceContext* context)
		{
			memcpy(mPixelShaderConstantBufferPerView.gEyePosW, pos);
			if (context)
				mPixelShaderConstantBufferPerView.Update(context);
		}

		void Light(const DirectionalLight& dl, ID3D11DeviceContext* context)
		{
			mPixelShaderConstantBufferPerFrame.gDirLight = dl;
			mLightType = LightType::DirectionLight;
			if (context)
				mPixelShaderConstantBufferPerFrame.Update(context);
		}
		void Light(const PointLight& pl, ID3D11DeviceContext* context)
		{
			mPixelShaderConstantBufferPerFrame.gPoiLight = pl;
			mLightType = LightType::PointLight;
			if (context)
				mPixelShaderConstantBufferPerFrame.Update(context);
		}
		void Light(const SpotLight& sl, ID3D11DeviceContext* context)
		{
			mPixelShaderConstantBufferPerFrame.gSpoLight = sl;
			mLightType = LightType::SpotLight;
			if (context)
				mPixelShaderConstantBufferPerFrame.Update(context);
		}

		void Mat(const Material& mat, ID3D11DeviceContext* context)
		{
			mPixelShaderConstantBufferPerPrimitive.gMat = mat;
			if (context)
				mPixelShaderConstantBufferPerPrimitive.Update(context);
		}

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
		{
			mPixelShaderDiffuseSRV = diff;

			if (context) {
				ID3D11ShaderResourceView* mpssrvs[] = {
					mPixelShaderDiffuseSRV,
					mPixelShaderNormalMapSRV,
					mPixelShaderShadowMapSRV
				};
				context->PSSetShaderResources(0, 3, mpssrvs);
			}
		}
		void NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
		{
			mPixelShaderNormalMapSRV = nmap;
			if (context) {
				ID3D11ShaderResourceView* mpssrvs[] = {
					mPixelShaderDiffuseSRV,
					mPixelShaderNormalMapSRV,
					mPixelShaderShadowMapSRV
				};
				context->PSSetShaderResources(0, 3, mpssrvs);
			}
		}
		void ShadowMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context){
			mPixelShaderShadowMapSRV = nmap;
			if (context) {
				ID3D11ShaderResourceView* mpssrvs[] = {
					mPixelShaderDiffuseSRV,
					mPixelShaderNormalMapSRV,
					mPixelShaderShadowMapSRV
				};
				context->PSSetShaderResources(0, 3, mpssrvs);
			}
		}
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	private:
		struct VScbPerFrame
		{
			matrix world;
			matrix worldinvtranspose;
			matrix worldviewproj;
			matrix shadowviewprojtex;
		public:
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<VScbPerFrame> mVertexShaderConstantBufferPerFrame;
		struct PScbPerFrame
		{
			DirectionalLight gDirLight;
			PointLight	   gPoiLight;
			SpotLight	   gSpoLight;
		public:
			static const std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<PScbPerFrame>  mPixelShaderConstantBufferPerFrame;
		struct PScbPerPrimitive
		{
			Material gMat;
		public:
			static const std::uint8_t slot = 1;
		};
		ShaderConstantBuffer<PScbPerPrimitive> mPixelShaderConstantBufferPerPrimitive;
		struct PScbPerView
		{
			float4 gEyePosW;
		public:
			static const std::uint8_t slot = 2;
		};
		ShaderConstantBuffer<PScbPerView> mPixelShaderConstantBufferPerView;

		LightType mLightType = LightType::DirectionLight;

		ID3D11ClassInstance* mLightInstances[3];
		struct {
			std::unique_ptr<ID3D11ClassInstance*[]> mInstances = nullptr;
			std::uint8_t mNumofInstance = 0;
		};

		std::uint8_t _gAbsLight_mInstanceIndex = 0;

		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader*	mPixelShader = nullptr;
		ID3D11SamplerState* mPixelShaderSampleState = nullptr;
		ID3D11SamplerState* mPixelShadersamShadow = nullptr;
		//Release
		ID3D11ClassLinkage* mPixelShaderClassLinkage = nullptr;

		ID3D11ShaderResourceView *mPixelShaderDiffuseSRV = nullptr;
		ID3D11ShaderResourceView *mPixelShaderNormalMapSRV = nullptr;
		ID3D11ShaderResourceView *mPixelShaderShadowMapSRV = nullptr;
	};

	const std::unique_ptr<EffectNormalMap>& EffectNormalMap::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectNormalMap>(new EffectNormalMapDelegate(device));
		return mInstance;
	}

	void EffectNormalMap::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->Apply(
			context
			);
	}

	void EffectNormalMap::WorldViewMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->WorldViewMatrix(
			load(matrix), context
			);
	}

	void EffectNormalMap::WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->WorldViewProjMatrix(
			load(matrix), context
			);
	}


	void LM_VECTOR_CALL EffectNormalMap::WorldViewMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->WorldViewMatrix(
			Matrix, context
			);
	}

	void LM_VECTOR_CALL  EffectNormalMap::WorldInvTransposeViewMatrix(std::array<__m128, 4> matrix, ID3D11DeviceContext* context) {
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->WorldInvTransposeViewMatrix(
			matrix, context
			);
	}

	void LM_VECTOR_CALL EffectNormalMap::WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->WorldViewProjMatrix(
			Matrix, context
			);
	}


	void EffectNormalMap::ShadowViewProjTexMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->ShadowViewProjTexMatrix(
			load(matrix), context
			);
	}

	void LM_VECTOR_CALL EffectNormalMap::ShadowViewProjTexMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->ShadowViewProjTexMatrix(
			Matrix, context
			);
	}

	void EffectNormalMap::EyePos(const float3& pos, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->EyePos(
			pos, context
			);
	}

	void EffectNormalMap::Light(const DirectionalLight& dl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->Light(
			dl, context
			);
	}

	void EffectNormalMap::Light(const PointLight& pl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->Light(
			pl, context
			);
	}

	void EffectNormalMap::Light(const SpotLight& sl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->Light(
			sl, context
			);
	}

	void EffectNormalMap::Mat(const Material& mat, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->Mat(
			mat, context
			);
	}

	void EffectNormalMap::DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->DiffuseSRV(
			diff, context
			);
	}

	void EffectNormalMap::NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->NormalMapSRV(
			nmap, context
			);
	}

	void EffectNormalMap::ShadowMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->ShadowMapSRV(
			nmap, context
			);
	}

	bool EffectNormalMap::SetLevel(EffectConfig::EffectLevel l)  lnothrow
	{
		lassume(dynamic_cast<EffectNormalMapDelegate *>(this));

		return ((EffectNormalMapDelegate *)this)->SetLevel(
			l
			);
	}
#pragma endregion

#pragma region EffectSprite
	class EffectSpriteDelegate :CONCRETE(EffectSprite), public Singleton<EffectSpriteDelegate>
	{
	public:
		EffectSpriteDelegate(ID3D11Device* device)
			:mGeometryConstantBufferPerCamera(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"sprite", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::BillBoard, 2, &layout);
			mGeometryShader = sm.CreateGeometryShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"sprite", D3D11_GEOMETRY_SHADER)));
			mPixelShader = sm.CreatePixelShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"sprite", D3D11_PIXEL_SHADER)));

			RenderStates rss;
			mPixelShaderSampleState = rss.GetSamplerState(L"LinearRepeat");
		}
		~EffectSpriteDelegate() = default;
	public:

		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"sprite");
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.GSSetShader(mGeometryShader, nullptr, 0);

			mGeometryConstantBufferPerCamera.Update(context);
			pContext.GSSetConstantBuffers(mGeometryConstantBufferPerCamera.slot, 1, &mGeometryConstantBufferPerCamera.mBuffer);
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetSamplers(0, 1, &mPixelShaderSampleState);
		}

		void EyePos(const float3& pos, ID3D11DeviceContext* context)
		{
			memcpy(mGeometryConstantBufferPerCamera.gEyePos, pos);
			if (context)
				mGeometryConstantBufferPerCamera.Update(context);
		}

		void ViewProj(const float4x4& matrix, ID3D11DeviceContext* context)
		{
			mGeometryConstantBufferPerCamera.gViewProj = Transpose(load(matrix));
			if (context)
				mGeometryConstantBufferPerCamera.Update(context);
		}

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}

	private:
		struct GScbPerCamera
		{
			pack_type<float3> gEyePos;
			matrix gViewProj;
		public:
			static const std::uint8_t slot = 0;
		};

		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11GeometryShader* mGeometryShader = nullptr;
		ID3D11PixelShader* mPixelShader = nullptr;

		ShaderConstantBuffer<GScbPerCamera> mGeometryConstantBufferPerCamera;

		ID3D11SamplerState* mPixelShaderSampleState = nullptr;
	};

	const std::unique_ptr<EffectSprite>& EffectSprite::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectSprite>(new EffectSpriteDelegate(device));
		return mInstance;
	}

	void EffectSprite::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSpriteDelegate*>(this));

		return ((EffectSpriteDelegate*)this)->Apply(
			context
			);
	}

	void EffectSprite::EyePos(const float3& pos, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSpriteDelegate*>(this));

		return ((EffectSpriteDelegate*)this)->EyePos(
			pos, context
			);
	}

	void EffectSprite::ViewProj(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSpriteDelegate*>(this));

		return ((EffectSpriteDelegate*)this)->ViewProj(
			matrix, context
			);
	}

	bool EffectSprite::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<EffectSpriteDelegate*>(this));

		return ((EffectSpriteDelegate*)this)->SetLevel(
			l
			);
	}
#pragma endregion

#pragma region EffectSky
	class EffectSkyDelegate :CONCRETE(EffectSky), public Singleton<EffectSkyDelegate>
	{
	public:
		EffectSkyDelegate(ID3D11Device* device)
			:mVertexConstantBufferPerCamera(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"sky", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::Sky, 1, &layout);
			mPixelShader = sm.CreatePixelShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"sky", D3D11_PIXEL_SHADER)));

			RenderStates rss;
			mPixelShaderSampleState = rss.GetSamplerState(L"LinearRepeat");
		}
		~EffectSkyDelegate() = default;
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"sky");
			pContext.VSSetShader(mVertexShader, nullptr, 0);

			mVertexConstantBufferPerCamera.Update(context);
			pContext.VSSetConstantBuffers(mVertexConstantBufferPerCamera.slot, 1, &mVertexConstantBufferPerCamera.mBuffer);
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetSamplers(0, 1, &mPixelShaderSampleState);
			context->OMSetDepthStencilState(nullptr, 0);
		}

		void EyePos(const float3& pos, ID3D11DeviceContext* context)
		{
			memcpy(mVertexConstantBufferPerCamera.gEyePos, pos);
			if (context)
				mVertexConstantBufferPerCamera.Update(context);
		}

		void LM_VECTOR_CALL ViewProj(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexConstantBufferPerCamera.gViewProj = Transpose(Matrix);
			if (context)
				mVertexConstantBufferPerCamera.Update(context);
		}

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}
	private:
		struct VScbPerCamera
		{
			float3 gEyePos;
			matrix gViewProj;
		public:
			static const std::uint8_t slot = 0;
		};
		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader* mPixelShader = nullptr;

		ShaderConstantBuffer<VScbPerCamera> mVertexConstantBufferPerCamera;

		ID3D11SamplerState* mPixelShaderSampleState = nullptr;
	};

	const std::unique_ptr<EffectSky>& EffectSky::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectSky>(new EffectSkyDelegate(device));
		return mInstance;
	}

	void EffectSky::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkyDelegate*>(this));

		return ((EffectSkyDelegate*)this)->Apply(
			context
			);
	}

	void EffectSky::EyePos(const float3& pos, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkyDelegate*>(this));

		return ((EffectSkyDelegate*)this)->EyePos(
			pos, context
			);
	}

	void EffectSky::ViewProj(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkyDelegate*>(this));

		return ((EffectSkyDelegate*)this)->ViewProj(
			load(matrix), context
			);
	}

	void LM_VECTOR_CALL EffectSky::ViewProj(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkyDelegate*>(this));

		return ((EffectSkyDelegate*)this)->ViewProj(
			Matrix, context
			);
	}


	bool EffectSky::SetLevel(EffectConfig::EffectLevel l) lnothrow
	{
		lassume(dynamic_cast<EffectSkyDelegate*>(this));

		return ((EffectSkyDelegate*)this)->SetLevel(
			l
			);
	}

#pragma endregion

#pragma region EffectPostRender
	class EffectPackDelegate :CONCRETE(EffectPack), public leo::Singleton<EffectPackDelegate>
	{
	public:
		EffectPackDelegate(ID3D11Device* device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"pack", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::PostEffect, 2, &layout);
			mPixelShader = sm.CreatePixelShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"pack", D3D11_PIXEL_SHADER)));

			mSampler = leo::RenderStates().GetSamplerState(L"LinearRepeat");

			mNoDepth = leo::RenderStates().GetDepthStencilState(L"NoDepthDSS");
		}
		~EffectPackDelegate() = default;
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"pack");

			pContext.OMSetRenderTargets(1, &mDstRTV, nullptr);
			pContext.OMSetDepthStencilState(mNoDepth, 0);
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetShaderResources(0, 1, &mPackSRV);
			pContext.PSSetSamplers(0, 1, &mSampler);
		}

		void SetPackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context)
		{
			mPackSRV = srv;
			if(context)
				context->PSSetShaderResources(0, 1, &mPackSRV);
		}

		void SetDstRTV(ID3D11RenderTargetView* rtv)
		{
			mDstRTV = rtv;
		}
	private:
		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader* mPixelShader = nullptr;

		ID3D11RenderTargetView* mDstRTV = nullptr;
		ID3D11ShaderResourceView* mPackSRV = nullptr;

		ID3D11SamplerState* mSampler = nullptr;

		ID3D11DepthStencilState* mNoDepth = nullptr;
	};

	void EffectPack::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectPackDelegate*>(this));

		return ((EffectPackDelegate*)this)->Apply(
			context
			);
	}

	void EffectPack::SetDstRTV(ID3D11RenderTargetView* rtv)
	{
		lassume(dynamic_cast<EffectPackDelegate*>(this));

		return ((EffectPackDelegate*)this)->SetDstRTV(
			rtv
			);
	}

	void EffectPack::SetPackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectPackDelegate*>(this));

		return ((EffectPackDelegate*)this)->SetPackSRV(
			srv,context
			);
	}

	const std::unique_ptr<EffectPack>& EffectPack::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectPack>(new EffectPackDelegate(device));
		return mInstance;
	}

	class EffectUnPackDelegate :CONCRETE(EffectUnPack), public leo::Singleton<EffectUnPackDelegate>
	{
	public:
		EffectUnPackDelegate(ID3D11Device* device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(L"Shader\\PostCommonVS.cso", nullptr, InputLayoutDesc::PostEffect, 2, &layout);
			mPixelShader = sm.CreatePixelShader(L"Shader\\PostUnPackPS.cso");

			RenderStates rss;
			mNoDepthDSS = rss.GetDepthStencilState(L"NoDepthDSS");
		}
		~EffectUnPackDelegate() = default;
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"unpack");
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.PSSetShader(mPixelShader, nullptr, 0);
			pContext.PSSetShaderResources(0, 1, &mPixelShaderSRV);
			pContext.OMSetDepthStencilState(mNoDepthDSS, 0);
		}

		void SetUnpackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context)
		{
			mPixelShaderSRV = srv;
			if (context)
				context->PSSetShaderResources(0, 1, &mPixelShaderSRV);
		}
	private:
		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader* mPixelShader = nullptr;

		ID3D11ShaderResourceView* mPixelShaderSRV = nullptr;

		ID3D11DepthStencilState* mNoDepthDSS = nullptr;
	};

	void EffectUnPack::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectUnPackDelegate*>(this));

		return ((EffectUnPackDelegate*)this)->Apply(
			context
			);
	}


	void EffectUnPack::SetUnpackSRV(ID3D11ShaderResourceView* srv, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectUnPackDelegate*>(this));

		return ((EffectUnPackDelegate*)this)->SetUnpackSRV(
			srv, context
			);
	}

	const std::unique_ptr<EffectUnPack>& EffectUnPack::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectUnPack>(new EffectUnPackDelegate(device));
		return mInstance;
	}
#pragma endregion

#pragma region EffectNormalLine
	class EffectNormalLineDelegate : CONCRETE(EffectNormalLine), public leo::Singleton<EffectNormalLineDelegate>
	{
	public:
		struct VScbPerModel
		{
			matrix World;
			matrix WorldInvTranspose;
		};
		struct GScbPerCamera
		{
			matrix gViewProj;
		};
		struct PScbPerSet
		{
			float4 gColor;
		};
	public:
		EffectNormalLineDelegate(ID3D11Device* device, const float4& color)
			:mVertexConstantBufferPerModel(device), mGeometryConstantBufferPerCamera(device), mPixelConstantBufferPerSet(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"normalline", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::NormalMap, 4, &layout);
			mGeometryShader = sm.CreateGeometryShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"normalline", D3D11_GEOMETRY_SHADER)));
			mPixelShader = sm.CreatePixelShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"normalline", D3D11_PIXEL_SHADER)));
			mPixelConstantBufferPerSet.gColor = color;
		}
		~EffectNormalLineDelegate() = default;
	public:
		void Apply(ID3D11DeviceContext* context)
		{
			context_wrapper pContext(context, L"normalline");
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.GSSetShader(mGeometryShader, nullptr, 0);
			pContext.PSSetShader(mPixelShader, nullptr, 0);

			mVertexConstantBufferPerModel.Update(context);
			mPixelConstantBufferPerSet.Update(context);
			mGeometryConstantBufferPerCamera.Update(context);

			context->VSSetConstantBuffers(0, 1, &mVertexConstantBufferPerModel.mBuffer);
			context->PSSetConstantBuffers(0, 1, &mPixelConstantBufferPerSet.mBuffer);
			context->GSSetConstantBuffers(0, 1, &mGeometryConstantBufferPerCamera.mBuffer);
		}

		void LM_VECTOR_CALL ViewProj(matrix Matrix, ID3D11DeviceContext* context)
		{
			mGeometryConstantBufferPerCamera.gViewProj = Transpose(Matrix);
			if (context)
				mGeometryConstantBufferPerCamera.Update(context);
		}
		void LM_VECTOR_CALL World(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexConstantBufferPerModel.World = Transpose(Matrix);
			vector pDet;
			mVertexConstantBufferPerModel.WorldInvTranspose = Inverse(pDet, Matrix);
			if (context)
				mVertexConstantBufferPerModel.Update(context);
		}
		void Color(const float4& color, ID3D11DeviceContext* context)
		{
			mPixelConstantBufferPerSet.gColor = color;
			if (context)
				mPixelConstantBufferPerSet.Update(context);
		}
	private:
		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11GeometryShader* mGeometryShader = nullptr;
		ID3D11PixelShader* mPixelShader = nullptr;

		ShaderConstantBuffer<VScbPerModel> mVertexConstantBufferPerModel;
		ShaderConstantBuffer<GScbPerCamera> mGeometryConstantBufferPerCamera;
		ShaderConstantBuffer<PScbPerSet> mPixelConstantBufferPerSet;
	};

	void EffectNormalLine::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->Apply(
			context
			);
	}

	void EffectNormalLine::ViewProj(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->ViewProj(
			load(matrix), context
			);
	}
	void EffectNormalLine::World(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->World(
			load(matrix), context
			);
	}

	void LM_VECTOR_CALL EffectNormalLine::ViewProj(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->ViewProj(
			Matrix, context
			);
	}
	void LM_VECTOR_CALL EffectNormalLine::World(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->World(
			Matrix, context
			);
	}

	void EffectNormalLine::Color(const float4& color, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectNormalLineDelegate*>(this));

		return ((EffectNormalLineDelegate*)this)->Color(
			color, context
			);
	}

	const std::unique_ptr<EffectNormalLine>& EffectNormalLine::GetInstance(ID3D11Device* device, const float4& color)
	{
		static auto mInstance = std::unique_ptr<EffectNormalLine>(new EffectNormalLineDelegate(device, color));
		return mInstance;
	}
#pragma endregion

	std::unique_ptr<context_wrapper::state> context_wrapper::swap_states[2] =
	{ std::make_unique<context_wrapper::state>(), std::make_unique<context_wrapper::state>() };

#ifndef PROFILE
	std::wstring context_wrapper::effect_name;
#endif
	inline void STDMETHODCALLTYPE context_wrapper::PSSetShader(
		/* [annotation] */
		_In_opt_  ID3D11PixelShader *pPixelShader,
		/* [annotation] */
		_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
		UINT NumClassInstances)
	{
		std::uint64_t f = 1;
		f <<= state::pssetshader;
		swap_states[1]->f |= f;
		context->PSSetShader(pPixelShader, ppClassInstances, NumClassInstances);
	}

	void STDMETHODCALLTYPE context_wrapper::GSSetShader(
		/* [annotation] */
		_In_opt_  ID3D11GeometryShader *pShader,
		/* [annotation] */
		_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
		UINT NumClassInstances)
	{
		std::uint64_t f = 1;
		f <<= state::gssetshader;
		swap_states[1]->f |= f;
		context->GSSetShader(pShader, ppClassInstances, NumClassInstances);
	}

	void STDMETHODCALLTYPE context_wrapper::HSSetShader(
		/* [annotation] */
		_In_opt_  ID3D11HullShader *pShader,
		/* [annotation] */
		_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
		UINT NumClassInstances)
	{
		std::uint64_t f = 1;
		f <<= state::hssetshader;
		swap_states[1]->f |= f;
		context->HSSetShader(pShader, ppClassInstances, NumClassInstances);
	}

	void STDMETHODCALLTYPE context_wrapper::DSSetShader(
		/* [annotation] */
		_In_opt_  ID3D11DomainShader *pShader,
		/* [annotation] */
		_In_reads_opt_(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
		UINT NumClassInstances)
	{
		std::uint64_t f = 1;
		f <<= state::dssetshader;
		swap_states[1]->f |= f;
		context->DSSetShader(pShader, ppClassInstances, NumClassInstances);
	}

	void STDMETHODCALLTYPE context_wrapper::OMSetRenderTargets(
		/* [annotation] */
		_In_range_(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)  UINT NumViews,
		/* [annotation] */
		_In_reads_opt_(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
		/* [annotation] */
		_In_opt_  ID3D11DepthStencilView *pDepthStencilView)
	{
		std::uint64_t f = 0;
		for (std::uint8_t i = 0; i != NumViews; ++i)
			f |= (static_cast<std::uint64_t>(1) << (state::omsetrendertargets + i));
		swap_states[1]->f |= f;
		context->OMGetRenderTargets(NumViews, reinterpret_cast<ID3D11RenderTargetView**>(&swap_states[1]->ptr[state::omsetrendertargets - 1].p), reinterpret_cast<ID3D11DepthStencilView**>(&swap_states[1]->ptr[state::omsetdepthstencilview - 1].p));
		if(!pDepthStencilView)
			context->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
		else
			context->OMSetRenderTargets(NumViews, ppRenderTargetViews,*reinterpret_cast<ID3D11DepthStencilView**>(&swap_states[1]->ptr[state::omsetdepthstencilview - 1].p));

	}

	inline void STDMETHODCALLTYPE context_wrapper::OMSetBlendState(
		/* [annotation] */
		_In_opt_  ID3D11BlendState *pBlendState,
		/* [annotation] */
		_In_opt_  const FLOAT BlendFactor[4],
		/* [annotation] */
		_In_  UINT SampleMask)
	{
		std::uint64_t f = 1;
		f <<= state::omsetblendstate;
		swap_states[1]->f |= f;
		context->OMSetBlendState(pBlendState, BlendFactor, SampleMask);
	}

#ifdef PROFILE
	inline
#endif
		void STDMETHODCALLTYPE context_wrapper::OMSetDepthStencilState(
		/* [annotation] */
		_In_opt_  ID3D11DepthStencilState *pDepthStencilState,
		/* [annotation] */
		_In_  UINT StencilRef)
	{
		std::uint64_t f = 1;
		f <<= state::omsetdepthstencilstate;
		swap_states[1]->f |= f;
#ifndef PROFILE
		ID3D11DepthStencilState* p = nullptr;
		context->OMGetDepthStencilState(&p, &swap_states[1]->ptr[state::stencil_ref - 1].n);
		//dx::DebugCOM(p, "context_wrapper::OMSetDepthStencilState");
		leo::win::ReleaseCOM(p);
#endif
		context->OMSetDepthStencilState(pDepthStencilState, StencilRef);
	}

	void STDMETHODCALLTYPE context_wrapper::SOSetTargets(
		/* [annotation] */
		_In_range_(0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumBuffers,
		/* [annotation] */
		_In_reads_opt_(NumBuffers)  ID3D11Buffer *const *ppSOTargets,
		/* [annotation] */
		_In_reads_opt_(NumBuffers)  const UINT *pOffsets)
	{
		std::uint64_t f = 1;
		f <<= state::sosettargets;
		swap_states[1]->f |= f;
		context->SOSetTargets(NumBuffers, ppSOTargets, pOffsets);
	}

	void STDMETHODCALLTYPE context_wrapper::RSSetState(
		/* [annotation] */
		_In_opt_  ID3D11RasterizerState *pRasterizerState)
	{
		std::uint64_t f = 1;
		f <<= state::rssetstate;
		swap_states[1]->f |= f;
		context->RSSetState(pRasterizerState);
	}

	void STDMETHODCALLTYPE context_wrapper::RSSetViewports(
		/* [annotation] */
		_In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
		/* [annotation] */
		_In_reads_opt_(NumViewports)  const D3D11_VIEWPORT *pViewports)
	{
		std::uint64_t f = 0;
		for (std::uint8_t i = 0; i != NumViewports; ++i)
			f |= (static_cast<std::uint64_t>(1) << (state::rssetviewprots + i));
		swap_states[1]->f |= f;
		UINT num = NumViewports;
		context->RSGetViewports(&num, &swap_states[1]->ptr[state::rssetviewprots - 1].v);
		context->RSSetViewports(NumViewports, pViewports);
	}

	void STDMETHODCALLTYPE context_wrapper::RSSetScissorRects(
		/* [annotation] */
		_In_range_(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
		/* [annotation] */
		_In_reads_opt_(NumRects)  const D3D11_RECT *pRects)
	{
		std::uint64_t f = 0;
		for (std::uint8_t i = 0; i != NumRects; ++i)
			f |= (static_cast<std::uint64_t>(1) << (state::rssetscissorrects + i));
		swap_states[1]->f |= f;
		UINT num = NumRects;
		context->RSGetScissorRects(&num, (&swap_states[1]->ptr[state::rssetscissorrects - 1].r));
		context->RSSetScissorRects(NumRects, pRects);
	}

	void context_wrapper::apply_swap()
	{
		std::uint64_t f = (swap_states[0]->f ^ swap_states[1]->f) & swap_states[0]->f;

		auto and = [&](std::uint8_t mask)
		{
			return (f &(static_cast<std::uint64_t>(1) << mask)) != 0;
		};

		auto count = [&](state::state_type beg, state::state_type end)
		{
			std::uint8_t _count = 0;
			for (std::uint8_t i = beg; i <= end; ++i)
				if (and (i))
					++_count;
			return _count;
		};

		if (and (state::omsetrendertargets))
		{
			auto numrt = count(state::omsetrendertargets, state::end_omsetrendertargets);
			auto pRTV = reinterpret_cast<ID3D11RenderTargetView**>(&swap_states[0]->ptr[state::omsetrendertargets - 1].p);
			auto pDSV = reinterpret_cast<ID3D11DepthStencilView*>(swap_states[0]->ptr[state::omsetdepthstencilview - 1].p);
			context->OMSetRenderTargets(numrt, pRTV, pDSV);
			for (uint32 i = 0; i != numrt; ++i)
				ReleaseCOM(pRTV[i],std::false_type());
			ReleaseCOM(pDSV, std::false_type());
		}

		if (and (state::rssetviewprots))
		{
			auto numvp = count(state::rssetviewprots, state::end_rssetviewprots);
			context->RSSetViewports(numvp, (&swap_states[0]->ptr[state::rssetviewprots - 1].v));
		}

		if (and (state::rssetscissorrects))
		{
			auto numsr = count(state::rssetscissorrects, state::end_rssetscissorrects);
			context->RSSetScissorRects(numsr, (&swap_states[0]->ptr[state::rssetscissorrects - 1].r));
		}

		if (and (state::rssetstate))
			context->RSSetState(nullptr);
		if (and (state::pssetshader))
			context->PSSetShader(nullptr, nullptr, 0);
		if (and (state::hssetshader))
			context->HSSetShader(nullptr, nullptr, 0);
		if (and (state::dssetshader))
			context->DSSetShader(nullptr, nullptr, 0);
		if (and (state::gssetshader))
			context->GSSetShader(nullptr, nullptr, 0);
		if (and (state::omsetblendstate))
			context->OMSetBlendState(nullptr, nullptr, 0);
		if (and (state::sosettargets))
			context->SOSetTargets(0, nullptr, nullptr);
		if (and (state::omsetdepthstencilstate))
		{
#ifndef PROFILE
			UINT StencilRef = swap_states[0]->ptr[state::stencil_ref - 1].n;
#else
			const UINT StencilRef = 0;
#endif
			context->OMSetDepthStencilState(nullptr, StencilRef);
		}

		std::swap(swap_states[0], swap_states[1]);
		//BUG修复,状态泄露
		swap_states[1]->f = 0U;
		std::memset(swap_states[1]->ptr, 0, sizeof(swap_states[1]->ptr));
#ifndef PROFILE
		//DebugPrintf(L"%s effect apply\n", effect_name.c_str());
		//Debug 用途,定位当前效果
		effect_name;
#endif
	}




}