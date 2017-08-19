/*!	\file GDI.h
\ingroup Win32/LCLib
\brief Win32 GDI 接口。
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
		\brief 扫描线：按指定扫描顺序复制一行像素。
		\warning 不检查迭代器有效性。
		*/
		//@{
		template<bool _bDec>
		struct CopyLine
		{
			/*!
			\brief 复制迭代器指定的一行像素。
			\tparam _tOut 输出迭代器类型（需要支持 + 操作，一般应是随机迭代器）。
			\tparam _tIn 输入迭代器类型。
			\pre 断言：对非零参数起始迭代器不能判定为不可解引用。
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
		//! \brief 使用 ::DeleteObject 删除句柄的删除器。
		struct LF_API GDIObjectDelete
		{
			using pointer = void*;

			void
				operator()(pointer) const lnothrow;
		};
		//@}


		/*!
		\brief 创建指定大小的兼容位图。
		\post 第二参数非空。
		\exception 异常中立：由 leo::CheckArithmetic 抛出。
		\throw Win32Exception ::CreateDIBSection 调用失败。
		\throw std::runtime_error 后置条件检查失败（ ::CreateDIBSection 实现错误）。
		\return 非空句柄。

		调用 ::CreateDIBSection 创建 32 位位图，使用兼容缓冲区指针填充第二参数。
		*/
		LF_API::HBITMAP
			CreateCompatibleDIBSection(const leo::Drawing::Size&,
				leo::Drawing::BitmapPtr&);
#	endif


		/*!
		\note 像素格式和 platform::Pixel 兼容。
		\warning 非虚析构。
		*/
		//@{
		/*!
		\brief 虚拟屏幕缓存。
		\note Android 平台：像素跨距等于实现的缓冲区的宽。
		*/
		class LF_API ScreenBuffer
		{
		private:
#	if LFL_HostedUI_XCB || LFL_Android
			/*!
			\invariant bool(p_impl) 。
			*/
			unique_ptr<ScreenBufferData> p_impl;
			/*!
			\brief 宽：以像素数计量的缓冲区的实际宽度。
			*/
			leo::SDst width;
#	elif LFL_Win32
			leo::Drawing::Size size;
			leo::Drawing::BitmapPtr p_buffer;
			/*!
			\invariant \c bool(p_bitmap) 。
			*/
			unique_ptr_from<GDIObjectDelete> p_bitmap;
#	endif

		public:
			//! \brief 构造：使用指定的缓冲区大小和等于缓冲区宽的像素跨距。
			ScreenBuffer(const leo::Drawing::Size&);
#	if LFL_HostedUI_XCB || LFL_Android
			/*!
			\brief 构造：使用指定的缓冲区大小和像素跨距。
			\throw Exception 像素跨距小于缓冲区大小。
			\since build 498
			*/
			ScreenBuffer(const leo::Drawing::Size&, leo::SDst);
#	endif
			ScreenBuffer(ScreenBuffer&&) lnothrow;
			~ScreenBuffer();

			/*!
			\brief 转移赋值：使用交换。
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
				\brief 从缓冲区更新并按 Alpha 预乘。
				\pre 断言：参数非空。
				\post ::HBITMAP 的 rgbReserved 为 0 。
				\warning 直接复制，没有边界和大小检查。实际存储必须和 32 位 ::HBITMAP 兼容。
				*/
				LB_NONNULL(1) void
				Premultiply(leo::Drawing::ConstBitmapPtr) lnothrow;
#	endif

			/*!
			\brief 重新设置大小。
			\note 当大小一致时无操作，否则重新分配，可导致本机句柄和缓冲区指针的改变。
			*/
			void
				Resize(const leo::Drawing::Size&);

			/*!
			\brief 从缓冲区更新。
			\pre 间接断言：参数非空。
			\pre Android 平台：缓冲区大小和像素跨距完全一致。
			\post Win32 平台： \c ::HBITMAP 的 \c rgbReserved 为 0 。
			\warning 直接复制，没有边界和大小检查。
			\warning Win32 平台：实际存储必须和 32 位 ::HBITMAP 兼容。
			\warning Android 平台：实际存储必须和 32 位 RGBA8888 兼容。
			*/
			LB_NONNULL(1) void
				UpdateFrom(leo::Drawing::ConstBitmapPtr);
			//@}

#	if LFL_Win32
			/*!
			\brief 从缓冲区更新指定边界的区域。
			\pre 间接断言：参数非空。
			\post \c ::HBITMAP 的 \c rgbReserved 为 0 。
			\warning 直接复制，没有边界和大小检查。
			\warning 实际存储必须和 32 位 ::HBITMAP 兼容。
			*/
			LB_NONNULL(1) void
				UpdateFromBounds(leo::Drawing::ConstBitmapPtr,
					const leo::Drawing::Rect&) lnothrow;

			/*!
			\pre 间接断言：本机句柄非空。
			*/
			void
				UpdatePremultipliedTo(NativeWindowHandle, leo::Drawing::AlphaType = 0xFF,
					const leo::Drawing::Point& = {});
#	endif

			/*!
			\pre 间接断言：本机句柄非空。
			\since build 589
			*/
			void
				UpdateTo(NativeWindowHandle, const leo::Drawing::Point& = {}) lnothrow;

#	if LFL_Win32
			/*!
			\pre 间接断言：本机句柄非空。
			*/
			void
				UpdateToBounds(NativeWindowHandle, const leo::Drawing::Rect&,
					const leo::Drawing::Point& = {}) lnothrow;
#	endif

			/*!
			\brief 交换。
			*/
			LF_API friend void
				swap(ScreenBuffer&, ScreenBuffer&) lnothrow;
		};


		/*!
		\brief 虚拟屏幕区域缓存。
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
		\brief 窗口内存表面：储存窗口上的二维图形绘制状态。
		\note 仅对内存上下文有所有权。
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
		\brief 窗口设备上下文。
		\note 仅对设备上下文有所有权。
		*/
		class LF_API WindowDeviceContext : public WindowDeviceContextBase
		{
		protected:
			/*!
			\pre 间接断言：参数非空。
			\throw leo::LoggedEvent 初始化失败。
			*/
			WindowDeviceContext(NativeWindowHandle);
			~WindowDeviceContext();
		};


		/*!
		\brief 多页面位图数据。
		\note 用于隐藏实现的非公开接口。
		*/
		class PaintStructData
		{
		public:
			struct Data;

		private:
			//! \note 保持和 \c ::PAINTSTRUCT 二进制兼容。
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
		\brief 窗口区域设备上下文。
		\note 仅对设备上下文有所有权。
		*/
		class LF_API WindowRegionDeviceContext : protected PaintStructData,
			public WindowDeviceContextBase, private leo::noncopyable
		{
		protected:
			/*!
			\pre 间接断言：参数非空。
			\throw leo::LoggedEvent 初始化失败。
			*/
			WindowRegionDeviceContext(NativeWindowHandle);
			~WindowRegionDeviceContext();

		public:
			/*!
			\brief 判断背景是否仍然有效而无需整体重新绘制。
			*/
			bool
				IsBackgroundValid() const lnothrow;

			/*!
			\exception 异常中立：由 leo::CheckArithmetic 抛出。
			*/
			leo::Drawing::Rect
				GetInvalidatedArea() const;
		};
		//@}


		/*!
		\brief 显示区域表面：储存显示区域上的二维图形绘制状态。
		\warning 非虚析构。
		*/
		template<typename _type = WindowDeviceContext>
		class GSurface : public _type, public WindowMemorySurface
		{
		public:
			/*!
			\pre 间接断言：参数非空。
			\exception leo::LoggedEvent 基类子对象初始化失败。
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