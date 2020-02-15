/*! \file lmemory.hpp
\ingroup LBase
\brief LeoEngineר�õ��ڴ������

*/

#ifndef LBase_lmemory_hpp
#define LBase_lmemory_hpp 1

#include "memory.hpp"




#include <new>//::operator new
#include <cstdint>//std::uint8_t
#include <cstddef>//std::size_t
#include <array>
#include <cassert>

#include "LBase/lmacro.h"

#if defined LEO_MEMORY_TRACKER
namespace leo
{
	void __memory_track_record_alloc(void * p, std::size_t count, std::uint8_t alignsize, const char* file, int line, const char* funcname, bool placement = false);
	void __memory_track_dealloc_record(void * p, std::uint8_t alignsize, bool placement = false);
}
#endif

namespace leo
{
	//  Description: Implements an aligned allocator for STL
	//               based on the Mallocator (http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx)
	// -------------------------------------------------------------------------
	template<typename T, int AlignSize>
	//���������
	class lalignas(AlignSize) aligned_alloc
	{
		static_assert(AlignSize > 1, "AlignSize must more than 1");
		static_assert(!(AlignSize &(AlignSize - 1)), "AlignSize must be power of 2");
	public:
		// The following will be the same for virtually all allocators
		//��׼����������������Ͷ���ͺ���
		typedef T * pointer;
		typedef const T * const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		pointer address(T& r) const {
			return &r;
		}

		const_pointer address(const T& s) const {
			return &s;
		}

		std::size_t max_size() const {
			return (static_cast<std::size_t>(-1)) / sizeof(value_type);
		}

		template<typename U> struct rebind {
			typedef aligned_alloc<U, AlignSize> other;
		};

		bool operator!=(const aligned_alloc& other) const {
			return !(*this == other);
		}

		void construct(pointer const p, const_reference t) const {
			void * const pv = static_cast<void*>(p);
			new (pv)value_type(t);
		}

		template<typename... ARGS>
		void construct(pointer const p, ARGS&&... args) const {
			void * const pv = static_cast<void*>(p);
			new (pv)value_type(std::forward<ARGS>(args)...);
		}

		void destroy(pointer const p) const;

		//��״̬������
		bool operator==(const aligned_alloc& other) const {
			return true;
		}
	private:
		std::allocator<std::uint8_t> impl;
	public:
		aligned_alloc() = default;
		~aligned_alloc() = default;
		aligned_alloc(const aligned_alloc&) = default;
		template <typename U>
		aligned_alloc(const aligned_alloc<U, AlignSize>&)
		{}
		aligned_alloc& operator=(const aligned_alloc&) = delete;

		// The following will be different for each allocator.
		pointer allocate(const size_t n)
		{
			// The return value of allocate(0) is unspecified.
			// aligned_alloc returns NULL in order to avoid depending
			// on malloc(0)'s implementation-defined behavior
			// (the implementation can define malloc(0) to return NULL,
			// in which case the bad_alloc check below would fire).
			// All allocators can return NULL in this case.
			if (n == 0)
			{
				return nullptr;
			}

			// All allocators should contain an integer overflow check.
			// The Standardization Committee recommends that std::length_error
			// be thrown in case of integer overflow.
			if (n > max_size())
			{
				throw std::length_error("aligned_alloc<T>::allocate() - Integer overflow.");
			}

			// aligned_alloc wraps allocator<T>.allocate().

			auto size = n * sizeof(value_type) + AlignSize;
			std::uintptr_t rawAddress = reinterpret_cast<std::uintptr_t>(impl.allocate(size));
			DebugPrintf(L"align_alloc.allocate:Address: %p,Size: %u,Alignesize: %u\n", rawAddress, size, AlignSize);

			std::uint8_t missalign = AlignSize - (rawAddress&(AlignSize - 1));
			std::uintptr_t alignAddress = rawAddress + missalign;

			std::uint8_t* storemissalign = (std::uint8_t*)(alignAddress) - 1;
			*storemissalign = missalign;

			auto pv = reinterpret_cast<pointer>(alignAddress);
			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
			if (!pv)
			{
				throw std::bad_alloc();
			}
			return pv;
		}

