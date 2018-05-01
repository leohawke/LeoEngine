/*! \file Engine\Asset\Loader.h
\ingroup Engine
\brief Asset Base Interface ...
*/
#ifndef LE_ASSET_LOADER_HPP
#define LE_ASSET_LOADER_HPP 1

#include "../emacro.h"
#include <LBase/memory.hpp>
#include <LBase/linttype.hpp>

#include <experimental/resumable>
#include <experimental/generator>
#include <experimental/coroutine>
namespace asset {
	template<unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3>
	struct four_cc {
		enum { value = (c0 << 0) + (c1 << 8) + (c2 << 16) + (c3 << 24) };
	};

	template<unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3>
	lconstexpr leo::uint32 four_cc_v = four_cc<c0, c1, c2, c3>::value;

	class IAssetLoading {
	public:
		virtual ~IAssetLoading();

		virtual std::size_t Type() const = 0;

		virtual std::size_t Hash() const = 0;
	};


	template<typename T>
	class AssetLoading : IAssetLoading
	{
	public:
		using AssetType = T;

		//wait Coroutine()
		//返回值非空 已完成
		virtual std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() = 0;
	};

}

#endif