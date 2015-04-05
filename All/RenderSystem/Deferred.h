////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   ShaderSystem/Deferred.h
//  Version:     v1.00
//  Created:     03/16/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 延迟渲染
// -------------------------------------------------------------------------
//  History:
//			Todo : DepthStencil属于延迟资源
////////////////////////////////////////////////////////////////////////////


#ifndef ShaderSystem_Deferred_H
#define ShaderSystem_Deferred_H

#include "..\IndePlatform\utility.hpp"
#include "..\IndePlatform\Singleton.hpp"

struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

namespace leo {

	class CameraFrustum;

	class  DeferredResources :public Singleton<DeferredResources>  {
	protected:
		DeferredResources() noexcept;
	public:
		using size_type = std::pair<leo::uint16, leo::uint16>;

		
		~DeferredResources();

		//todo:不再使用无参实例化,而是传入Device对象
		static DeferredResources& GetInstance() noexcept {
			static DeferredResources obj;
			return obj;
		}

		ID3D11RenderTargetView** GetMRTs() const;
		ID3D11ShaderResourceView** GetSRVs() const;

		//GBuffer用作输出的准备动作
		void OMSet() noexcept;
		//GBuffer用作输入的准备动作
		void IASet() noexcept;

		//before call this ,please call IASet
		void ComputerSSAO() noexcept;

		void ReSize(const size_type& size) noexcept;

		void SetFrustum(const CameraFrustum& frustum) noexcept;
	};

}

#endif