		void deallocate(pointer const p, const size_t n)
		{
			std::uint8_t adjust = *reinterpret_cast<std::uint8_t*>((std::uintptr_t)p - 1);
			std::uint8_t* rawAddress = reinterpret_cast<std::uint8_t*>((std::uintptr_t)p - adjust);
			auto rawn = n * sizeof(value_type) + AlignSize;
			DebugPrintf(L"align_alloc.deallocate Adddress: %p,Alignesize: %u\n", rawAddress, AlignSize);
			impl.deallocate(rawAddress, rawn);
		}

		// The following will be the same for all allocators that ignore hints.

		template <typename U>
		pointer allocate(const std::size_t n, const U* /* const hint */) const
		{
			return allocate(n);
		}
	};

	// The definition of destroy() must be the same for all allocators.
	template <typename T, int AlignSize>
	void aligned_alloc<T, AlignSize>::destroy(pointer const p) const
	{
		p->~T();
	}
}

namespace leo
{
	//ģ��Orge

	//hack�Ӷ��Ϸ����ڴ���Ϊ
	template<typename ALLOC>
	class AllocPolicy
	{
	private:
		ALLOC _impl;
	public:
		inline void * allocate(std::size_t count, const char * file = nullptr, int line = 0, const char* func = 0)
		{
			auto p = _impl.allocate(count);
#if defined LEO_MEMORY_TRACKER
			__memory_track_record_alloc(p, count, 1, file, line, func);
#endif
			return p;
		}
		inline void deallocate(void * p, const char * file = nullptr, int line = 0, const char* func = 0)
		{
#if defined LEO_MEMORY_TRACKER
			__memory_track_dealloc_record(p, 1);
#endif
			_impl.deallocate(p, 0);
		}
	};

	enum class MemoryCategory
	{
		/// General purpose
		MEMCATEGORY_GENERAL = 0,
		/// Geometry held in main memory
		MEMCATEGORY_GEOMETRY = 1,
		/// Animation data like tracks, bone matrices
		MEMCATEGORY_ANIMATION = 2,
		/// Nodes, control data
		MEMCATEGORY_SCENE_CONTROL = 3,
		/// Scene object instances
		MEMCATEGORY_SCENE_OBJECTS = 4,
		/// Other resources
		MEMCATEGORY_RESOURCE = 5,
		/// Scripting
		MEMCATEGORY_SCRIPTING = 6,
		/// Rendersystem structures
		MEMCATEGORY_RENDERSYS = 7,


		// sentinel value, do not use 
		MEMCATEGORY_COUNT = 8
	};

	template<MemoryCategory cate, int AlignSize>
	class CateAlloc {
	public:
		using value_type = stdex::byte;

		std::size_t max_size() const {
			return (static_cast<std::size_t>(-1)) / sizeof(value_type);
		}

		//Todo : loop allocate
		// The following will be different for each allocator.
		void* allocate(const size_t n)
		{
			// The return value of allocate(0) is unspecified.
			// aligned_alloc returns NULL in order to avoid depending
			// on malloc(0)'s implementation-defined behavior
			// (the implementation can define malloc(0) to return NULL,
			// in which case the bad_alloc check below would fire).
			// All allocators can return NULL in this case.
			if (n == 0)
			{
				return nullptr;
			}

			// All allocators should contain an integer overflow check.
			// The Standardization Committee recommends that std::length_error
			// be thrown in case of integer overflow.
			if (n > max_size())
			{
				throw std::length_error("aligned_alloc<T>::allocate() - Integer overflow.");
			}

			// aligned_alloc wraps allocator<T>.allocate().

			auto size = n * sizeof(value_type) + AlignSize;
			std::uintptr_t rawAddress = reinterpret_cast<std::uintptr_t>(::operator new(size));
			DebugPrintf(L"CateAlloc.allocate:Address: %p,Size: %u,Alignesize: %u\n", rawAddress, size, AlignSize);

			std::uint8_t missalign = AlignSize - (rawAddress&(AlignSize - 1));
			std::uintptr_t alignAddress = rawAddress + missalign;

			std::uint8_t* storemissalign = (std::uint8_t*)(alignAddress) - 1;
			*storemissalign = missalign;

			auto pv = reinterpret_cast<void*>(alignAddress);
			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
			if (!pv)
			{
				throw std::bad_alloc();
			}
			return pv;
		}

