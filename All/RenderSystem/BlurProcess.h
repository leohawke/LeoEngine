#ifndef ShaderSystem_BlurProcess_Hpp
#define ShaderSystem_BlurProcess_Hpp

#include "PostProcess.hpp"

namespace leo
{
	class SeparableGaussianFilterProcess :public PostProcess {
	public:
		SeparableGaussianFilterProcess(ID3D11Device* create, int kernel_radius, float multiplier,bool x_dir);

		void InputPin(ID3D11DeviceContext* context, uint32 width, uint32 height, uint32 format) override;

		void Apply(ID3D11DeviceContext*) override;
	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalcSampleOffsets(uint32 tex_size, float deviation);
	protected:
		int mKernelRadius;
		float mMuliplier;
		bool mXDir;

		ID3D11VertexShader* mBlurVS;
		ID3D11SamplerState* bilinear_sampler;


		//[0,1] src_tex_size
		//[4,11] color_weight
		//[12,19] tex_coord_offset;
		float cpu_params[20];
		win::unique_com<ID3D11Buffer> gpu_params;
	};

	template<typename T>
	class BlurPorcess :public PostProcessChain {
	public:
		BlurPorcess(ID3D11Device* create, int kernel_radius, float multiplier)
		{
			Append(std::make_shared<SeparableGaussianFilterProcess>(create, kernel_radius, multiplier, true));
			Append(std::make_shared<SeparableGaussianFilterProcess>(create, kernel_radius, multiplier, false));
		}
	};

}

#endif
