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
	class HDRProcess :PostProcess {
	private:
		class HDRCommon:PostProcess{
		public:
			HDRCommon(ID3D11Device* create);

			void Apply(ID3D11DeviceContext* context) override;
		protected:
			void GetSampleOffset(UINT width,UINT height);
		private:
			static ID3D11Buffer* mGpuParams ;//sizeof = float4[2]
			static ID3D11VertexShader* mLumVS;

			static ID3D11SamplerState* src_sampler;
			static ID3D11SamplerState* last_lum_sampler;

			std::array<float4, 2> mCpuParams;
		};
		class LumLogProcess : public HDRCommon {
		public:
			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override;
		private:
			ID3D11PixelShader* mLumLogPS = nullptr;
		};
		class LumIterativeProcess : public HDRCommon {
		public:
			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override;
		private:
			ID3D11PixelShader* mLumIterativePS = nullptr;
		};
		class LumAdaptedProcess : public HDRCommon {
		public:
			void Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) override;
		private:
			ID3D11PixelShader* mLumAdaptedPS = nullptr;
		};

		
		class IHDRBundleProcess {

		};

		//note:this class create/update/set LumAdaptedProcess pixel shader constants
		class HDRBundleProcess :public IHDRBundleProcess {

		};

		//TODO :impl this
		class HDRBundleCSProcess :public IHDRBundleProcess {
		};
	};
}