		void deallocate(void* const p, const size_t n)
		{
			std::uint8_t adjust = *reinterpret_cast<std::uint8_t*>((std::uintptr_t)p - 1);
			std::uint8_t* rawAddress = reinterpret_cast<std::uint8_t*>((std::uintptr_t)p - adjust);
			DebugPrintf(L"CateAlloc.deallocate Adddress: %p,Alignesize: %u\n", rawAddress, AlignSize);
			::operator delete(rawAddress);
		}
	};

	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_GENERAL, 16>> GeneralAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_GEOMETRY, 16>> GeometryAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_ANIMATION, 16>> AnimationAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_SCENE_CONTROL, 16>> SceneCtlAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_SCENE_OBJECTS, 16>> SceneObjAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_RESOURCE, 16>> ResourceAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_SCRIPTING, 16>> ScriptingAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_RENDERSYS, 16>> RenderSysAllocPolicy;


	template<typename AllocPolice>
	class AllocatedObject
	{
	private:
		static AllocPolice impl;
	public:
		explicit AllocatedObject()
		{ }
#if 1
		virtual ~AllocatedObject()
		{ }
#endif
		/// operator new, with debug line info
		void* operator new(size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz, file, line, func);
		}

		void* operator new(size_t sz)
		{
			return impl.allocate(sz);
		}

		/// placement operator new
		void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

