//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/MeshData.hpp
//  Version:     v1.00
//  Created:     11/6/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供网格数据结构
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_MeshData_Hpp
#define Core_MeshData_Hpp



#include "..\IndePlatform\\leoint.hpp"

#include "Vertex.hpp"
#include "..\LightBuffer.h"
#include<vector>

struct ID3D11ShaderResourceView;
struct ID3D11Buffer;


namespace leo{
	//?需要保存网格中的相关数据吗? no
	//?这个结构应该释放自己的资源吗? yes
	//包围模型暂时不加QAQ

	struct MeshData{
		struct SubSetData{
			Material mMat;
			ID3D11ShaderResourceView* mDiff;
			ID3D11ShaderResourceView* mNormalMap;
			uint32 mIndexOffset;
			uint32 mIndicesCount;
			//std::wstring mDiffName;
			//std::wstring mNormalMapName;
		};
		//顶点信息,GPU
		ID3D11Buffer* mNormalMapVB;
		//索引信息,GPU
		ID3D11Buffer* mIB;

		//索引信息,CPU
		//std::vector<uint32> mIndices;
		//顶点信息,CPU
		//std::vector<Vertex::NormalMap> mVertexs;

		std::vector<SubSetData> mSubSets;
	};
}


#endif