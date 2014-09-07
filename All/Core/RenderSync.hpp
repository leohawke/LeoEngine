////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/RenderSync.hpp
//  Version:     v1.00
//  Created:     8/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 多线程渲染必须的同步对象<单列>
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////


#ifndef Core_RenderSync_hpp
#define Core_RenderSync_hpp

#include "..\IndePlatform\ldef.h"
#include "..\IndePlatform\utility.hpp"
#include "..\IndePlatform\memory.hpp"

namespace leo
{
	class RenderSync : ABSTRACT
	{
	public:
		class Block
		{
		public:
			Block()
			{
				RenderSync::GetInstance()->Wait();
			}
			~Block()
			{
				RenderSync::GetInstance()->Release();
			}
		};
	public:
		void Sync();
		void Present();

		void Wait();
		void Release();

	public:
		static const std::unique_ptr<RenderSync>& GetInstance();
	};
}


#endif