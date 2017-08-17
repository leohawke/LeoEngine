/*!	\file HostedGUI.h
\ingroup LCLib
\ingroup LCLibLimitedPlatforms
\brief 宿主 GUI 接口。
*/

#ifndef LFrameWork_LCLib_HostGUI_h
#define LFrameWork_LCLib_HostGUI_h 1

#include <LBase/operators.hpp>
#include <LBase/type_traits.hpp>
#include <LBase/string.hpp>
#include <LFramework/LCLib/FCommon.h>
#include <LFramework/Adaptor/LAdaptor.h>

namespace platform {
#if LFL_Win32
	using SPos = long;
	using SDst = unsigned long;
#else
	using SPos = leo::int16;
	using SDst = leo::uint16;
#endif
}

namespace leo {
	using platform::SPos;
	using platform::SDst;

	namespace Drawing {

		class Size;
		class Rect;

		/*!
		\brief 屏幕二元组。
		\warning 非虚析构。
		*/
		template<typename _type>
		class GBinaryGroup : private equality_comparable<GBinaryGroup<_type>>
		{
			static_assert(is_nothrow_copyable<_type>(),
				"Invalid type found.");

		public:
			static const GBinaryGroup Invalid; //!< 无效（不在屏幕坐标系中）对象。

			_type X = 0, Y = 0; //!< 分量。

								/*!
								\brief 无参数构造。
								\note 零初始化。
								*/
			lconstfn DefDeCtor(GBinaryGroup)
				/*!
				\brief 复制构造：默认实现。
				*/
				lconstfn DefDeCopyCtor(GBinaryGroup)
				/*!
				\brief 构造：使用 Size 对象。
				*/
				explicit lconstfn
				GBinaryGroup(const Size&) lnothrow;
			/*!
			\brief 构造：使用 Rect 对象。
			*/
			explicit lconstfn
				GBinaryGroup(const Rect&) lnothrow;
			/*!
			\brief 构造：使用两个纯量。
			\tparam _tScalar1 第一分量纯量类型。
			\tparam _tScalar2 第二分量纯量类型。
			\warning 模板参数和 _type 符号不同时隐式转换可能改变符号，不保证唯一结果。
			*/
			template<typename _tScalar1, typename _tScalar2>
			lconstfn
				GBinaryGroup(_tScalar1 x, _tScalar2 y) lnothrow
				: X(_type(x)), Y(_type(y))
			{}
			/*!
			\brief 构造：使用纯量对。
			\note 使用 std::get 取分量。仅取前两个分量。
			*/
			template<typename _tPair>
			lconstfn
				GBinaryGroup(const _tPair& pr) lnothrow
				: X(std::get<0>(pr)), Y(std::get<1>(pr))
			{}

			DefDeCopyAssignment(GBinaryGroup)

				/*!
				\brief 负运算：取加法逆元。
				*/
				lconstfn PDefHOp(GBinaryGroup, -, ) const lnothrow
				ImplRet(GBinaryGroup(-X, -Y))

				/*!
				\brief 加法赋值。
				*/
				PDefHOp(GBinaryGroup&, +=, const GBinaryGroup& val) lnothrow
				ImplRet(lunseq(X += val.X, Y += val.Y), *this)
				/*!
				\brief 减法赋值。
				*/
				PDefHOp(GBinaryGroup&, -=, const GBinaryGroup& val) lnothrow
				ImplRet(lunseq(X -= val.X, Y -= val.Y), *this)

				lconstfn DefGetter(const lnothrow, _type, X, X)
				lconstfn DefGetter(const lnothrow, _type, Y, Y)

				DefSetter(, _type, X, X)
				DefSetter(, _type, Y, Y)

				/*!
				\brief 判断是否是零元素。
				*/
				lconstfn DefPred(const lnothrow, Zero, X == 0 && Y == 0)

				/*!
				\brief 选择分量引用。
				\note 第二参数为 true 时选择第一分量，否则选择第二分量。
				*/
				//@{
				PDefH(_type&, GetRef, bool b = true) lnothrow
				ImplRet(b ? X : Y)
				PDefH(const _type&, GetRef, bool b = true) const lnothrow
				ImplRet(b ? X : Y)
				//@}
		};

		//! \relates GBinaryGroup
		//@{
		template<typename _type>
		const GBinaryGroup<_type> GBinaryGroup<_type>::Invalid{
			std::numeric_limits<_type>::lowest(), std::numeric_limits<_type>::lowest() };

		/*!
		\brief 比较：屏幕二元组相等关系。
		*/
		template<typename _type>
		lconstfn bool
			operator==(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return x.X == y.X && x.Y == y.Y;
		}

		/*!
		\brief 加法：屏幕二元组。
		*/
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator+(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X + y.X, x.Y + y.Y);
		}

