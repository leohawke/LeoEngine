#ifndef IndePlatform_script_hpp
#define IndePlatform_script_hpp

#include "leomathtype.hpp"
#include <string>
#include <memory>
#include <experimental/string_view.hpp>
namespace leo {
	namespace script {
		struct scheme_int;
		struct scheme_real;

		struct scheme_value;

		using handle = std::intptr_t;

		/*!
		\def scheme_obj
		\since build 1.00
		*/
		class scheme_obj : std::enable_shared_from_this<scheme_obj>{
		public:
			virtual handle GetHandle() const = 0;

			virtual ~scheme_obj();

			virtual scheme_value Apply(const std::experimental::string_view& proc, scheme_value& input) = 0;
			virtual scheme_value Apply(const std::string& proc, scheme_value& input) = 0;
		protected:
			virtual std::string ToString() = 0;
		};

		class scheme_void_t :public scheme_obj {
		public:
			handle GetHandle() const override {
				return 0;
			}

			scheme_value Apply(const std::experimental::string_view& proc, scheme_value& input) override;
			scheme_value Apply(const std::string& proc, scheme_value& input) override;

			~scheme_void_t() = default;
			
		protected:
			std::string ToString() override {
				return "scheme_void";
			}
		};
	}
}


#endif
