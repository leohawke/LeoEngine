/*! \file Core\ResourcesHolder.h
\ingroup Engine
\brief 可以关联资产实例化后的资源(Resource),也提供对资源状态的查询。
*/

#ifndef LE_Core_ResourcesHolder_h
#define LE_Core_ResourcesHolder_h 1

#include <LBase/sutility.h>
#include <LBase/any.h>
#include <LBase/tuple.hpp>
#include "Resource.h"

#include <memory_resource>

namespace platform {
	class IResourcesHolder :leo::nonmovable, leo::noncopyable {
	public:
		virtual std::shared_ptr<void> FindResource(const leo::any& key) = 0;

		template<typename _type,typename ... _tParams>
		std::shared_ptr<void> FindResource(const std::shared_ptr<_type> &asset, _tParams&&... args) {
			std::weak_ptr<void> base = asset;
			leo::any key = std::make_tuple(base, lforward(args)...);
			return FindResource(key);
		}

		template<typename _type, typename ... _tParams>
		static leo::any MakeKey(const std::shared_ptr<_type> &asset, _tParams&&... args) {
			std::weak_ptr<void> base = asset;
			return { std::make_tuple(base, lforward(args)...) };
		}
	protected:
		virtual ~IResourcesHolder();
		IResourcesHolder();

		static std::pmr::synchronized_pool_resource pool_resource;
	};

	template<typename _type>
	class  ResourcesHolder :public IResourcesHolder {
		static_assert(leo::is_convertible<_type&, Resource&>());

		template<typename _tAsset, typename ... _tParams>
		std::shared_ptr<_type> FindResource(const std::shared_ptr<_tAsset> &asset, _tParams&&... args) {
			return std::static_pointer_cast<_type>(IResourcesHolder::FindResource(asset, lforward(args)...));
		}
	};
}

#endif