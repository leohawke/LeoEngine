#include <BaseMacro.h>
#include <limits>


namespace  leo
{

	namespace HUD
	{

		struct Size;
		struct Rect;


		/*!
		\brief 屏幕二元组。
		\warning 非虚析构。
		*/
		template<typename _type>
		class GBinaryGroup
		{
		public:
			static const GBinaryGroup Invalid; //!< 无效（不在屏幕坐标系中）对象。

			_type X = 0, Y = 0; //!< 分量。

								/*!
								\brief 无参数构造。
								\note 零初始化。
								\since build 319
								*/
			lconstfn DefDeCtor(GBinaryGroup)
				/*!
				\brief 复制构造：默认实现。
				*/
				lconstfn DefDeCopyCtor(GBinaryGroup)
				/*!
				\brief 构造：使用 Size 对象。
				\since build 319
				*/
				explicit lconstfn
				GBinaryGroup(const Size&) lnothrow;
			/*!
			\brief 构造：使用 Rect 对象。
			\since build 319
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

			//! \since build 554
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
				ImplRet(yunseq(X += val.X, Y += val.Y), *this)
				/*!
				\brief 减法赋值。
				*/
				PDefHOp(GBinaryGroup&, -=, const GBinaryGroup& val) lnothrow
				ImplRet(yunseq(X -= val.X, Y -= val.Y), *this)

				lconstfn DefGetter(const lnothrow, _type, X, X)
				lconstfn DefGetter(const lnothrow, _type, Y, Y)

				DefSetter(_type, X, X)
				DefSetter(_type, Y, Y)

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

		template<typename _type>
		lconstfn bool
			operator==(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return x.X == y.X && x.Y == y.Y;
		}

		template<typename _type>
		lconstfn bool
			operator!=(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return !(x == y);
		}

		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator+(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X + y.X, x.Y + y.Y);
		}

		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator-(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X - y.X, x.Y - y.Y);
		}

		template<typename _type, typename _tScalar>
		lconstfn GBinaryGroup<_type>
			operator*(const GBinaryGroup<_type>& val, _tScalar l) lnothrow
		{
			return GBinaryGroup<_type>(val.X * l, val.Y * l);
		}

		template<class _tBinary>
		lconstfn _tBinary
			Transpose(const _tBinary& obj) lnothrow
		{
			return _tBinary(obj.Y, obj.X);
		}

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
	}
}
