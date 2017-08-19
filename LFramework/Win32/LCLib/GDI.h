/*!	\file GDI.h
\ingroup Win32/LCLib
\brief Win32 GDI �ӿڡ�
*/

#ifndef LFrameWork_Win32_GDI_h
#define LFrameWork_Win32_GDI_h 1

#include <LFramework/LCLib/HostGUI.h>

namespace leo {
	namespace Drawing {
		using Pixel = uint32;
		using BitmapPtr = Pixel*;
		using ConstBitmapPtr = const Pixel*;

#define PixelGetA(pixel) static_cast<uint8>((pixel & 0xFF000000) >> 24)
#define PixelGetR(pixel) static_cast<uint8>((pixel & 0x00FF0000) >> 16)
#define PixelGetG(pixel) static_cast<uint8>((pixel & 0x0000FF00) >> 8)
#define PixelGetB(pixel) static_cast<uint8>((pixel & 0x000000FF) >> 0)
#define PixelCtor(A,R,G,B) static_cast<Pixel>(A << 24 | R << 16 | G << 8 | B)

		template<bool _bSwapLR, bool _bSwapUD, typename _tOut, typename _tIn,
			typename _fBlitScanner>
			inline void
			BlitLines(_fBlitScanner scanner, _tOut dst, _tIn src, const Size& ds,
				const Size& ss, const Point& dp, const Point& sp, const Size& sc)
		{
			throw leo::unimplemented();
		}


		/*!
		\ingroup BlitScanner
		\brief ɨ���ߣ���ָ��ɨ��˳����һ�����ء�
		\warning ������������Ч�ԡ�
		*/
		//@{
		template<bool _bDec>
		struct CopyLine
		{
			/*!
			\brief ���Ƶ�����ָ����һ�����ء�
			\tparam _tOut ������������ͣ���Ҫ֧�� + ������һ��Ӧ���������������
			\tparam _tIn ������������͡�
			\pre ���ԣ��Է��������ʼ�����������ж�Ϊ���ɽ����á�
			*/
			//@{
			template<typename _tOut, typename _tIn>
			void
				operator()(_tOut& dst_iter, _tIn& src_iter, SDst delta_x) const
			{
				using leo::is_undereferenceable;

				LAssert(delta_x == 0 || !is_undereferenceable(dst_iter),
					"Invalid output iterator found."),
					YAssert(delta_x == 0 || !is_undereferenceable(src_iter),
						"Invalid input iterator found.");
				std::copy_n(src_iter, delta_x, dst_iter);
				// NOTE: Possible undefined behavior. See $2015-02
				//	@ %Documentation::Workflow::Annual2015.
				lunseq(src_iter += delta_x, dst_iter += delta_x);
			}
			template<typename _tOut, typename _tPixel>
			void
				operator()(_tOut& dst_iter, leo::pseudo_iterator<_tPixel> src_iter,
					SDst delta_x) const
			{
				using leo::is_undereferenceable;

				LAssert(delta_x == 0 || !is_undereferenceable(dst_iter),
					"Invalid output iterator found."),
					std::fill_n(dst_iter, delta_x, Deref(src_iter));
				// NOTE: Possible undefined behavior. See $2015-02
				//	@ %Documentation::Workflow::Annual2015.
				dst_iter += delta_x;
			}
			//@}
		};
	}
}

#if LF_Hosted
namespace platform_ex
{
	namespace Windows {
#	if LFL_Win32
		//@{
		//! \brief ʹ�� ::DeleteObject ɾ�������ɾ������
		struct LF_API GDIObjectDelete
		{
			using pointer = void*;

			void
				operator()(pointer) const lnothrow;
		};
		//@}


		/*!
		\brief ����ָ����С�ļ���λͼ��
		\post �ڶ������ǿա�
		\exception �쳣�������� leo::CheckArithmetic �׳���
		\throw Win32Exception ::CreateDIBSection ����ʧ�ܡ�
		\throw std::runtime_error �����������ʧ�ܣ� ::CreateDIBSection ʵ�ִ��󣩡�
		\return �ǿվ����

		���� ::CreateDIBSection ���� 32 λλͼ��ʹ�ü��ݻ�����ָ�����ڶ�������
		*/
		LF_API::HBITMAP
			CreateCompatibleDIBSection(const leo::Drawing::Size&,
				leo::Drawing::BitmapPtr&);
#	endif


		/*!
		\note ���ظ�ʽ�� platform::Pixel ���ݡ�
		\warning ����������
		*/
		//@{
		/*!
		\brief ������Ļ���档
		\note Android ƽ̨�����ؿ�����ʵ�ֵĻ������Ŀ�
		*/
		class LF_API ScreenBuffer
		{
		private:
#	if LFL_HostedUI_XCB || LFL_Android
			/*!
			\invariant bool(p_impl) ��
			*/
			unique_ptr<ScreenBufferData> p_impl;
			/*!
			\brief ���������������Ļ�������ʵ�ʿ�ȡ�
			*/
			leo::SDst width;
#	elif LFL_Win32
			leo::Drawing::Size size;
			leo::Drawing::BitmapPtr p_buffer;
			/*!
			\invariant \c bool(p_bitmap) ��
			*/
			unique_ptr_from<GDIObjectDelete> p_bitmap;
#	endif

