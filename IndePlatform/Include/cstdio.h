#ifndef IndePlatform_cstdio_h
#define IndePlatform_cstdio_h

#include "Lassert.h" // for ../ldef.h, <cstdio> and lconstraint;
#include <cstdarg> // for std::va_list;
#include <memory> // for std::unique_ptr;
#include <ios> // for std::ios_base::openmode;
#include "iterator_op.hpp" // for iterator_operators_t, is_undereferenceable;
#include "base.h" // for noncopyable;

namespace stdex
{
	/*!
	\brief 基于 ISO C 标准库的流只读迭代器。
	*/
	class LB_API ifile_iterator : public std::iterator<std::input_iterator_tag,
		byte, ptrdiff_t, const byte*, const byte&>, public leo::iterator_operators_t<
		ifile_iterator, std::iterator_traits<limpl(std::iterator<
			std::input_iterator_tag, byte, ptrdiff_t, const byte*, const byte&>) >>
	{
	protected:
		using traits_type = std::iterator<std::input_iterator_tag, byte, ptrdiff_t,
			const byte*, const byte&>;

	public:
		using char_type = byte;
		using int_type = int;

	private:
		/*!
		*/
		std::FILE* stream{};
		char_type value;

	public:
		/*!
		\brief 无参数构造。
		\post <tt>!stream</tt> 。

		构造空流迭代器。
		*/
		lconstfn
			ifile_iterator()
			: value()
		{}
		/*!
		\brief 构造：使用流指针。
		\pre 断言： <tt>ptr</tt> 。
		\post <tt>stream == ptr</tt> 。
		*/
		explicit
			ifile_iterator(std::FILE* ptr)
			: stream(ptr)
		{
			lconstraint(ptr);
			++*this;
		}
		/*!
		\brief 复制构造：默认实现。
		*/
		lconstfn
			ifile_iterator(const ifile_iterator&) = default;
		~ifile_iterator() = default;

		ifile_iterator&
			operator=(const ifile_iterator&) = default;

		lconstfn reference
			operator*() const lnothrow
		{
			return value;
		}

		/*
		\brief 前置自增。
		\pre 断言：流指针非空。
		\return 自身引用。
		\note 当读到 EOF 时置流指针为空指针。

		使用 std::fgetc 读字符。
		*/
		ifile_iterator&
			operator++();

		friend bool
			operator==(const ifile_iterator& x, const ifile_iterator& y)
		{
			return x.stream == y.stream;
		}

		lconstfn std::FILE*
			get_stream() const
		{
			return stream;
		}

		/*!
		\brief 向流中写回字符。
		*/
		//@{
		//! \pre 断言： <tt>!stream</tt> 。
		int_type
			sputbackc(char_type c)
		{
			lconstraint(stream);
			return std::ungetc(c, stream);
		}
		//! \pre 断言： <tt>!stream || steram == s</tt> 。
		int_type
			sputbackc(char_type c, std::FILE* s)
		{
			lconstraint(!stream || stream == s);
			stream = s;
			return sputbackc(c);
		}

		//! \pre 间接断言： <tt>!stream</tt> 。
		int_type
			sungetc()
		{
			return sputbackc(value);
		}
		//! \pre 间接断言： <tt>!stream || steram == s</tt> 。
		int_type
			sungetc(std::FILE* s)
		{
			return sputbackc(value, s);
		}
		//@}
	};


	/*!
	\ingroup is_undereferenceable
	\brief 判断 ifile_iterator 实例是否确定为不可解引用。
	*/
	inline bool
		is_undereferenceable(const ifile_iterator& i) lnothrow
	{
		return !i.get_stream();
	}
}


#endif
