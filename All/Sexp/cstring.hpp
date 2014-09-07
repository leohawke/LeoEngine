#pragma once
#include <memory>
#include <cstddef>
namespace leo
{
	namespace sexp
	{
		struct cstring
		{
		private:
			cstring(const cstring&) = delete;
		public:
			static const std::size_t cstring_growsize = 8192;
		public:
			std::unique_ptr<char[]> base;
			std::size_t cap;
			std::size_t len;
		public:
			explicit cstring(std::size_t s);
			cstring& add(char * a);

			cstring& add(char a);

			cstring& trim();

			cstring& shrink_to_fit()
			{
				trim();
			}

			void clear()
			{
				len = 0;
			}

			const char* c_str() const
			{
				return base.get();
			}

			char* c_str()
			{
				return base.get();
			}

			cstring& operator+(char * a)
			{
				return add(a);
			}

			cstring& operator+(char a)
			{
				return add(a);
			}

			cstring& operator+=(char * a)
			{
				return add(a);
			}

			cstring& operator+=(char a)
			{
				return add(a);
			}
		};
	}
}