		/// array operator new, with debug line info
		void* operator new[](size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz, file, line, func);
		}

			void* operator new[](size_t sz)
		{
			return impl.allocate(sz);
		}

			void operator delete(void* ptr)
		{
			impl.deallocate(ptr);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete(void* ptr, void*)
		{
			impl.deallocate(ptr);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete(void* ptr, const char* file, int line, const char* func)
		{
			impl.deallocate(ptr, file, line, func);
		}

		void operator delete[](void* ptr)
		{
			impl.deallocate(ptr);
		}

			void operator delete[](void* ptr, const char*, int, const char*)
		{
			impl.deallocate(ptr);
		}
	};


	template<typename AllocPolice>
	class DataAllocatedObject
	{
	private:
		static AllocPolice impl;
	public:
		explicit DataAllocatedObject()
		{ }

		/// operator new, with debug line info
		void* operator new(size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz, file, line, func);
		}

		void* operator new(size_t sz)
		{
			return impl.allocate(sz);
		}

		/// placement operator new
		void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

		/// array operator new, with debug line info
		void* operator new[](size_t sz, const char* file, int line, const char* func)
		{
			return impl.allocate(sz, file, line, func);
		}

			void* operator new[](size_t sz)
		{
			return impl.allocate(sz);
		}

			void operator delete(void* ptr)
		{
			impl.deallocate(ptr);
		}

		// Corresponding operator for placement delete (second param same as the first)
		void operator delete(void* ptr, void*)
		{
			impl.deallocate(ptr);
		}

		// only called if there is an exception in corresponding 'new'
		void operator delete(void* ptr, const char* file, int line, const char* func)
		{
			impl.deallocate(ptr, file, line, func);
		}

		void operator delete[](void* ptr)
		{
			impl.deallocate(ptr);
		}

			void operator delete[](void* ptr, const char*, int, const char*)
		{
			impl.deallocate(ptr);
		}
	};


	typedef AllocatedObject<GeneralAllocPolicy> GeneralAllocatedObject;
	typedef AllocatedObject<GeometryAllocPolicy> GeometryAllocatedObject;
	typedef AllocatedObject<AnimationAllocPolicy> AnimationAllocatedObject;
	typedef AllocatedObject<SceneCtlAllocPolicy> SceneCtlAllocatedObject;
	typedef AllocatedObject<SceneObjAllocPolicy> SceneObjAllocatedObject;
	typedef AllocatedObject<ResourceAllocPolicy> ResourceAllocatedObject;
	typedef AllocatedObject<ScriptingAllocPolicy> ScriptingAllocatedObject;
	typedef AllocatedObject<RenderSysAllocPolicy> RenderSysAllocatedObject;

	typedef ScriptingAllocatedObject    AbstractNodeAlloc;
	typedef AnimationAllocatedObject    AnimableAlloc;
	typedef AnimationAllocatedObject    AnimationAlloc;
	typedef GeneralAllocatedObject      ArchiveAlloc;
	typedef GeneralAllocatedObject      LightAlloc;
	typedef GeometryAllocatedObject     BatchedGeometryAlloc;
	typedef RenderSysAllocatedObject    BufferAlloc;
	typedef GeneralAllocatedObject      CodecAlloc;
	typedef ResourceAllocatedObject     CompositorInstAlloc;
	typedef GeneralAllocatedObject      ConfigAlloc;
	typedef GeneralAllocatedObject      ControllerAlloc;
	typedef GeometryAllocatedObject     DebugGeomAlloc;
	typedef GeneralAllocatedObject      DynLibAlloc;
	typedef GeometryAllocatedObject     EdgeDataAlloc;
	typedef GeneralAllocatedObject      FactoryAlloc;
	typedef SceneObjAllocatedObject     FXAlloc;
	typedef GeneralAllocatedObject      ImageAlloc;
	typedef GeometryAllocatedObject     IndexDataAlloc;
	typedef GeneralAllocatedObject      LogAlloc;
	typedef SceneObjAllocatedObject     MoveableAlloc;
	typedef SceneCtlAllocatedObject     NodeAlloc;
	typedef SceneObjAllocatedObject     OverlayAlloc;
	typedef RenderSysAllocatedObject    GpuParamsAlloc;
	typedef ResourceAllocatedObject     PassAlloc;
	typedef GeometryAllocatedObject     PatchAlloc;
	typedef GeneralAllocatedObject      PluginAlloc;
	typedef GeneralAllocatedObject      ProfilerAlloc;
	typedef GeometryAllocatedObject     ProgMeshAlloc;
	typedef SceneCtlAllocatedObject     RenderQueueAlloc;
	typedef RenderSysAllocatedObject    RenderSysAlloc;
	typedef GeneralAllocatedObject      RootAlloc;
	typedef ResourceAllocatedObject     ResourceAlloc;
	typedef GeneralAllocatedObject      SerializerAlloc;
	typedef SceneCtlAllocatedObject     SceneMgtAlloc;
	typedef ScriptingAllocatedObject    ScriptCompilerAlloc;
	typedef ScriptingAllocatedObject    ScriptTranslatorAlloc;
	typedef SceneCtlAllocatedObject     ShadowDataAlloc;
	typedef GeneralAllocatedObject      StreamAlloc;
	typedef SceneObjAllocatedObject     SubEntityAlloc;
	typedef ResourceAllocatedObject     SubMeshAlloc;
	typedef ResourceAllocatedObject     TechniqueAlloc;
	typedef GeneralAllocatedObject      TimerAlloc;
	typedef ResourceAllocatedObject     TextureUnitStateAlloc;
	typedef GeneralAllocatedObject      UtilityAlloc;
	typedef GeometryAllocatedObject     VertexDataAlloc;
	typedef RenderSysAllocatedObject    ViewportAlloc;
	typedef SceneCtlAllocatedObject     LodAlloc;
	typedef GeneralAllocatedObject      FileSystemLayerAlloc;
	typedef GeneralAllocatedObject      StereoDriverAlloc;
}

namespace leo
{
#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable:4244)
#endif
	//�ڴ��
	struct MemoryChunk {
		std::unique_ptr<stdex::byte[]> mData;
		std::size_t mSize;

		std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
			assert(u64Offset + uBytesToRead <= mSize);
			std::memcpy(pBuffer, &mData[u64Offset], uBytesToRead);
			return uBytesToRead;
		}
		void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite) {
			assert(u64Offset + uBytesToWrite <= mSize);
			std::memcpy(&mData[u64Offset], pBuffer, uBytesToWrite);
		}
	};
#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif
}

namespace leo
{
	template<typename T, typename U>
	void inline memcpy(T& dst, const U& src)
	{
		constexpr auto size = sizeof(T) < sizeof(U) ? sizeof(T) : sizeof(U);
		std::memcpy(&dst, &src, size);
	}

	template<typename T>
	void inline memset(T& dst, int val)
	{
		std::memset(&dst, val, sizeof(T));
	}

	template<typename T>
	void inline BZero(T& dst)
	{
		memset(dst, 0);
	}
}
#endif
