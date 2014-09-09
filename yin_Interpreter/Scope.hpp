#pragma once
#include <any.h>
#include <map>
#include <type_traits>
#include <set>

namespace std
{
	template<typename T>
	bool operator<(const std::reference_wrapper<T>& l, const std::reference_wrapper<T>& r)
	{
		return l.get() < r.get();
	}

	bool operator==(const leo::any& l, const leo::any& r)
	{
		return l.type() == r.type() && l.get() == r.get();
	}
}
namespace leo
{
	namespace yin
	{
		class Value;
		
		
		class Scope
		{
		public:
			typedef any value_type;
			typedef std::map<std::wstring, value_type> prop_type;
			typedef std::map < std::wstring, prop_type> map_type;
			map_type table;
			std::shared_ptr<Scope> parent = nullptr;

		public:
			Scope() = default;
			Scope(const std::shared_ptr<Scope>& parent)
				:parent(parent)
			{}
			Scope(const Scope&) = default;
			Scope(Scope&& rvalue)
				:table(std::move(rvalue.table)), parent(std::move(rvalue.parent))
			{}
			Scope(const nullptr_t&)
			{}

		public:
			Scope& operator=(const Scope& lvalue)
			{
				table = lvalue.table;
				parent = lvalue.parent;
				return *this;
			}
			Scope& operator=(Scope&& rvalue)
			{
				table = std::move(rvalue.table);
				parent = std::move(rvalue.parent);
				return *this;
			}
			virtual bool operator==(const Scope& obj)
			{
				return table == obj.table && parent == obj.parent;
			}
			virtual bool operator==(const nullptr_t&)
			{
				return parent == nullptr && table.empty();
			}
			virtual std::wstring toString() const
			{
				return std::wstring(L"leo::yin::Scope");
			}
			virtual std::size_t hashCode() const
			{
				return std::_Hash_seq(reinterpret_cast<const unsigned char*>(this), sizeof(Scope));
			}
		public:
			void putAll(const Scope& other)
			{
				for (auto map : other.table){
					table.insert(map);
				}
			}

			Value& lookup(const std::wstring& name);
			Value& lookupLocal(const std::wstring& name);
			Value& lookupType(const std::wstring& name);
			Value& lookupLocalType(const std::wstring& name);
			Value& lookupPropertyLocal(const std::wstring& name, const std::wstring& key);
			Value& lookupProperty(const std::wstring& name, const std::wstring& key);
			prop_type lookupAllProps(const std::wstring& name)
			{
				return table[name];
			}

			Scope findDefiningScope(const std::wstring& name)
			{
				auto v = table[name];
				if (!v.empty()){
					return *this;
				}
				else if (parent != nullptr){
					return parent->findDefiningScope(name);
				}
				else {
					return nullptr;
				}
			}

			template<typename ValueType>
			void put(const std::wstring& name, const std::wstring& key, ValueType& value)
			{
				if (table.find(name) == table.end())
					table.emplace(name, prop_type());

				auto & item = table[name];
				item.emplace(key,value);
			}
			void putProperties(const std::wstring& name, const prop_type& props)
			{
				if (table.find(name) == table.end())
					table.emplace(name, props);
				else
				{
					for (auto& value : props)
						table[name].emplace(value);
				}
			}
			void putValue(const std::wstring& name, const Value& value);
			void putType(const std::wstring& name, const Value& value);
			std::set<std::reference_wrapper<const std::wstring>> keySet()
			{
				std::set<std::reference_wrapper<const std::wstring>> result;
				for (auto & key : table)
					result.emplace(std::reference_wrapper<const std::wstring>(key.first));
				return result;
			}
		public:
			static std::shared_ptr<Scope> buildInitScope();
			static std::shared_ptr<Scope> buildInitTypeScope();
		};
	}
}