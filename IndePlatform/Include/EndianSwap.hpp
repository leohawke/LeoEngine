////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/EndianSwap.hpp
//  Version:     v1.01
//  Created:     7/21/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-8-9 20:28: fix SwapEndian function bug
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_EndianSwap_hpp
#define IndePlatform_EndianSwap_hpp

//不同平台大小端不同,读取文件(文件按小编码保存)时需要交换编码
//大端平台,读取时间增加

#include "leoint.hpp"
#include "platform_macro.h"
#include <cstring> //std::memcpy
namespace leo
{
	enum class endian : bool
	{
		little = false,
		big = true
	};

	inline lconstexpr endian GetPlatformEndian()
	{
#ifdef Big_Endian
		return endian::big;
#else
		return endian::little;
#endif
	}


	void SwapEndian(const std::type_info& info, std::size_t check, void * data, std::size_t nCount = 1, bool bWriting = false);

	template<typename T>
	inline void SwapEndianBase(T* t, std::size_t nCount = 1, bool bWriting = false)
	{
		static_assert(is_pod<T>::value, "Onlu Support POD type");
		SwapEndian(typeid(T), sizeof(T), t, nCount, bWriting);
	}

	template<>
	//warning: sizeof(char)*8 == CHAR_BIT == 8
	inline void SwapEndianBase(char* p, std::size_t nCount, bool bWriting)
	{
		static_assert(CHAR_BIT == 8, "UnSupported Platform(sizeof(char) != 8)");
	}
	template<>
	inline void SwapEndianBase(uint8* p, std::size_t nCount, bool bWriting)
	{}
	template<>
	inline void SwapEndianBase(int8* p, std::size_t nCount, bool bWriting)
	{}

	template<>
	inline void SwapEndianBase(uint16* p, std::size_t nCount, bool bWriting)
	{

		for (; nCount-- > 0; p++)
			*p = (uint16)(((*p >> 8) + (*p << 8)) & 0xFFFF);

	}

	template<>
	inline void SwapEndianBase(int16* p, std::size_t nCount, bool bWriting)
	{
		SwapEndianBase((uint16*)p, nCount);
	}

	template<>
	inline void SwapEndianBase(uint32* p, std::size_t nCount, bool bWriting)
	{

		for (; nCount-- > 0; p++)
			*p = (*p >> 24) + ((*p >> 8) & 0xFF00) + ((*p & 0xFF00) << 8) + (*p << 24);

	}
	template<>
	inline void SwapEndianBase(int32* p, std::size_t nCount, bool bWriting)
	{
		SwapEndianBase((uint32*)p, nCount);
	}
	template<>
	//warning: sizeof(float) == sizeof(uint32) == 4
	inline void SwapEndianBase(float* p, std::size_t nCount, bool bWriting)
	{
		static_assert(sizeof(float) == 4, "UnSuppoted Platform(sizeof(float) != 4)");
		SwapEndianBase((uint32*)p, nCount);
	}

	template<>
	inline void SwapEndianBase(uint64* p, std::size_t nCount, bool bWriting)
	{

		for (; nCount-- > 0; p++)
			*p = (*p >> 56) + ((*p >> 40) & 0xFF00) + ((*p >> 24) & 0xFF0000) + ((*p >> 8) & 0xFF000000)
			+ ((*p & 0xFF000000) << 8) + ((*p & 0xFF0000) << 24) + ((*p & 0xFF00) << 40) + (*p << 56);
	}
	template<>
	inline void SwapEndianBase(int64* p, std::size_t nCount, bool bWriting)
	{
		SwapEndianBase((uint64*)p, nCount);
	}

#ifndef PLATFORM_64BIT

	template<>
	//warning: sizeof(double) == sizeof(uint64) == 8
	inline void SwapEndianBase(double* p, std::size_t nCount, bool bWriting)
	{
		static_assert(sizeof(double) == 8, "UnSuppoted Platform(sizeof(double) != 8)");
		SwapEndianBase((uint64*)p, nCount);
	}


	template<>
	//warning: sizeof(long double) == sizeof(uint64) == 8
	inline void SwapEndianBase(long double* p, std::size_t nCount, bool bWriting)
	{
		static_assert(sizeof(long double) == 8, "UnSuppoted Platform(sizeof(long double) != 8)");
		SwapEndianBase((uint64*)p, nCount);
	}
#endif

	template<class T>
	inline void SwapEndian(T* t, std::size_t nCount, endian bSwapEndian = GetPlatformEndian())
	{
		if (bSwapEndian)
			SwapEndianBase(t, nCount);
	}

	// 指定int和uint,reason: 重载歧义.
	template<class T>
	inline void SwapEndian(T* t, int nCount, endian bSwapEndian = GetPlatformEndian())
	{
		if (bSwapEndian)
			SwapEndianBase(t, nCount);
	}

#ifdef PLATFORM_64BIT
	template<class T>
	inline void SwapEndian(T* t, unsigned int nCount, endian bSwapEndian = GetPlatformEndian())
	{
		if (bSwapEndian)
			SwapEndianBase(t, nCount);
	}
#endif

	template<class T>
	inline void SwapEndian(T& t, endian bSwapEndian = GetPlatformEndian())
	{
		if (bSwapEndian)
			SwapEndianBase(&t, 1);
	}

	template<class T>
	inline T SwapEndianValue(T t, endian bSwapEndian = GetPlatformEndian())
	{
		if (bSwapEndian)
			SwapEndianBase(&t, 1);
		return t;
	}

	template<class A>
	inline void SwapEndianArray(A& array, endian bSwapEndian = GetPlatformEndian())
	{
		SwapEndian(array.begin(), array.size(), bSwapEndian);
	}

	template<class T, class D>
	inline T* StepData(D*& pData, size_t nCount, endian bSwapEndian = GetPlatformEndian())
	{
		T* Elems = (T*)pData;
		SwapEndian(Elems, nCount, bSwapEndian);
		pData = (D*)((T*)pData + nCount);
		return Elems;
	}

	template<class T, class D>
	inline T* StepData(D*& pData, endian bSwapEndian = GetPlatformEndian())
	{
		return StepData<T, D>(pData, 1, bSwapEndian);
	}

	template<class T, class D>
	inline T* StepData(T*& Result, D*& pData, size_t nCount = 1, endian bSwapEndian = GetPlatformEndian())
	{
		return Result = StepData<T>(pData, nCount, bSwapEndian);
	}

	template<class T, class D>
	inline void StepDataCopy(T* Dest, D*& pData, size_t nCount, endian bSwapEndian = GetPlatformEndian())
	{
		std::memcpy(Dest, pData, nCount*sizeof(T));
		SwapEndian(Dest, nCount, bSwapEndian);
		pData = (D*)((T*)pData + nCount);
	}

	template<class T, class D>
	inline void StepDataWrite(D*& pDest, const T* aSrc, size_t nCount, endian bSwapEndian)
	{
		std::memcpy(pDest, aSrc, nCount*sizeof(T));
		if (bSwapEndian)
			SwapEndianBase((T*)pDest, nCount, true);
		(T*&)pDest += nCount;
	}

	template<class T, class D>
	inline void StepDataWrite(D*& pDest, const T& Src, endian bSwapEndian)
	{
		StepDataWrite(pDest, &Src, 1, bSwapEndian);
	}
}
#endif