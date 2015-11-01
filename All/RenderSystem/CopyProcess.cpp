#include "CopyProcess.hpp"

class PointCopy :public leo::PostProcess {
public:
	PointCopy(ID3D11Device* create)
		:PostProcess(create)
	{}
};

class BilinearCopy :public leo::PostProcess {
public:
	BilinearCopy(ID3D11Device* create)
		:PostProcess(create)
	{}
};

class AddPointCopy :public PointCopy {
public:
	AddPointCopy(ID3D11Device* create)
		:PointCopy(create)
	{}
};

class AddBilinearCopy :public BilinearCopy {
public:
	AddBilinearCopy(ID3D11Device* create)
		:BilinearCopy(create)
	{}
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
