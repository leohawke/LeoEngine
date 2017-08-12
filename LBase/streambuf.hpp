/*!	\file streambuf.hpp
\ingroup LBase
\brief ISO C++ 标准库标准流缓冲扩展。
*/


#ifndef LB_stdex_streambuf_hpp_
#define LB_stdex_streambuf_hpp_ 1

#include "algorithm.hpp" // for leo::equal;
#include <streambuf> // for std::basic_streambuf;
#include <iterator> // for std::istreambuf_iterator;

namespace leo
{

	/*!
	\brief 比较指定流缓冲区的内容相等。
	\note 流读取失败视为截止。
	*/
	template<typename _tChar, class _tTraits>
	bool
		streambuf_equal(std::basic_streambuf<_tChar, _tTraits>& a,
			std::basic_streambuf<_tChar, _tTraits>& b)
	{
		using iter_type = std::istreambuf_iterator<_tChar, _tTraits>;

		return
			leo::equal(iter_type(&a), iter_type(), iter_type(&b), iter_type());
	}


	//@{
	/*!
	\brief 内存中储存不具有所有权的只读流缓冲。
	\warning 流操作不写缓冲区，否则行为未定义。
	*/
	template<typename _tChar, class _tTraits = std::char_traits<_tChar>>
	class basic_membuf : public std::basic_streambuf<_tChar, _tTraits>
	{
	public:
		using char_type = _tChar;
		using int_type = typename _tTraits::int_type;
		using pos_type = typename _tTraits::pos_type;
		using off_type = typename _tTraits::off_type;
		using traits_type = _tTraits;

		LB_NONNULL(2)
			basic_membuf(const char_type* buf, size_t size)
		{
			lconstraint(buf);
			// NOTE: This should be safe since get area is not used to be written.
			const auto p(const_cast<char_type*>(buf));

			this->setg(p, p, p + size);
		}
	};

	using membuf = basic_membuf<char>;
	using wmembuf = basic_membuf<wchar_t>;
	//@}

} // namespace leo;

#endif

