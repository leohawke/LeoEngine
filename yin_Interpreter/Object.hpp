#pragma once
#include "..\All\\IndePlatform\memory.hpp"

namespace leo
{
	namespace yin
	{
		class Object : GeneralAllocatedObject
		{
		public:
			Object(const nullptr_t&)
			{}

			Object() = default;
			virtual ~Object()
			{}

			Object(const Object&) = default;
			Object(Object&&){}

			Object& operator=(const Object&)
			{}
			Object& operator=(Object&&)
			{}
		public:
			virtual bool operator==(const Object& obj)
			{
				return this == &obj;
			}
			virtual bool operator==(const nullptr_t&) 
			{
				return toString().empty();
			}
			virtual std::wstring toString() const = 0
			{
				return std::wstring();
			}
			virtual std::size_t hashCode() const = 0
			{
				return std::_Hash_seq(reinterpret_cast<const unsigned char*>(toString().c_str()), toString().length()*sizeof(wchar_t) / sizeof(char));
			}
		};		
	}
}

namespace std
{
	template<>
	class hash < leo::yin::Object >
	{
		size_t operator()(const leo::yin::Object& key) const
		{
			return key.hashCode();
		}
	};
}

