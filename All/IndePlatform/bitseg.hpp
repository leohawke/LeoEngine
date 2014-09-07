#ifndef IndePlatform_bitseg_hpp
#define IndePlatform_bitseg_hpp

#include "type_op.hpp"
#include "utility.hpp"
#include <iterator>

//位段数据结构和访问
namespace leo
{
#if LB_IMPL_MSCPP || LB_IMPL_CLANGPP
	template<unsigned char _vN,bool _bEndian = false>
#else
	template<unsigned char _vN,bool _bEndian = {}>
#endif
	class bitseg_iterator : public std::iterator<std::random_access_iterator_tag,
		stdex::byte, ptrdiff_t, stdex::byte*,stdex::byte&>
	{
		static_assert(_vN != 0, "A bit segment should contain at least one bit.");
		static_assert(_vN != CHAR_BIT, "A bit segment should not be a byte.");
		static_assert(CHAR_BIT % _vN == 0,
			"A byte should be divided by number of segments without padding.");

	public:
		using byte = stdex::byte;
		using difference_type = ptrdiff_t;
		using pointer = byte*;
		using reference = byte&;

		static lconstexpr unsigned char seg_n = CHAR_BIT / _vN;
		static lconstexpr unsigned char seg_size = 1 << _vN;
		static lconstexpr unsigned char seg_width = _vN;

	protected:
		byte* base;
		unsigned char shift;
		mutable byte value;

	public:
		bitseg_iterator(byte* p = {}, unsigned char n = 0) lnothrow
			: base(p), shift(n)
		{
			lassume(shift < seg_n);
		}

		bitseg_iterator&
			operator+=(difference_type n) lnothrowv
		{
			lconstraint(base);
			lassume(shift < seg_n);

			const size_t new_shift(shift + n);

			lunseq(base += new_shift / seg_n, shift = new_shift % seg_n);
			return *this;
		}

			bitseg_iterator&
			operator-=(difference_type n) lnothrowv
		{
			base += -n;
			return *this;
		}

			reference
			operator*() const lnothrowv
		{
			lconstraint(base);
			return value = *base >> seg_width * (_bEndian ? seg_n - 1 - shift
				: seg_width) & seg_width;
		}

			lconstfn pointer
			operator->() const lnothrowv
		{
			return &**this;
		}

			inline bitseg_iterator&
			operator++() lnothrowv
		{
			lconstraint(base);
			lassume(shift < seg_n);
			if (++shift == seg_n)
				lunseq(shift = 0, ++base);
			return *this;
		}
			bitseg_iterator
			operator++(int)lnothrowv
		{
			auto i(*this);

			++*this;
			return i;
		}

			inline bitseg_iterator&
			operator--() lnothrowv
		{
			lconstraint(base);
			lassume(shift < seg_n);
			if (shift == 0)
				lunseq(--base, shift = seg_n - 1);
			else
				--shift;
			return *this;
		}
			bitseg_iterator
			operator--(int)lnothrowv
		{
			auto i(*this);

			--*this;
			return i;
		}

			reference
			operator[](difference_type n) const lnothrowv
		{
			const auto i(*this);

			i += n;
			return *i.operator->();
		}

			lconstfn bitseg_iterator
			operator+(difference_type n) const lnothrow
		{
			return bitseg_iterator(base + n);
		}

			lconstfn bitseg_iterator
			operator-(difference_type n) const lnothrow
		{
			return bitseg_iterator(base - n);
		}

			lconstfn explicit
			operator pointer() const lnothrow
		{
			return base;
		}

			lconstfn size_t
			get_shift() const lnothrow
		{
			return shift;
		}
	};

	template<size_t _vN, bool _bEndian>
	inline bool
		operator==(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		using pointer = typename bitseg_iterator<_vN, _bEndian>::pointer;

		return pointer(x) == pointer(y) && x.get_shift() == y.get_shift();
	}

	template<size_t _vN, bool _bEndian>
	inline bool
		operator!=(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		return !(x == y);
	}

	template<size_t _vN, bool _bEndian>
	inline bool
		operator<(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		using pointer = typename bitseg_iterator<_vN, _bEndian>::pointer;

		return pointer(x) < pointer(y)
			|| (pointer(x) == pointer(y) && x.get_shift() < y.get_shift());
	}

	template<size_t _vN, bool _bEndian>
	bool
		operator<=(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		return !(y < x);
	}

	template<size_t _vN, bool _bEndian>
	bool
		operator>(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		return y < x;
	}

	template<size_t _vN, bool _bEndian>
	bool
		operator>=(const bitseg_iterator<_vN, _bEndian>& x,
		const bitseg_iterator<_vN, _bEndian>& y)
	{
		return !(x < y);
	}
}

#endif