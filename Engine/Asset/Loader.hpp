/*! \file Engine\Asset\Loader.h
\ingroup Engine
\brief Asset Base Interface ...
*/
#ifndef LE_ASSET_LOADER_HPP
#define LE_ASSET_LOADER_HPP 1

#include "../emacro.h"
#include <LBase/memory.hpp>

#include <experimental/resumable>
#include <experimental/generator>
#include <experimental/coroutine>
namespace asset {
	class IAssetLoading {
	public:
		virtual ~IAssetLoading();

		virtual std::size_t Type() const = 0;
	};


	template<typename T>
	class AssetLoading : IAssetLoading
	{
	public:
		using AssetType = T;

		virtual ~AssetLoading()
		{}

		//wait Coroutine()
		//返回值非空 已完成
		virtual std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() = 0;
	};

	template<typename Loading, typename... _tParams>
	std::shared_ptr<typename Loading::AssetType> SyncLoad(_tParams&&... args)
	{
		auto loading = std::make_unique<Loading>(lforward(args)...);

		auto coroutine = loading->Coroutine();

		auto iter = coroutine.begin();
		while (!*iter)
			++iter;

		return *iter;
	}
}

#endif