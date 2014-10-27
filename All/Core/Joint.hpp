////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Joint.hpp
//  Version:     v1.01
//  Created:     8/15/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层相关逻辑
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Joint_Hpp
#define Core_Joint_Hpp

#include "..\IndePlatform\LeoMath.h"

namespace leo{
	struct Joint{
		float4x4 mInvBindPose;
		std::size_t mName;
	};
}

#endif