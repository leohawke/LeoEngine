////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Skeleton.h
//  Version:     v1.00
//  Created:     10/27/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层相关接口
// -------------------------------------------------------------------------
//  History:
//				11/05/2014 接口,逻辑层移入底层
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_Skeleton_Hpp
#define Core_Skeleton_Hpp

#include<memory>

#include "..\IndePlatform\LeoMath.h"
#include "..\IndePlatform\ldef.h"
struct ID3D11DeviceContext;

namespace leo{
	//骨骼数据(顶点信息,材质信息,动画信息)
	struct SkeletonData;

	//骨骼实例(骨骼数据可被多个实例共享)
	class SkeletonInstance;

	//未实现
	class SkeletonGroup;

	class Camera;

	class SkeletonInstance{
		//共享的骨骼数据
		std::shared_ptr<SkeletonData> mSkeData;
		//动画索引(名字hash后的值)
		std::size_t  mAniIndex;
		//标准时间点(range[0.f,1.f])
		float mNorT;
		//蒙皮矩阵
		std::unique_ptr<float4x4[]> mSkinMatrix;
	public:
		SkeletonInstance(const std::shared_ptr<SkeletonData>& skeData);
		~SkeletonInstance();

		bool SwitchAnimation(const std::wstring& aniName);
		bool SwitchAnimation(const wchar_t* aniName);

		void Update();
		void Render(ID3D11DeviceContext* context, const Camera& camera);
	};

	struct SkeletonData{
		//从文件载入<参数:文件名>
		static std::shared_ptr<SkeletonData> Load(const std::wstring& fileName);
		static std::shared_ptr<SkeletonData> Load(const wchar_t* fileName);
		//从内存载入,未实现
		static std::shared_ptr<SkeletonData> Load(const std::unique_ptr<stdex::byte[]>& data, std::size_t dataSize);
	};
}

#endif