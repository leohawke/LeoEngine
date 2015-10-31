#include "LeoScheme.h"
#include <vector>
namespace leo {
	namespace script {
		extern std::shared_ptr<scheme_void_t> scheme_void = std::make_shared<scheme_void_t>();

		scheme_value scheme_void_t::Apply(const std::experimental::string_view& proc, scheme_value& input)
		{
			assert(!"scheme_void_t::Apply");
			return scheme_value(scheme_void);
		}

		scheme_value scheme_void_t::Apply(const std::string& proc, scheme_value& input)
		{
			assert(!"scheme_void_t::Apply");
			return scheme_value(scheme_void);
		}

		template<typename T>
		class scheme_vector :public scheme_obj, private std::vector<T>
		{
		public:
			handle GetHandle() const override{
				return reinterpret_cast<handle>(this);
			}

			scheme_value Apply(const std::experimental::string_view& proc, scheme_value& input) override {
				if (proc == "resize")
					resize(2);

				if (proc == "at")
					at(2);

				return scheme_value(scheme_void);
			}

			scheme_value Apply(const std::string& proc, scheme_value& input) override {
				if (proc == "resize")
					resize(2);

				return scheme_value(scheme_void);
			}
		protected:
			std::string ToString() {
				return "scheme_vector<T>";
			}
		};

		scheme_value test(std::make_shared <scheme_vector<int>>());
	}
}