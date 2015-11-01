#ifndef ShaderSystem_CopyProcess_Hpp
#define ShaderSystem_CopyProcess_Hpp

#include "PostProcess.hpp"

namespace leo {
	enum copyprocesss_type {
		point_process,
		bilinear_process,
		point_addprocess,
		bilinear_addprocess
	};

	std::shared_ptr<PostProcess> Make_CopyProcess(ID3D11Device* create, copyprocesss_type type);
}


#endif