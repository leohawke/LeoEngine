/*! \file Core\Resources.h
\ingroup Engine
\brief 公共资源描述。
*/
#ifndef LE_Core_Resource_H
#define LE_Core_Resource_H 1

#include <string>
#include <LBase/sutility.h>
#include <LBase/lmacro.h>

namespace platform {

	class Resources:leo::noncopyable {
	public:
		Resources(const std::string& name) :
			name(name) {
		}

		DefGetter(const lnothrow,const std::string&, Name,name)
	private:
		std::string name;
	};
}

#endif