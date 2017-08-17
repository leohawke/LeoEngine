/*!	\file HostedGUI.h
\ingroup LCLib
\ingroup LCLibLimitedPlatforms
\brief ���� GUI �ӿڡ�
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
		\brief ��Ļ��Ԫ�顣
		\warning ����������
		*/
		template<typename _type>
		class GBinaryGroup : private equality_comparable<GBinaryGroup<_type>>
		{
			static_assert(is_nothrow_copyable<_type>(),
				"Invalid type found.");

		public:
			static const GBinaryGroup Invalid; //!< ��Ч��������Ļ����ϵ�У�����

			_type X = 0, Y = 0; //!< ������

								/*!
								\brief �޲������졣
								\note ���ʼ����
								*/
			lconstfn DefDeCtor(GBinaryGroup)
				/*!
				\brief ���ƹ��죺Ĭ��ʵ�֡�
				*/
				lconstfn DefDeCopyCtor(GBinaryGroup)
				/*!
				\brief ���죺ʹ�� Size ����
				*/
				explicit lconstfn
				GBinaryGroup(const Size&) lnothrow;
			/*!
			\brief ���죺ʹ�� Rect ����
			*/
			explicit lconstfn
				GBinaryGroup(const Rect&) lnothrow;
			/*!
			\brief ���죺ʹ������������
			\tparam _tScalar1 ��һ�����������͡�
			\tparam _tScalar2 �ڶ������������͡�
			\warning ģ������� _type ���Ų�ͬʱ��ʽת�����ܸı���ţ�����֤Ψһ�����
			*/
			template<typename _tScalar1, typename _tScalar2>
			lconstfn
				GBinaryGroup(_tScalar1 x, _tScalar2 y) lnothrow
				: X(_type(x)), Y(_type(y))
			{}
			/*!
			\brief ���죺ʹ�ô����ԡ�
			\note ʹ�� std::get ȡ��������ȡǰ����������
			*/
			template<typename _tPair>
			lconstfn
				GBinaryGroup(const _tPair& pr) lnothrow
				: X(std::get<0>(pr)), Y(std::get<1>(pr))
			{}

			DefDeCopyAssignment(GBinaryGroup)

				/*!
				\brief �����㣺ȡ�ӷ���Ԫ��
				*/
				lconstfn PDefHOp(GBinaryGroup, -, ) const lnothrow
				ImplRet(GBinaryGroup(-X, -Y))

				/*!
				\brief �ӷ���ֵ��
				*/
				PDefHOp(GBinaryGroup&, +=, const GBinaryGroup& val) lnothrow
				ImplRet(lunseq(X += val.X, Y += val.Y), *this)
				/*!
				\brief ������ֵ��
				*/
				PDefHOp(GBinaryGroup&, -=, const GBinaryGroup& val) lnothrow
				ImplRet(lunseq(X -= val.X, Y -= val.Y), *this)

				lconstfn DefGetter(const lnothrow, _type, X, X)
				lconstfn DefGetter(const lnothrow, _type, Y, Y)

				DefSetter(, _type, X, X)
				DefSetter(, _type, Y, Y)

				/*!
				\brief �ж��Ƿ�����Ԫ�ء�
				*/
				lconstfn DefPred(const lnothrow, Zero, X == 0 && Y == 0)

				/*!
				\brief ѡ��������á�
				\note �ڶ�����Ϊ true ʱѡ���һ����������ѡ��ڶ�������
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
		\brief �Ƚϣ���Ļ��Ԫ����ȹ�ϵ��
		*/
		template<typename _type>
		lconstfn bool
			operator==(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return x.X == y.X && x.Y == y.Y;
		}

		/*!
		\brief �ӷ�����Ļ��Ԫ�顣
		*/
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator+(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X + y.X, x.Y + y.Y);
		}

		/*!
		\brief ��������Ļ��Ԫ�顣
		*/
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			operator-(const GBinaryGroup<_type>& x, const GBinaryGroup<_type>& y) lnothrow
		{
			return GBinaryGroup<_type>(x.X - y.X, x.Y - y.Y);
		}

		/*!
		\brief ���ˣ���Ļ��Ԫ�顣
		*/
		template<typename _type, typename _tScalar>
		lconstfn GBinaryGroup<_type>
			operator*(const GBinaryGroup<_type>& val, _tScalar l) lnothrow
		{
			return GBinaryGroup<_type>(val.X * l, val.Y * l);
		}

		/*!
		\brief ת�á�
		*/
		template<class _tBinary>
		lconstfn _tBinary
			Transpose(const _tBinary& obj) lnothrow
		{
			return _tBinary(obj.Y, obj.X);
		}

		//@{
		//! \brief ת�ñ任����ʱ����תֱ�ǡ�
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			TransposeCCW(const GBinaryGroup<_type>& val) lnothrow
		{
			return GBinaryGroup<_type>(val.Y, -val.X);
		}

		//! \brief ת�ñ任��˳ʱ����תֱ�ǡ�
		template<typename _type>
		lconstfn GBinaryGroup<_type>
			TransposeCW(const GBinaryGroup<_type>& val) lnothrow
		{
			return GBinaryGroup<_type>(-val.Y, val.X);
		}
		//@}

		/*!
		\brief ȡ������
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
		\brief ת��Ϊ�ַ�����
		\note ʹ�� ADL ��
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
		\brief ��Ļ��ά�㣨ֱ�������ʾ����
		*/
		using Point = GBinaryGroup<SPos>;


		/*!
		\brief ��Ļ��ά������ֱ�������ʾ����
		*/
		using Vec = GBinaryGroup<SPos>;


		/*!
		\brief ��Ļ�����С��
		\warning ����������
		*/
		class LF_API Size : private equality_comparable<Size>
		{
		public:
			/*!
			\brief ��Ч����
			*/
			static const Size Invalid;

			SDst Width, Height; //!< ��͸ߡ�

			/*!
			\brief �޲������졣
			\note ���ʼ����
			*/
			lconstfn
				Size() lnothrow
				: Width(0), Height(0)
			{}
			/*!
			\brief ���ƹ��졣
			*/
			lconstfn
				Size(const Size& s) lnothrow
				: Width(s.Width), Height(s.Height)
			{}
			/*!
			\brief ���죺ʹ�� Rect ����
			*/
			explicit lconstfn
				Size(const Rect&) lnothrow;
			/*!
			\brief ���죺ʹ����Ļ��Ԫ�顣
			*/
			template<typename _type>
			explicit lconstfn
				Size(const GBinaryGroup<_type>& val) lnothrow
				: Width(static_cast<SDst>(val.X)), Height(static_cast<SDst>(val.Y))
			{}
			/*!
			\brief ���죺ʹ������������
			*/
			template<typename _tScalar1, typename _tScalar2>
			lconstfn
				Size(_tScalar1 w, _tScalar2 h) lnothrow
				: Width(static_cast<SDst>(w)), Height(static_cast<SDst>(h))
			{}

			DefDeCopyAssignment(Size)

			/*!
			\brief �ж��Ƿ�Ϊ�ջ�ǿա�
			*/
			lconstfn DefBoolNeg(explicit lconstfn, Width != 0 || Height != 0)

			/*!
			\brief ������һ����Ļ�����С�Ľ���
			\note ����ɷ�����Сֵ���졣
			*/
			PDefHOp(Size&, &=, const Size& s) lnothrow
			ImplRet(lunseq(Width = min(Width, s.Width),
				Height = min(Height, s.Height)), *this)

			/*!
			\brief ������һ����Ļ��׼���εĲ���
			\note ����ɷ������ֵ���졣
			*/
			PDefHOp(Size&, |=, const Size& s) lnothrow
			ImplRet(lunseq(Width = max(Width, s.Width),
				Height = max(Height, s.Height)), *this)

			/*!
			\brief ת������Ļ��ά������
			\note ��Width �� Height ������Ϊ����� X �� Y������
			*/
			lconstfn DefCvt(const lnothrow, Vec, Vec(Width, Height))

			/*!
			\brief �ж��Ƿ�Ϊ�߶Σ������������һ����ֵ���� 0 ��
			*/
			lconstfn DefPred(const lnothrow, LineSegment,
				!((Width == 0) ^ (Height == 0)))
			/*!
			\brief �ж��Ƿ�Ϊ���ϸ�Ŀվ������򣺰����վ��κ��߶Ρ�
			*/
			lconstfn DefPred(const lnothrow, UnstrictlyEmpty, Width == 0 || Height == 0)

			/*!
			\brief ѡ��������á�
			\note �ڶ�����Ϊ true ʱѡ���һ����������ѡ��ڶ�������
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