#ifndef HUD_Graphics_H
#define HUD_Graphics_H

#include "HUD.h"

LEO_BEGIN

/*!
\warning Ϊ����YSLIB����,����DXGI_FORMAT��ʽ
*/
enum PixelFormat {
	C48888,//4��ͨ����ÿͨ��8bit
};
template<PixelFormat = C48888>
struct Pixel;

template<>
struct Pixel<C48888> {
	stdex::byte r, g, b, a;
};

HUD_BEGIN

using BitmapPtr = Pixel<>*;
using ConstBitmapPtr = const Pixel<>*;

template<typename _tOut, typename _tIn>
void
CopyBitmapBuffer(_tOut p_dst, _tIn p_src, const Size& s)
{
	std::copy_n(Nonnull(p_src), GetAreaOf(s), Nonnull(p_dst));
}

/*!
\brief ��άͼ�νӿ�������ģ�塣
\warning Ϊ����YSLIB����
*/
template<typename _tPointer, class _tSize = Size>
class GGraphics
{
	static_assert(std::is_nothrow_move_constructible<_tPointer>::value,
		"Invalid pointer type found.");
	static_assert(std::is_nothrow_copy_constructible<_tSize>::value,
		"Invalid size type found.");

public:
	using PointerType = _tPointer;
	using SizeType = _tSize;
	using PixelType = std::decay_t<decltype(PointerType()[0])>;

protected:
	/*!
	\brief ��ʾ������ָ�롣
	\warning ���� PointerType ����Ϊ��������Ȩ������ָ�룬��Ӧ��Ϊ��������Ȩ��
	*/
	PointerType pBuffer{};
	//! \brief ͼ�������С��
	SizeType sGraphics{};

public:
	//! \brief Ĭ�Ϲ��죺ʹ�ÿ�ָ��ʹ�С��
	DefDeCtor(GGraphics)
		//! \brief ���죺ʹ��ָ��λͼָ��ʹ�С��
		explicit lconstfn
		GGraphics(PointerType p_buf, const SizeType& s = {}) lnothrow
		: pBuffer(std::move(p_buf)), sGraphics(s)
	{}
	//! \brief ���죺ʹ���������͵�ָ��λͼָ��ʹ�С��
	template<typename _tPointer2, class _tSize2>
	explicit lconstfn
		GGraphics(_tPointer2 p_buf, const _tSize2& s = {}) lnothrow
		: GGraphics(static_cast<PointerType>(std::move(p_buf)),
			static_cast<SizeType>(s))
	{}
	//! \brief ���죺ʹ���������͵�ָ��λͼָ��ʹ�С���͵Ķ�άͼ�νӿ������ġ�
	template<typename _tPointer2, class _tSize2>
	lconstfn
		GGraphics(const GGraphics<_tPointer2, _tSize2>& g) lnothrow
		: GGraphics(std::move(g.GetBufferPtr()), g.GetSize())
	{}
	//! \brief ���ƹ��죺ǳ���ơ�
	DefDeCopyCtor(GGraphics)
		DefDeMoveCtor(GGraphics)

		DefDeCopyAssignment(GGraphics)
		DefDeMoveAssignment(GGraphics)

		/*!
		\brief �ж���Ч����Ч�ԡ�
		\since build 319
		*/
		DefBoolNeg(explicit,
			bool(pBuffer) && sGraphics.Width != 0 && sGraphics.Height != 0)

		/*!
		\brief ȡָ������Ԫ��ָ�롣
		\pre ���ԣ�������Խ�硣
		\pre ��Ӷ��ԣ�������ָ��ǿա�
		*/
		PointerType
		operator[](size_t r) const lnothrow
	{
		LAssert(r < sGraphics.Height, "Access out of range.");
		return Nonnull(pBuffer) + r * sGraphics.Width;
	}

	//! \since build 566
	DefGetter(const lnothrow, const PointerType&, BufferPtr, pBuffer)
		DefGetter(const lnothrow, const SizeType&, Size, sGraphics)
		//! \since build 196
		DefGetter(const lnothrow, platform::unitlength_type, Width, sGraphics.Width)
		//! \since build 196
		DefGetter(const lnothrow, platform::unitlength_type, Height, sGraphics.Height)
		//! \since build 177
		DefGetter(const lnothrow, size_t, SizeOfBuffer,
			sizeof(PixelType) * size_t(GetAreaOf(sGraphics))) //!< ȡ������ռ�ÿռ䡣

															  /*!
															  \brief ȡָ������Ԫ��ָ�롣
															  \throw GeneralEvent ������ָ��Ϊ�ա�
															  \throw std::out_of_range ����Խ�硣
															  */
		PointerType
		at(size_t r) const
	{
		if (LB_UNLIKELY(!pBuffer))
			throw general_event("Null pointer found.");
		if (LB_UNLIKELY(r >= sGraphics.Height))
			throw std::out_of_range("Access out of range.");
		return pBuffer + r * sGraphics.Width;
	}
};

using ConstGraphics = GGraphics<ConstBitmapPtr>;
using Graphics = GGraphics<BitmapPtr>;

/*
\brief ���������ġ�
\warning ����������
\since build 255
*/
struct LB_API PaintContext
{
	Graphics Target; //!< ��ȾĿ�꣺ͼ�νӿ������ġ�
					 /*!
					 \brief �ο�λ�á�

					 ָ����ȾĿ������Ĳο����λ�õ�ƫ�����ꡣ
					 ��������Լ����ѡȡ��ȾĿ�����Ͻ�Ϊԭ�����Ļ����ϵ��
					 */
	Point Location;
	/*!
	\brief ��������

	���ͼ�νӿ������ĵı�׼���Σ�ָ����Ҫ��֤��ˢ�µı߽�����
	��������Լ�������������λ������ѡȡ��ȾĿ�����Ͻ�Ϊԭ�����Ļ����ϵ��
	*/
	Rect ClipArea;
};

HUD_END

LEO_END

#endif