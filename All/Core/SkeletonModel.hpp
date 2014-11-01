////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Joint.hpp
//  Version:     v1.00
//  Created:     10/29/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层模型渲染,数据载入
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_SkeletonModel_Hpp
#define Core_SkeletonModel_Hpp

#include "SkeletonAnimation.hpp"
#include <string>

struct ID3D11Device;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11DeviceContext;
struct ID3D11Device;
namespace leo{
	class Camera;
	class Mesh;
	class SkeletonModel{

		leo::Mesh mMesh;

		ID3D11Buffer* mAnimationDataBUffer;

		std::shared_ptr<Skeleton> mSkeleton;
		//一个骨骼不止一个动画
		std::map<std::size_t,Animation> mAnimations;
	public:
		void LoadFromFile(const std::wstring& filename,ID3D11Device* device);

		void Render(ID3D11DeviceContext* context, const Camera& camera);

		void Update();
	};
}

#endif