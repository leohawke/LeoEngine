// CopyRight 2014. LeoHawke. All rights reserved.

#ifndef IndePlatform_memory_Hpp
#define IndePlatform_memory_Hpp


#include <memory>// smart ptr
#include <cstdlib>
#include <cstring>
#include "leoint.hpp"
//存储和智能指针特性
namespace leo
{
	template<typename _type>
	lconstfn _type*
		//取内建指针
		get_raw(_type* const& p) lnothrow
	{
		return p;
	}
		template<typename _type>
	lconstfn auto
		get_raw(const std::unique_ptr<_type>& p) lnothrow -> decltype(p.get())
	{
		return p.get();
	}
		template<typename _type>
	lconstfn _type*
		get_raw(const std::shared_ptr<_type>& p) lnothrow
	{
		return p.get();
	}
#ifdef LEO_USE_WEAK_PTR
		template<typename _type>
	lconstfn _type*
		get_raw(const std::weak_ptr<_type>& p) lnothrow
	{
		return p.lock().get();
	}
#endif

		template<typename _type>
	inline bool
		reset(std::unique_ptr<_type>& p) lnothrow
	{
		if (p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}
		template<typename _type>
	inline bool
		reset(std::shared_ptr<_type>& p) lnothrow
	{
		if (p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}

		template<typename _type, typename _pSrc>
	lconstfn std::unique_ptr<_type>
		//_pSrc是内建指针
		unique_raw(const _pSrc& p)
	{
		static_assert(is_pointer<_pSrc>::value, "Invalid type found.");

		return std::unique_ptr<_type>(p);
	}

	template<typename _type, typename _pSrc>
	lconstfn std::unique_ptr<_type>
		//_pSrc是内建指针
		unique_raw(_pSrc&& p)
	{
		static_assert(is_pointer<_pSrc>::value, "Invalid type found.");

		return std::unique_ptr<_type>(p);
	}

	template<typename _type>
	lconstfn std::unique_ptr<_type>
		unique_raw(_type* p)
	{
		return std::unique_ptr<_type>(p);
	}

	template<typename _type>
	lconstfn std::unique_ptr<_type>
		unique_raw(nullptr_t) lnothrow
	{
		return std::unique_ptr<_type>();
	}

		template<typename _type, typename _pSrc>
	lconstfn std::shared_ptr<_type>
		share_raw(const _pSrc& p)
	{
		static_assert(is_pointer<_pSrc>::value, "Invalid type found.");

		return std::shared_ptr<_type>(p);
	}

	template<typename _type, typename _pSrc>
	lconstfn std::shared_ptr<_type>
		share_raw(_pSrc&& p)
	{
		static_assert(is_pointer<_pSrc>::value, "Invalid type found.");

		return std::shared_ptr<_type>(p);
	}
	template<typename _type>
	lconstfn std::shared_ptr<_type>
		share_raw(_type* p)
	{
		return std::shared_ptr<_type>(p);
	}

	template<typename _type>
	lconstfn std::shared_ptr<_type>
		share_raw(nullptr_t) lnothrow
	{
		return std::shared_ptr<_type>();
	}

	//Visual C++ 2012以上或c++11以上
#if LB_IMPL_MSCPP >= 1800 || LB_IMPL_CPP > 201103L
	using std::make_unique;
#else
	//make_unique函数
	//使用new和指定参数构造指定类型的std::unique_ptr对象.
	//ref http://herbsutter.com/gotw/_102/
	//ISO WG21 / N3797 20.7.2[memory.syn]
	template<typename _type, typename... _tParams>
	lconstfn limpl(enable_if_t<!is_array<_type>::value, std::unique_ptr<_type>>)
		make_unique(_tParams&&... args)
	{
		return std::unique_ptr<_type>(new _type(lforward(args)...));
	}
	template<typename _type, typename... _tParams>
	lconstfn limpl(enable_if_t<is_array<_type>::value && extent<_type>::value == 0,
		std::unique_ptr<_type >> )
		make_unique(size_t size)
	{
		return std::unique_ptr<_type>(new remove_extent_t<_type>[size]());
	}
	template<typename _type, typename... _tParams>
	limpl(enable_if_t<extent<_type>::value != 0, void>)
		make_unique(_tParams&&...) = delete;
#endif

	//make_shared
	//使用 new 和指定参数构造指针类型的 std::shared_ptr
	// 不同于std::make_shared(会导致目标文件增大)
	template<typename _type, typename... _tParams>
	lconstfn std::shared_ptr<_type>
		make_shared(_tParams&&... args)
	{
		return std::shared_ptr<_type>(new _type(lforward(args)...));
	}
}

#include <new>//::operator new
#include <cstdint>//std::uint8_t
#include <cstddef>//std::size_t
#include <array>
#include <cassert>

#include "BaseMacro.h"
//并不重载operator new
#include "DebugOutput.hpp"

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
	//对齐分配器
	class lalignas(AlignSize) aligned_alloc
	{
		static_assert(AlignSize > 1, "AlignSize must more than 1");
		static_assert(!(AlignSize &(AlignSize - 1)), "AlignSize must be power of 2");
	public:
		// The following will be the same for virtually all allocators
		//标准分配器所需求的类型定义和函数
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

		const_pointer address(const T& s) const{
			return &s;
		}

		std::size_t max_size() const{
			return (static_cast<std::size_t>(-1)) / sizeof(value_type);
		}

		template<typename U> struct rebind{
			typedef aligned_alloc<U, AlignSize> other;
		};

		bool operator!=(const aligned_alloc& other) const{
			return !(*this == other);
		}

		void construct(pointer const p, const_reference t) const{
			void * const pv = static_cast<void*>(p);
			new (pv)value_type(t);
		}

		template<typename... ARGS>
		void construct(pointer const p, ARGS&&... args) const {
			void * const pv = static_cast<void*>(p);
			new (pv)value_type(std::forward<ARGS>(args)...);
		}

		void destroy(pointer const p) const;

		//无状态分配器
		bool operator==(const aligned_alloc& other) const{
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

			auto size = n *sizeof(value_type) + AlignSize;
			std::uintptr_t rawAddress = reinterpret_cast<std::uintptr_t>(impl.allocate(size));
			DebugPrintf(L"align_alloc.allocate:Address: %p,Size: %u,Alignesize: %u\n", rawAddress, size, AlignSize);

			std::uint8_t missalign = AlignSize - (rawAddress&(AlignSize - 1));
			std::uintptr_t alignAddress = rawAddress + missalign;

			std::uint8_t* storemissalign = (std::uint8_t*)(alignAddress)-1;
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
	//模仿Orge

	//hack从堆上分配内存行为
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
	class CateAlloc{
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

			auto size = n *sizeof(value_type) + AlignSize;
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

	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_GENERAL,16>> GeneralAllocPolicy;
	typedef AllocPolicy<CateAlloc<MemoryCategory::MEMCATEGORY_GEOMETRY,16>> GeometryAllocPolicy;
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
	//内存块
	struct MemoryChunk{
		std::unique_ptr<stdex::byte[]> mData;
		std::size_t mSize;

		std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const{
			assert(u64Offset + uBytesToRead <= mSize);
			std::memcpy(pBuffer, &mData[u64Offset], uBytesToRead);
			return uBytesToRead;
		}
		void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
			assert(u64Offset + uBytesToWrite <= mSize);
			std::memcpy(&mData[u64Offset], pBuffer, uBytesToWrite);
		}
	};
#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif

	//接口类,从class_interface继承的
	class alloc :public std::allocator<std::uint8_t>//, leo::class_interface
	{
	private:
		using std::allocator<std::uint8_t>::address;
		using std::allocator<std::uint8_t>::allocate;
		using std::allocator<std::uint8_t>::construct;
		using std::allocator<std::uint8_t>::deallocate;
		using std::allocator<std::uint8_t>::destroy;
	public:
		virtual void* Alloc(std::size_t size) = 0;
		virtual void	Free(pointer p) = 0;
		void*	AllocWithAlign(std::size_t size, std::uint8_t alignsize)
		{
			//至少大于1
			assert(alignsize > 1);
			//为2^n
			assert(!(alignsize &(alignsize - 1)));

			size += alignsize;
			std::uintptr_t rawAddress = (std::uintptr_t)(this->Alloc(size));

			std::uint8_t missalign = alignsize - (rawAddress&(alignsize-1));
			std::uintptr_t alignAddress = rawAddress + missalign;

			std::uint8_t* storemissalign = (std::uint8_t*)(alignAddress)-1;
			*storemissalign = missalign;
			return (void*)alignAddress;
		}
		void	FreeWithAlign(pointer alignAddress)
		{
			std::uint8_t adjust = *reinterpret_cast<std::uint8_t*>((std::uintptr_t)alignAddress - 1);
			std::uintptr_t rawAddress = (std::uintptr_t)alignAddress - adjust;
			this->Free((pointer)rawAddress);
		}
	};

	//堆栈分配器,自身维持三个状态,总数量,已分配数量,raw pointer
	template<std::size_t SIZE>
	class StackAlloc : public alloc
	{
	private:
		static_assert(SIZE >= 1024,"SIZE < 1024");
		std::uint8_t buff[SIZE];
		std::size_t pos;
	public:
		//enum { _EEN_SIZE = SIZE };	// helper for expression evaluator
	private:
		using alloc::FreeWithAlign;
		void Free(pointer)
		{
			std::uintptr_t alignAddress{};
			std::uint8_t adjust = *((pointer)alignAddress - 1);
			std::uintptr_t rawAddress = alignAddress - adjust;
			Free((pointer)rawAddress);
		}
	public:
		explicit StackAlloc()
			:pos()
		{}
		~StackAlloc() = default;
		void*	Alloc(std::size_t size)
		{
			assert(pos + size < SIZE);
			void* p = buff + pos;
			pos += size;
			return	p;
		}
		void Free(std::size_t p)
		{
			this->pos = p;
		}
		void	Clear()
		{
			pos = 0;
		}
		//一字节请调用Alloc
		DefGetter(const lnothrow, decltype(pos), Pos, pos)
	public:
	};
}

namespace leo
{
#undef min
	template<typename T,typename U>
	void inline memcpy(T& dst, const U& src)
	{
		auto size = std::min(sizeof(T), sizeof(U));
		std::memcpy(&dst, &src, size);
	}

	template<typename T>
	void inline memset(T& dst,int val)
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