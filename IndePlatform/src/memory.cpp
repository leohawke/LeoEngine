#include "platform.h"

#include "memory.hpp"
#include <set>

namespace leo
{

	GeneralAllocPolicy GeneralAllocatedObject::impl;
	GeometryAllocPolicy GeometryAllocatedObject::impl;
	AnimationAllocPolicy AnimationAllocatedObject::impl;
	SceneCtlAllocPolicy SceneCtlAllocatedObject::impl;
	SceneObjAllocPolicy SceneObjAllocatedObject::impl;
	ResourceAllocPolicy ResourceAllocatedObject::impl;
	ScriptingAllocPolicy ScriptingAllocatedObject::impl;
	RenderSysAllocPolicy RenderSysAllocatedObject::impl;

	GeneralAllocPolicy DataAllocatedObject<GeneralAllocPolicy>::impl;


	struct __memroy_track_struct
	{
		void *p;
		std::uint8_t alignsize;
		bool placement;
	};

	bool operator<(const __memroy_track_struct& lhs, const __memroy_track_struct & rhs)
	{
		return lhs.p < rhs.p;
	}

	class ABCDEFGHIGK
	{
	public:
		~ABCDEFGHIGK()
		{
			if (!sets.empty())
			{
				MessageBoxW(nullptr, L"内存泄露!", L"运行错误:", MB_OK);
				std::abort();
			}
		}

		void record_alloc(void * p, std::uint8_t alignsize, bool placement)
		{
			sets.insert({ p, alignsize, placement });
		}
		void dealloc_record(void * p, std::uint8_t alignsize, bool placement)
		{
			__memroy_track_struct s{ p, alignsize, placement };
			auto pos = sets.find(s);
			if (pos == sets.cend())
			{
				MessageBoxW(nullptr, L"释放同一块内存多次!", L"运行错误:", MB_ERR_INVALID_CHARS);
				std::abort();
			}
			sets.erase(pos);
		}
		std::multiset<__memroy_track_struct> sets;
	};

	ABCDEFGHIGK* __memory_track_sets = nullptr;
	UINT32 ref_count = 0;

	void __memory_track_record_alloc(void * p, std::size_t count, std::uint8_t alignsize, const char* file, int line, const char* funcname, bool placement)
	{
		/*
		if (ref_count == 0)
			__memory_track_sets = new ABCDEFGHIGK();
		++ref_count;
		__memory_track_sets->record_alloc(p, alignsize, placement);
		*/
			
	}
	void __memory_track_dealloc_record(void * p, std::uint8_t alignsize, bool placement)
	{
		/*
		__memory_track_sets->dealloc_record(p, alignsize, placement);
		--ref_count;
		if (ref_count == 0)
			delete __memory_track_sets;
		*/
	}
}