		/*!
		\brief 减法：屏幕二元组。
		*/
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator-(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X - y.X, x.Y - y.Y);
		}

		/*!
		\brief 数乘：屏幕二元组。
		*/
		template<typename _type, typename _tScalar>
		lconstfn GBinaryGroup<_type>
			operator*(const GBinaryGroup<_type>& val, _tScalar l) lnothrow
		{
			return GBinaryGroup<_type>(val.X * l, val.Y * l);
		}

		/*!
		\brief 转置。
		*/
		template<class _tBinary>
		lconstfn _tBinary
			Transpose(const _tBinary& obj) lnothrow
		{
			return _tBinary(obj.Y, obj.X);
		}

		//@{
		//! \brief 转置变换：逆时针旋转直角。
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			TransposeCCW(const GBinaryGroup<_type>& val) lnothrow
		{
			return GBinaryGroup<_type>(val.Y, -val.X);
		}

		//! \brief 转置变换：顺时针旋转直角。
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			TransposeCW(const GBinaryGroup<_type>& val) lnothrow
		{
			return GBinaryGroup<_type>(-val.Y, val.X);
		}
		//@}

		/*!
		\brief 取分量。
		*/
		//@{
		template<size_t _vIdx, typename _type>
		lconstfn _type&
			get(GBinaryGroup<_type>& val)
		{
			static_assert(_vIdx < 2, "Invalid index found.");

			return _vIdx == 0 ? val.X : val.Y;
		}
		template<size_t _vIdx, typename _type>
		lconstfn const _type&
			get(const GBinaryGroup<_type>& val)
		{
			static_assert(_vIdx < 2, "Invalid index found.");

			return _vIdx == 0 ? val.X : val.Y;
		}
		//@}

		/*!
		\brief 转换为字符串。
		\note 使用 ADL 。
		*/
		template<typename _type>
		string
			to_string(const GBinaryGroup<_type>& val)
		{
			using leo::to_string;

			return quote(to_string(val.X) + ", " + to_string(val.Y), '(', ')');
		}
		//@}


		/*!
		\brief 屏幕二维点（直角坐标表示）。
		*/
		using Point = GBinaryGroup<SPos>;


		/*!
		\brief 屏幕二维向量（直角坐标表示）。
		*/
		using Vec = GBinaryGroup<SPos>;


		/*!
		\brief 屏幕区域大小。
		\warning 非虚析构。
		*/
		class LF_API Size : private equality_comparable<Size>
		{
		public:
			/*!
			\brief 无效对象。
			*/
			static const Size Invalid;

			SDst Width, Height; //!< 宽和高。

			/*!
			\brief 无参数构造。
			\note 零初始化。
			*/
			lconstfn
				Size() lnothrow
				: Width(0), Height(0)
			{}
			/*!
			\brief 复制构造。
			*/
			lconstfn
				Size(const Size& s) lnothrow
				: Width(s.Width), Height(s.Height)
			{}
			/*!
			\brief 构造：使用 Rect 对象。
			*/
			explicit lconstfn
				Size(const Rect&) lnothrow;
			/*!
			\brief 构造：使用屏幕二元组。
			*/
			template<typename _type>
			explicit lconstfn
				Size(const GBinaryGroup<_type>& val) lnothrow
				: Width(static_cast<SDst>(val.X)), Height(static_cast<SDst>(val.Y))
			{}
			/*!
			\brief 构造：使用两个纯量。
			*/
			template<typename _tScalar1, typename _tScalar2>
			lconstfn
				Size(_tScalar1 w, _tScalar2 h) lnothrow
				: Width(static_cast<SDst>(w)), Height(static_cast<SDst>(h))
			{}

			DefDeCopyAssignment(Size)

			/*!
			\brief 判断是否为空或非空。
			*/
			lconstfn DefBoolNeg(explicit lconstfn, Width != 0 || Height != 0)

			/*!
			\brief 求与另一个屏幕区域大小的交。
			\note 结果由分量最小值构造。
			*/
			PDefHOp(Size&, &=, const Size& s) lnothrow
			ImplRet(lunseq(Width = min(Width, s.Width),
				Height = min(Height, s.Height)), *this)

			/*!
			\brief 求与另一个屏幕标准矩形的并。
			\note 结果由分量最大值构造。
			*/
			PDefHOp(Size&, |=, const Size& s) lnothrow
			ImplRet(lunseq(Width = max(Width, s.Width),
				Height = max(Height, s.Height)), *this)

			/*!
			\brief 转换：屏幕二维向量。
			\note 以Width 和 Height 分量作为结果的 X 和 Y分量。
			*/
			lconstfn DefCvt(const lnothrow, Vec, Vec(Width, Height))

			/*!
			\brief 判断是否为线段：长或宽中有且一个数值等于 0 。
			*/
			lconstfn DefPred(const lnothrow, LineSegment,
				!((Width == 0) ^ (Height == 0)))
			/*!
			\brief 判断是否为不严格的空矩形区域：包括空矩形和线段。
			*/
			lconstfn DefPred(const lnothrow, UnstrictlyEmpty, Width == 0 || Height == 0)

			/*!
			\brief 选择分量引用。
			\note 第二参数为 true 时选择第一分量，否则选择第二分量。
			*/
			//@{
			PDefH(SDst&, GetRef, bool b = true) lnothrow
			ImplRet(b ? Width : Height)
			PDefH(const SDst&, GetRef, bool b = true) const lnothrow
			ImplRet(b ? Width : Height)
			//@}
		};
	}
}

#endif