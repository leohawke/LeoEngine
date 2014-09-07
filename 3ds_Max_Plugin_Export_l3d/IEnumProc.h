#ifndef IENUM_PROC_H
#define IENUM_PROC_H

#include "3ds_Max_Plugin_Export_l3d.h"
#include "l3d_mesh.h"
#include <vector>
namespace Tree
{
	class EnumProc : public ITreeEnumProc
	{
	public:
		EnumProc(Interface* ip)
			:ip(ip)
		{

		}
		~EnumProc()
		{}
	public:
		int callback(INode *);
		void Read(INode * node,Object* object);
		void Save(const TCHAR* name);
	private:
		Interface* ip;

		MeshFileHeader l3d_header;
		std::vector<MeshMaterial> l3d_mats;
		std::vector<std::vector<std::uint32_t>> l3d_indexss;
		std::vector<MeshVertex> l3d_vertexs;		
	};
}

#endif