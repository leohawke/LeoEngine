#include "EffectSkeleton.hpp"
#include "Vertex.hpp"
#include "EngineConfig.h"
#include "FileSearch.h"
#include "RenderSystem\ShaderMgr.h"
#include "RenderSystem\RenderStates.hpp"
#include <d3dcompiler.h>

#include <atomic>
#pragma comment(lib,"d3dcompiler.lib")
namespace leo{

	using vector = __m128;
	using matrix = std::array<vector, 4>;

	class EffectSkeletonDelegate :CONCRETE(EffectSkeleton), public Singleton<EffectSkeletonDelegate>
	{
	public:
		EffectSkeletonDelegate(ID3D11Device* device)
			:mVertexShaderConstantBufferPerFrame(device),mVertexShaderConstantBufferPerSkin(device),
			 mPixelShaderConstantBufferPerFrame(device), mPixelShaderConstantBufferPerPrimitive(device),
			 mPixelShaderConstantBufferPerView(device)
		{
			leo::ShaderMgr sm;
			ID3D11InputLayout* layout;
			mVertexShader = sm.CreateVertexShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"skeleton", D3D11_VERTEX_SHADER)), nullptr, InputLayoutDesc::Skinned,arrlen(InputLayoutDesc::Skinned), &layout);

			device->CreateClassLinkage(&mPixelShaderClassLinkage);
			auto blob = sm.CreateBlob(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"skeleton", D3D11_PIXEL_SHADER)));
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
		}

		~EffectSkeletonDelegate()
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
			mVertexShaderConstantBufferPerSkin.Update(context);

			ID3D11Buffer* mvscbs[] = {
				mVertexShaderConstantBufferPerFrame.mBuffer,
				mVertexShaderConstantBufferPerSkin.mBuffer
			};
			pContext.VSSetShader(mVertexShader, nullptr, 0);
			pContext.VSSetConstantBuffers(0, 2, mvscbs);


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
				mPixelShaderNormalMapSRV
			};

			mInstances[_gAbsLight_mInstanceIndex] = mLightInstances[static_cast<uint8>(mLightType)];
			pContext.PSSetShader(mPixelShader, mInstances.get(), mNumofInstance);
			pContext.PSSetConstantBuffers(0, 3, mpscbs);
			pContext.PSSetShaderResources(0, 2, mpssrvs);
			pContext.PSSetSamplers(0, 1, &mPixelShaderSampleState);
		}

		void  LM_VECTOR_CALL WorldMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			vector pDet;
			mVertexShaderConstantBufferPerFrame.worldinvtranspose = Inverse(pDet, Matrix);
			mVertexShaderConstantBufferPerFrame.world = Transpose(Matrix);
			if (context)
				mVertexShaderConstantBufferPerFrame.Update(context);
		}
		void LM_VECTOR_CALL WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
		{
			mVertexShaderConstantBufferPerFrame.worldviewproj = Transpose(Matrix);
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

			if (context){
				ID3D11ShaderResourceView* mpssrvs[] = {
					mPixelShaderDiffuseSRV,
					mPixelShaderNormalMapSRV
				};
				context->PSSetShaderResources(0, 2, mpssrvs);
			}
		}
		void NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
		{
			mPixelShaderNormalMapSRV = nmap;
			if (context){
				ID3D11ShaderResourceView* mpssrvs[] = {
					mPixelShaderDiffuseSRV,
					mPixelShaderNormalMapSRV
				};
				context->PSSetShaderResources(0, 2, mpssrvs);
			}
		}

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow
		{
			return true;
		}

		void SkinMatrix(float4x4Object * globalmatrix , std::uint32_t numJoint){
			mNumJoint = numJoint;
			for (auto i = 0u; i != numJoint; ++i){
				mVertexShaderConstantBufferPerSkin.SkinMatrix[i] = Transpose(load(static_cast<const float4x4&>(globalmatrix[i])));
			}
		}
	private:
	private:
		struct VScbPerFrame
		{
			matrix world;
			matrix worldinvtranspose;
			matrix worldviewproj;
		public:
			const static std::uint8_t slot = 0;
		};
		ShaderConstantBuffer<VScbPerFrame> mVertexShaderConstantBufferPerFrame;
		struct VScbPerSkin
		{
			matrix SkinMatrix[96];
		public:
			const static std::uint8_t slot = 1;
		};
		ShaderConstantBuffer<VScbPerSkin> mVertexShaderConstantBufferPerSkin;
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
		struct{
			std::unique_ptr<ID3D11ClassInstance*[]> mInstances = nullptr;
			std::uint8_t mNumofInstance = 0;
		};

		std::uint8_t _gAbsLight_mInstanceIndex = 0;

		ID3D11VertexShader* mVertexShader = nullptr;
		ID3D11PixelShader*	mPixelShader = nullptr;
		ID3D11SamplerState* mPixelShaderSampleState = nullptr;
		//Release
		ID3D11ClassLinkage* mPixelShaderClassLinkage = nullptr;

		ID3D11ShaderResourceView *mPixelShaderDiffuseSRV = nullptr;
		ID3D11ShaderResourceView *mPixelShaderNormalMapSRV = nullptr;

		std::uint32_t mNumJoint = 0;
	};

	const std::unique_ptr<EffectSkeleton>& EffectSkeleton::GetInstance(ID3D11Device* device)
	{
		static auto mInstance = std::unique_ptr<EffectSkeleton>(new EffectSkeletonDelegate(device));
		return mInstance;
	}

	void EffectSkeleton::Apply(ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->Apply(
			context
			);
	}

	void EffectSkeleton::WorldMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->WorldMatrix(
			load(matrix), context
			);
	}

	void LM_VECTOR_CALL EffectSkeleton::WorldMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->WorldMatrix(
			Matrix, context
			);
	}

	void EffectSkeleton::WorldViewProjMatrix(const float4x4& matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->WorldViewProjMatrix(
			load(matrix), context
			);
	}

	void LM_VECTOR_CALL EffectSkeleton::WorldViewProjMatrix(matrix Matrix, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->WorldViewProjMatrix(
			Matrix, context
			);
	}

	void EffectSkeleton::EyePos(const float3& pos, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->EyePos(
			pos, context
			);
	}

	void EffectSkeleton::Light(const DirectionalLight& dl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->Light(
			dl, context
			);
	}

	void EffectSkeleton::Light(const PointLight& pl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->Light(
			pl, context
			);
	}

	void EffectSkeleton::Light(const SpotLight& sl, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->Light(
			sl, context
			);
	}

	void EffectSkeleton::Mat(const Material& mat, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->Mat(
			mat, context
			);
	}

	void EffectSkeleton::DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->DiffuseSRV(
			diff, context
			);
	}

	void EffectSkeleton::NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context)
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->NormalMapSRV(
			nmap, context
			);
	}

	bool EffectSkeleton::SetLevel(EffectConfig::EffectLevel l)  lnothrow
	{
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->SetLevel(
			l
			);
	}

	void EffectSkeleton::SkinMatrix(float4x4Object* globalmatrix, std::uint32_t numJoint){
		lassume(dynamic_cast<EffectSkeletonDelegate *>(this));

		return ((EffectSkeletonDelegate *)this)->SkinMatrix(globalmatrix, numJoint
			);
	}
}