// CopyRight 2014. LeoHawke.

#ifndef RenderSystem_d3dx_Hpp
#define RenderSystem_d3dx_Hpp

#include "..\IndePlatform\platform.h"
#include "..\IndePlatform\memory.hpp"
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#ifdef DEBUG
#include <d3d11sdklayers.h>
#ifdef SHADER_TRACE
#include <d3d11shadertracing.h>
#endif
#endif

//Predefine classes
namespace DirectX
{
	class RenderSystem;
	class RenderWindowBase;
	class Texture;
	class TextureManager;
	class DepthBuffer;
	class Driver;
	class DriverList;
	class VideoMode;
	class VideoModeList;
	class GpuProgram;
	class GpuProgramManager;
	class HardwareBufferManager;
	class HardwareIndexBuffer;
	class HLSLProgramFactory;
	class HLSLProgram;
	class VertexDeclaration;
	class Device;
	class HardwareBuffer;
	class HardwarePixelBuffer;

	using GpuProgramPtr = std::shared_ptr<GpuProgram>;
	using HLSLProgramPtr = std::shared_ptr<HLSLProgram>;
	using TexturePtr = std::shared_ptr<Texture>;
}

//Core Function
//Core Interface
namespace DirectX
{
	template<typename COM>
	auto ReleaseCOM(COM* &com)->decltype(com->Release())
	{
		decltype(com->Release())  refCount {};
		if (com)
			refCount = com->Release();
		com = nullptr;
		return refCount;
	}

	//不推荐使用
	template<typename T> class IScope
	{
		IScope(const ScopedCOMObject&) = delete;
		IScope& operator=(const IScope&) = delete;

		T* pointer = nullptr;
	public:
		explicit IScope(T *p = 0) : pointer(p) {}
		~IScope()
		{
			ReleaseCOM(pointer);
		}

		operator bool() const { return bool(pointer); }

		T& operator*() { return *pointer; }
		T* operator->() { return pointer; }
		T** operator&() { return &pointer; }

		operator T*(){ return pointer; }
	};
}


#endif