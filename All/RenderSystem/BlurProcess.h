#ifndef ShaderSystem_BlurProcess_Hpp
#define ShaderSystem_BlurProcess_Hpp

#include "PostProcess.hpp"

namespace leo
{
	class SeparableGaussianFilterProcess :public PostProcess {
	public:
		SeparableGaussianFilterProcess(ID3D11Device* create, int kernel_radius, float multiplier,bool x_dir);

		void Apply(ID3D11DeviceContext*) override;
	protected:
		float GaussianDistribution(float x, float y, float rho);
		void CalcSampleOffsets(uint32 tex_size, float deviation);
	protected:
		int mKernelRadius;
		float mMuliplier;
		bool mXDir;

		//src_tex_size_ep_;
		//color_weight_ep;
		//tex_coord_offset_ep;
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
