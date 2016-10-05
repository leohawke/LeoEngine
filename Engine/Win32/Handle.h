/*! \file Engine\WIN32\Handle.h
\ingroup Engine
\brief Win32 HANDLE 接口。
*/
#ifndef LE_WIN32_HANDLE_H
#define LE_WIN32_HANDLE_H 1

#include <LBase/ldef.h>
#include <LBase/type_traits.hpp>
#include <LBase/memory.hpp>

namespace platform_ex {
	namespace Windows {

		// 这个代码部分是 MCF 的一部分。
		// 有关具体授权说明，请参阅 MCFLicense.txt。
		template<class CloserT>
		class UniqueHandle {
		public:
			using Handle = std::decay_t<decltype(CloserT()())>;
			using Closer = CloserT;

			static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
			lnoexcept_assert("Handle closer must not throw.",Closer()(std::declval<Handle>()));

		private:
			Handle x_hObject;

		private:
			void X_Dispose() lnoexcept {
				const auto hObject = x_hObject;
				if (hObject) {
					Closer()(hObject);
				}
#ifndef NDEBUG
				std::memset(&x_hObject, 0xEF, sizeof(x_hObject));
#endif
			}

		public:
			lconstexpr UniqueHandle() lnoexcept
				: x_hObject(Closer()())
			{
			}
			explicit lconstexpr UniqueHandle(Handle rhs) lnoexcept
				: x_hObject(rhs)
			{
			}
			UniqueHandle(UniqueHandle &&rhs) lnoexcept
				: x_hObject(rhs.Release())
			{
			}
			UniqueHandle &operator=(UniqueHandle &&rhs) lnoexcept {
				return Reset(std::move(rhs));
			}
			~UniqueHandle() {
				X_Dispose();
			}

			UniqueHandle(const UniqueHandle &) = delete;
			UniqueHandle &operator=(const UniqueHandle &) = delete;

		public:
			lconstexpr bool IsNull() const lnoexcept {
				return x_hObject == Closer()();
			}
			lconstexpr Handle Get() const lnoexcept {
				return x_hObject;
			}
			Handle Release() lnoexcept {
				return std::exchange(x_hObject, Closer()());
			}

			UniqueHandle &Reset(Handle rhs = Closer()()) lnoexcept {
				UniqueHandle(rhs).Swap(*this);
				return *this;
			}
			UniqueHandle &Reset(UniqueHandle &&rhs) lnoexcept {
				UniqueHandle(std::move(rhs)).Swap(*this);
				return *this;
			}

			void Swap(UniqueHandle &rhs) lnoexcept {
				using std::swap;
				swap(x_hObject, rhs.x_hObject);
			}

		public:
			explicit lconstexpr operator bool() const lnoexcept {
				return !IsNull();
			}
			explicit lconstexpr operator Handle() const lnoexcept {
				return Get();
			}

			template<class OtherCloserT>
			lconstexpr bool operator==(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject == rhs.x_hObject;
			}
			lconstexpr bool operator==(Handle rhs) const lnoexcept {
				return x_hObject == rhs;
			}
			friend lconstexpr bool operator==(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs == rhs.x_hObject;
			}

			template<class OtherCloserT>
			lconstexpr bool operator!=(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject != rhs.x_hObject;
			}
			lconstexpr bool operator!=(Handle rhs) const lnoexcept {
				return x_hObject != rhs;
			}
			friend lconstexpr bool operator!=(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs != rhs.x_hObject;
			}

			template<class OtherCloserT>
			lconstexpr bool operator<(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject < rhs.x_hObject;
			}
			lconstexpr bool operator<(Handle rhs) const lnoexcept {
				return x_hObject < rhs;
			}
			friend lconstexpr bool operator<(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs < rhs.x_hObject;
			}

			template<class OtherCloserT>
			lconstexpr bool operator>(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject > rhs.x_hObject;
			}
			lconstexpr bool operator>(Handle rhs) const lnoexcept {
				return x_hObject > rhs;
			}
			friend lconstexpr bool operator>(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs > rhs.x_hObject;
			}

			template<class OtherCloserT>
			lconstexpr bool operator<=(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject <= rhs.x_hObject;
			}
			lconstexpr bool operator<=(Handle rhs) const lnoexcept {
				return x_hObject <= rhs;
			}
			friend lconstexpr bool operator<=(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs <= rhs.x_hObject;
			}

			template<class OtherCloserT>
			lconstexpr bool operator>=(const UniqueHandle<OtherCloserT> &rhs) const lnoexcept {
				return x_hObject >= rhs.x_hObject;
			}
			lconstexpr bool operator>=(Handle rhs) const lnoexcept {
				return x_hObject >= rhs;
			}
			friend lconstexpr bool operator>=(Handle lhs, const UniqueHandle &rhs) lnoexcept {
				return lhs >= rhs.x_hObject;
			}

			friend void swap(UniqueHandle &lhs, UniqueHandle &rhs) lnoexcept {
				lhs.Swap(rhs);
			}
		};
	}
}

#endif
