#pragma once
#include <memory>
#include <functional>
namespace leo
{
	template<typename _Ty>
	class unique_res;

	template<typename _Ty>
	inline bool default_check(_Ty *pointer)
	{
		return pointer != nullptr;
	}
	template<typename _Ty>
	inline void default_delete(_Ty *pointer)
	{
		delete pointer;
	}
	
	template<typename _Ty>
	class unique_res<_Ty*>
	{
	public:
		typedef _Ty value_type;
		typedef _Ty* pointer_type;
		typedef std::function<bool(pointer_type)> _CX;
		typedef std::function<void(pointer_type)> _DX;
	public:
		unique_res(pointer_type poi, _CX che = default_check<value_type>, _DX del = default_delete<value_type>)
			:_Pointer(poi), _Check(che(_Pointer)), _Del(del)
		{}
		unique_res(unique_res && rvalue)
		{
			_Pointer = std::move(rvalue._Pointer);
			_Check = std::move(rvalue._Check);
			_Del = std::move(rvalue._Del);
			rvalue._Pointer = nullptr;
			rvalue._Check = false;
			rvalue._Del = default_delete<value_type>;
		}
		~unique_res()
		{
			if (_Check) _Del(_Pointer);
			_Check = false;
		}
		bool is_good() const
		{
			return _Check;
		}
		const pointer_type get() const
		{
			return _Pointer;
		}
	public:
		unique_res() = delete;
		unique_res(const unique_res&) = delete;
		void operator = (const unique_res&) = delete;
	private:
		_Ty* _Pointer;
		bool _Check;
		_DX _Del;
	};

	static void * const invalid_void_pointer = reinterpret_cast<void*>(-1);

	struct HandleCloser
	{
		typedef void* value_type;
		void* operator()() const {
			return invalid_void_pointer;
		}
		void operator()(value_type hFile) const {
			::CloseHandle(hFile);
		}
	};

	template<class Closer_t>
	class UniqueHandle {
	public:
		typedef typename Closer_t::value_type Handle;

	private:
		Handle xm_hObj;

	public:
		 UniqueHandle() _NOEXCEPT : UniqueHandle(Closer_t()()) {
		}
		 explicit UniqueHandle(Handle hObj) _NOEXCEPT : xm_hObj(hObj) {
		}
		UniqueHandle(UniqueHandle &&rhs) _NOEXCEPT : UniqueHandle(rhs.Release()) {
		}
		UniqueHandle &operator=(Handle hObj) _NOEXCEPT{
			Reset(hObj);
			return *this;
		}
			UniqueHandle &operator=(UniqueHandle &&rhs) _NOEXCEPT{
			Reset(std::move(rhs));
			return *this;
		}
			~UniqueHandle(){
				Reset();
		}

		UniqueHandle(const UniqueHandle &) = delete;
		void operator=(const UniqueHandle &) = delete;

	public:
		bool IsGood() const _NOEXCEPT{
			return Get() != Closer_t()();
		}
			Handle Get() const _NOEXCEPT{
			return xm_hObj;
		}
			Handle Release() _NOEXCEPT{
			const Handle hOld = xm_hObj;
			xm_hObj = Closer_t()();
			return hOld;
		}

			void Reset(Handle hObj = Closer_t()()) _NOEXCEPT{
			const Handle hOld = xm_hObj;
			xm_hObj = hObj;
			if (hOld != Closer_t()()){
				Closer_t()(hOld);
			}
		}
			void Reset(UniqueHandle &&rhs) _NOEXCEPT{
			if (&rhs == this){
				return;
			}
			Reset(rhs.Release());
		}

			void Swap(UniqueHandle &rhs) _NOEXCEPT{
			if (&rhs == this){
				return;
			}
			std::swap(xm_hObj, rhs.xm_hObj);
		}

	public:
		explicit operator bool() const _NOEXCEPT{
			return IsGood();
		}
			explicit operator Handle() const _NOEXCEPT{
			return Get();
		}

			bool operator==(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() == rhs.Get();
		}
			bool operator!=(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() != rhs.Get();
		}
			bool operator<(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() < rhs.Get();
		}
			bool operator<=(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() <= rhs.Get();
		}
			bool operator>(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() > rhs.Get();
		}
			bool operator>=(const UniqueHandle &rhs) const _NOEXCEPT{
			return Get() >= rhs.Get();
		}
	};
}