		public:
			//! \brief ���죺ʹ��ָ���Ļ�������С�͵��ڻ�����������ؿ�ࡣ
			ScreenBuffer(const leo::Drawing::Size&);
#	if LFL_HostedUI_XCB || LFL_Android
			/*!
			\brief ���죺ʹ��ָ���Ļ�������С�����ؿ�ࡣ
			\throw Exception ���ؿ��С�ڻ�������С��
			\since build 498
			*/
			ScreenBuffer(const leo::Drawing::Size&, leo::SDst);
#	endif
			ScreenBuffer(ScreenBuffer&&) lnothrow;
			~ScreenBuffer();

			/*!
			\brief ת�Ƹ�ֵ��ʹ�ý�����
			*/
			PDefHOp(ScreenBuffer&, =, ScreenBuffer&& sbuf) lnothrow
				ImplRet(swap(sbuf, *this), *this)

#	if LFL_HostedUI_XCB || LFL_Android
				//! \since build 492
				leo::Drawing::BitmapPtr
				GetBufferPtr() const lnothrow;
			//! \since build 566
			leo::Drawing::Graphics
				GetContext() const lnothrow;
			//! \since build 498
			leo::Drawing::Size
				GetSize() const lnothrow;
			//! \since build 498
			leo::SDst
				GetStride() const lnothrow;
#	elif LFL_Win32
				//@{
				DefGetter(const lnothrow, leo::Drawing::BitmapPtr, BufferPtr, p_buffer)
				DefGetter(const lnothrow, ::HBITMAP, NativeHandle,
					::HBITMAP(p_bitmap.get()))
				DefGetter(const lnothrow, const leo::Drawing::Size&, Size, size)

				/*!
				\brief �ӻ��������²��� Alpha Ԥ�ˡ�
				\pre ���ԣ������ǿա�
				\post ::HBITMAP �� rgbReserved Ϊ 0 ��
				\warning ֱ�Ӹ��ƣ�û�б߽�ʹ�С��顣ʵ�ʴ洢����� 32 λ ::HBITMAP ���ݡ�
				*/
				LB_NONNULL(1) void
				Premultiply(leo::Drawing::ConstBitmapPtr) lnothrow;
#	endif

			/*!
			\brief �������ô�С��
			\note ����Сһ��ʱ�޲������������·��䣬�ɵ��±�������ͻ�����ָ��ĸı䡣
			*/
			void
				Resize(const leo::Drawing::Size&);

			/*!
			\brief �ӻ��������¡�
			\pre ��Ӷ��ԣ������ǿա�
			\pre Android ƽ̨����������С�����ؿ����ȫһ�¡�
			\post Win32 ƽ̨�� \c ::HBITMAP �� \c rgbReserved Ϊ 0 ��
			\warning ֱ�Ӹ��ƣ�û�б߽�ʹ�С��顣
			\warning Win32 ƽ̨��ʵ�ʴ洢����� 32 λ ::HBITMAP ���ݡ�
			\warning Android ƽ̨��ʵ�ʴ洢����� 32 λ RGBA8888 ���ݡ�
			*/
			LB_NONNULL(1) void
				UpdateFrom(leo::Drawing::ConstBitmapPtr);
			//@}

#	if LFL_Win32
			/*!
			\brief �ӻ���������ָ���߽������
			\pre ��Ӷ��ԣ������ǿա�
			\post \c ::HBITMAP �� \c rgbReserved Ϊ 0 ��
			\warning ֱ�Ӹ��ƣ�û�б߽�ʹ�С��顣
			\warning ʵ�ʴ洢����� 32 λ ::HBITMAP ���ݡ�
			*/
			LB_NONNULL(1) void
				UpdateFromBounds(leo::Drawing::ConstBitmapPtr,
					const leo::Drawing::Rect&) lnothrow;

			/*!
			\pre ��Ӷ��ԣ���������ǿա�
			*/
			void
				UpdatePremultipliedTo(NativeWindowHandle, leo::Drawing::AlphaType = 0xFF,
					const leo::Drawing::Point& = {});
#	endif

			/*!
			\pre ��Ӷ��ԣ���������ǿա�
			\since build 589
			*/
			void
				UpdateTo(NativeWindowHandle, const leo::Drawing::Point& = {}) lnothrow;

#	if LFL_Win32
			/*!
			\pre ��Ӷ��ԣ���������ǿա�
			*/
			void
				UpdateToBounds(NativeWindowHandle, const leo::Drawing::Rect&,
					const leo::Drawing::Point& = {}) lnothrow;
#	endif

