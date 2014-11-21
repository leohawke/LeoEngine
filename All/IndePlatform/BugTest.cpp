#define lalignas(_n) _declspec(align(_n))
#include <memory>
#include <stdexcept>
#include <cstdint>
#include <stdio.h>

namespace leo
{
	using uint8 = std::uint8_t;
	using std::make_unique;
}

namespace leo
{
	//  Description: Implements an aligned allocator for STL
	//               based on the Mallocator (http://blogs.msdn.com/b/vcblog/archive/2008/08/28/the-mallocator.aspx)
	// -------------------------------------------------------------------------
	template<typename T, int AlignSize>
	//对齐分配器
	class aligned_alloc
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
		void construct(pointer const p, ARGS... args) const {
			void * const pv = static_cast<void*>(p);
			new (pv)value_type(args...);
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
			printf("align_alloc.allocate:Address: %p,Size: %u,Alignesize: %u\n", rawAddress, size, AlignSize);

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
			printf("align_alloc.deallocate Adddress: %p,Alignesize: %u\n", rawAddress, AlignSize);
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


	template<MemoryCategory cate>
	class AllocatedObject
	{
	private:
	public:
		explicit AllocatedObject()
		{ }
#if 1
		virtual ~AllocatedObject()
		{ }
#endif
		void* operator new(size_t sz)
		{
			return aligned_alloc<uint8, 16>().allocate(sz);
		}
			void operator delete(void* ptr)
		{
			aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}
		/// placement operator new
		void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

			void operator delete(void* ptr, void*)
		{
			//aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}

		void* operator new[](size_t sz)
		{
			return aligned_alloc<uint8, 16>().allocate(sz);
		}
			void operator delete[](void* ptr)
		{
			aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}
	};


	template<MemoryCategory cate>
	class DataAllocatedObject
	{
	private:
	public:
		explicit DataAllocatedObject()
		{ }

#if 1
		virtual ~DataAllocatedObject()
		{ }
#endif

		void* operator new(size_t sz)
		{
			return aligned_alloc<uint8, 16>().allocate(sz);
		}
			void operator delete(void* ptr)
		{
			aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}
		/// placement operator new
		void* operator new(size_t sz, void* ptr)
		{
			(void)sz;
			return ptr;
		}

			void operator delete(void* ptr, void*)
		{
			//aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}

		void* operator new[](size_t sz)
		{
			return aligned_alloc<uint8, 16>().allocate(sz);
		}
			void operator delete[](void* ptr)
		{
			aligned_alloc<uint8, 16>().deallocate(typename aligned_alloc<uint8, 16>::pointer(ptr), 1);
		}
	};


	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_GENERAL> GeneralAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_GEOMETRY> GeometryAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_ANIMATION> AnimationAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_SCENE_CONTROL> SceneCtlAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_SCENE_OBJECTS> SceneObjAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_RESOURCE> ResourceAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_SCRIPTING> ScriptingAllocatedObject;
	typedef AllocatedObject<MemoryCategory::MEMCATEGORY_RENDERSYS> RenderSysAllocatedObject;

}


using namespace leo;



struct lalignas(16) float4
{
	union {
		struct {
			float x, y, z, w;
		};
		float data[4];
	};

	float4() = default;

	float4(float X, float Y, float Z, float W)
		:x(X), y(Y), z(Z), w(W)
	{}
};

typedef DataAllocatedObject<MemoryCategory::MEMCATEGORY_SCENE_CONTROL> float4x4Allocated;
struct lalignas(16) float4x4 : public float4x4Allocated{
	float4 r[4];


	float4x4() = default;
};

struct SQT {
	float4 y;
};

class SQTObject :public SQT, public GeneralAllocatedObject
{
};


class SkeletonInstance : public SQTObject {

	std::unique_ptr<float4x4[]> mSkinMatrixs;

public:
	SkeletonInstance() {
		mSkinMatrixs = make_unique<float4x4[]>(58);
	}
};

int main() {
	auto pSke = make_unique<SkeletonInstance[]>(3);
	pSke.reset(nullptr);
}