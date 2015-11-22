#ifndef ShaderSystem_HDRProcess_Hpp
#define ShaderSystem_HDRProcess_Hpp
#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <platform.h>
#include <Core\COM.hpp>
#include <exception.hpp>
#include "d3dx11.hpp"
#include "RenderStates.hpp"
#include "PostProcess.hpp"

namespace leo {
	class HDRProcess :public PostProcess {
	private:
		class HDRCommon:public PostProcess{
		public:
			HDRCommon(ID3D11Device* create);

			~HDRCommon();
			
			void Input(UINT width, UINT height);

			void Apply(ID3D11DeviceContext* context) override;
		protected:
			void GetSampleOffset(UINT width,UINT height);

			std::array<float4, 2> mCpuParams;

			D3D11_VIEWPORT mViewPort;

			CD3D11_TEXTURE2D_DESC mTexDesc;

			UINT mWidth, mHeight;

			static ID3D11Buffer* mGpuParams;//sizeof = float4[2]

			static std::size_t	mRefCount;
		private:
			
			static ID3D11VertexShader* mLumVS;

			static ID3D11SamplerState* src_sampler;
			static ID3D11SamplerState* last_lum_sampler;

		};
		class LumLogProcess : public HDRCommon {
		public:
			LumLogProcess(ID3D11Device* create,unsigned level);

			ID3D11ShaderResourceView* Output(UINT& width, UINT& height) const;

			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView*) override;
		private:
			win::unique_com<ID3D11RenderTargetView> mLumLogRTV = nullptr;
			win::unique_com<ID3D11ShaderResourceView> mLumLogOutput = nullptr;
		};
		class LumIterativeProcess : public HDRCommon {
		public:
			LumIterativeProcess(ID3D11Device* create,unsigned level);

			~LumIterativeProcess();

			ID3D11ShaderResourceView* Output(UINT& width, UINT& height) const;

			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView*) override;
		private:
			win::unique_com<ID3D11RenderTargetView> mLumIterRTV = nullptr;
			win::unique_com<ID3D11ShaderResourceView> mLumIterOutput = nullptr;
		};
		class LumAdaptedProcess : public HDRCommon {
		public:
			LumAdaptedProcess(ID3D11Device* create);

			ID3D11ShaderResourceView* Output() const;
			
			void SetFrameDelta(float dt);

			//!note this function will clear rt
			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override;
		private:
			bool mIndex = false;
			win::unique_com<ID3D11RenderTargetView> mLumAdaptedSwapRTV[2] = {nullptr,nullptr};
			win::unique_com<ID3D11ShaderResourceView> mLumAdaptedSwapOutput[2] = { nullptr,nullptr };

			win::unique_com<ID3D11Buffer> mDeltaGpuParams;
			float4 mDeltaCpuParams;
		};

		
		class IHDRStatProcess {
		public:
			virtual ~IHDRStatProcess();

			virtual void Apply(ID3D11DeviceContext*,float dt) = 0;

			virtual void Input(ID3D11Device*,ID3D11Texture2D* tex) = 0;

			virtual ID3D11ShaderResourceView* Output() = 0;
		};

		class HDRStatProcess :public IHDRStatProcess {
		public:
			HDRStatProcess(ID3D11Device* create, ID3D11Texture2D* tex);

			void Apply(ID3D11DeviceContext*,float dt) override;

			ID3D11ShaderResourceView* Output() override;

			void Input(ID3D11Device*, ID3D11Texture2D* tex) override;
		private:
			leo::win::unique_com<ID3D11ShaderResourceView> mSrcSRV = nullptr;

			leo::win::unique_com<ID3D11ShaderResourceView> mScale = nullptr;
			leo::win::unique_com<ID3D11RenderTargetView> mScaleRT = nullptr;
			
			std::shared_ptr<PostProcess> mCopyProcess;

			std::unique_ptr<LumLogProcess> pLumFirst;
		  	std::vector<std::unique_ptr<LumIterativeProcess>> pLumIterVec;
			std::unique_ptr<LumAdaptedProcess> pLumFinal;

			D3D11_VIEWPORT mViewPort;

			UINT mWidth, mHeight;

			UINT mMaxLevel;
		};

		//TODO :impl this
		class HDRStatCSProcess :public IHDRStatProcess {
		};

		class SqrBright:public PostProcess {
		public:
			SqrBright(ID3D11Device* create);

			void Apply(ID3D11DeviceContext* context) override;
		private:
			ID3D11SamplerState* bilinear_sampler;
		};

		class GlowMerger:public PostProcess {
		public:
			GlowMerger(ID3D11Device* create);

			void Apply(ID3D11DeviceContext* context) override;
		private:
			ID3D11SamplerState* bilinear_sampler;
		};

		class IHDRLensPorcess {
		public:
			virtual ~IHDRLensPorcess();

			virtual void Apply(ID3D11DeviceContext*) = 0;

			virtual void Input(ID3D11Device*, ID3D11Texture2D* tex) = 0;

			virtual ID3D11ShaderResourceView* Output() = 0;
		};

		class HDRLensProcess : public IHDRLensPorcess {
		public:
			HDRLensProcess(ID3D11Device* create, ID3D11Texture2D* tex);

			void Apply(ID3D11DeviceContext*) override;

			ID3D11ShaderResourceView* Output() override;

			void Input(ID3D11Device*, ID3D11Texture2D* tex) override;
		private:
			
			std::unique_ptr<SqrBright> bright_pass_downsampler;
			std::shared_ptr<PostProcess> downsamplers[2];
			std::shared_ptr<PostProcessChain> blurs[3];
			std::unique_ptr<GlowMerger> glow_merger;
		private:
			leo::win::unique_com<ID3D11ShaderResourceView> bright_pass_input;
			std::pair<uint16, uint16> bright_pass_output_size;
			leo::win::unique_com<ID3D11RenderTargetView> bring_pass_output;
			leo::win::unique_com<ID3D11RenderTargetView> downsampler_output[2];
			leo::win::unique_com<ID3D11ShaderResourceView> blur_inputs[3];

			leo::win::unique_com<ID3D11RenderTargetView> blur_outputs[3];
			leo::win::unique_com<ID3D11ShaderResourceView> glow_input[3];

			leo::win::unique_com<ID3D11RenderTargetView> glow_output;
			leo::win::unique_com<ID3D11ShaderResourceView> lens_output;
		};

		//TODO :impl this
		class HDRFFTLensProcess : public IHDRLensPorcess {
		};
	public:
		HDRProcess(ID3D11Device* create,ID3D11Texture2D* src);

		void SetFrameDelta(float dt);

		void Apply(ID3D11DeviceContext* context) override;

		void ReSize(ID3D11Device* create, ID3D11Texture2D* src);
	private:
		std::unique_ptr<IHDRStatProcess> mStatProcess;
		std::unique_ptr<IHDRLensPorcess> mLensProcess;

		win::unique_com<ID3D11Buffer> mToneGpuParams;
		float2 mToneCpuParams;

		float mDt;
	};
}

#endif