			/*!
			\brief ������
			*/
			LF_API friend void
				swap(ScreenBuffer&, ScreenBuffer&) lnothrow;
		};


		/*!
		\brief ������Ļ���򻺴档
		*/
		class LF_API ScreenRegionBuffer : private ScreenBuffer
		{
		private:
			leo::mutex mtx{};

		public:
			using ScreenBuffer::ScreenBuffer;

			DefGetter(lnothrow, ScreenBuffer&, ScreenBufferRef, *this)

				//! \since build 589
				PDefH(leo::locked_ptr<ScreenBuffer>, Lock, )
				ImplRet({ &GetScreenBufferRef(), mtx })
		};
		//@}

		//@{
		/*!
		\brief �����ڴ���棺���洰���ϵĶ�άͼ�λ���״̬��
		\note �����ڴ�������������Ȩ��
		*/
		class LF_API WindowMemorySurface
		{
		private:
			::HDC h_owner_dc, h_mem_dc;

		public:
			WindowMemorySurface(::HDC);
			~WindowMemorySurface();

			DefGetter(const lnothrow, ::HDC, OwnerHandle, h_owner_dc)
				DefGetter(const lnothrow, ::HDC, NativeHandle, h_mem_dc)

				void
				UpdateBounds(ScreenBuffer&, const leo::Drawing::Rect&,
					const leo::Drawing::Point& = {}) lnothrow;

			void
				UpdatePremultiplied(ScreenBuffer&, NativeWindowHandle,
					leo::Drawing::AlphaType = 0xFF, const leo::Drawing::Point& = {})
				lnothrow;
		};


		class LF_API WindowDeviceContextBase
		{
		protected:
			NativeWindowHandle hWindow;
			::HDC hDC;

			WindowDeviceContextBase(NativeWindowHandle h_wnd, ::HDC h_dc) lnothrow
				: hWindow(h_wnd), hDC(h_dc)
			{}
			DefDeDtor(WindowDeviceContextBase)

		public:
			DefGetter(const lnothrow, ::HDC, DeviceContextHandle, hDC)
				DefGetter(const lnothrow, NativeWindowHandle, WindowHandle, hWindow)
		};


		/*!
		\brief �����豸�����ġ�
		\note �����豸������������Ȩ��
		*/
		class LF_API WindowDeviceContext : public WindowDeviceContextBase
		{
		protected:
			/*!
			\pre ��Ӷ��ԣ������ǿա�
			\throw leo::LoggedEvent ��ʼ��ʧ�ܡ�
			*/
			WindowDeviceContext(NativeWindowHandle);
			~WindowDeviceContext();
		};


		/*!
		\brief ��ҳ��λͼ���ݡ�
		\note ��������ʵ�ֵķǹ����ӿڡ�
		*/
		class PaintStructData
		{
		public:
			struct Data;

		private:
			//! \note ���ֺ� \c ::PAINTSTRUCT �����Ƽ��ݡ�
#if LFL_Win64
			leo::aligned_storage_t<72, 8> ps;
#else
			leo::aligned_storage_t<64, 4> ps;
#endif
			//! \invariant <tt>&pun.get() == &ps</tt>
			leo::pun_ref<Data> pun;

		public:
			PaintStructData();
			~PaintStructData();

			DefGetter(const lnothrow, Data&, , pun.get())
		};


		/*!
		\brief ���������豸�����ġ�
		\note �����豸������������Ȩ��
		*/
		class LF_API WindowRegionDeviceContext : protected PaintStructData,
			public WindowDeviceContextBase, private leo::noncopyable
		{
		protected:
			/*!
			\pre ��Ӷ��ԣ������ǿա�
			\throw leo::LoggedEvent ��ʼ��ʧ�ܡ�
			*/
			WindowRegionDeviceContext(NativeWindowHandle);
			~WindowRegionDeviceContext();

		public:
			/*!
			\brief �жϱ����Ƿ���Ȼ��Ч�������������»��ơ�
			*/
			bool
				IsBackgroundValid() const lnothrow;

			/*!
			\exception �쳣�������� leo::CheckArithmetic �׳���
			*/
			leo::Drawing::Rect
				GetInvalidatedArea() const;
		};
		//@}


		/*!
		\brief ��ʾ������棺������ʾ�����ϵĶ�άͼ�λ���״̬��
		\warning ����������
		*/
		template<typename _type = WindowDeviceContext>
		class GSurface : public _type, public WindowMemorySurface
		{
		public:
			/*!
			\pre ��Ӷ��ԣ������ǿա�
			\exception leo::LoggedEvent �����Ӷ����ʼ��ʧ�ܡ�
			*/
			GSurface(NativeWindowHandle h_wnd)
				: _type(leo::Nonnull(h_wnd)),
				WindowMemorySurface(_type::GetDeviceContextHandle())
			{}
		};


		
	}
}
#endif


#endif