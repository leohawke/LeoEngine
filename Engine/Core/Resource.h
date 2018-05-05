/*! \file Core\Resource.h
\ingroup Engine
\brief 公共资源描述。
*/
#ifndef LE_Core_Resource_H
#define LE_Core_Resource_H 1

#include <string>
#include <LBase/sutility.h>
#include <LBase/lmacro.h>

namespace platform {

	class Resource:leo::noncopyable {
	public:
		virtual ~Resource();

		virtual  const std::string& GetName() const lnothrow = 0;
	};
}

#endif