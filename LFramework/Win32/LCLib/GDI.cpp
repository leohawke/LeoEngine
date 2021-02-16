#include "GDI.h"
#include "Mingw32.h"
#include "../../Core/LCoreUtilities.h"

#if LF_Hosted

using namespace leo;
using namespace Drawing;
using namespace platform_ex;
using namespace platform_ex::Windows;

#	if LFL_Win32
namespace platform_ex
{
	namespace Windows {
namespace {
	::RECT
		FetchWindowRect(::HWND h_wnd)
	{
		::RECT rect;

		LCL_CallF_Win32(GetWindowRect, h_wnd, &rect);
		return rect;
	}

	Size
		FetchSizeFromBounds(const ::RECT& rect)
	{
		return { CheckArithmetic<SDst>(rect.right - rect.left, "width"),
			CheckArithmetic<SDst>(rect.bottom - rect.top, "height") };
	}

	Rect
		FetchRectFromBounds(const ::RECT& rect)
	{
		return { rect.left, rect.top, FetchSizeFromBounds(rect) };
	}
}

void
GDIObjectDelete::operator()(pointer h) const lnothrow
{
	static_assert(std::is_same<pointer, ::HGDIOBJ>::value,
		"Mismatched type found.");

	LFL_CallWin32F_Trace(DeleteObject, h);
}


::HBITMAP
CreateCompatibleDIBSection(const leo::Drawing::Size& s, BitmapPtr& p_buffer)
{
	// NOTE: There is no resolution information created. See https://msdn.microsoft.com/en-us/library/dd183494.aspx.
	// NOTE: It would return %ERROR_INVALID_PARAMETER for many calls if
	//	allocated memory is not on 32-bit boundary. Anyway it is not a matter
	//	here because the pixel is at least 32-bit. See
	//	http://msdn2.microsoft.com/en-us/library/ms532292.aspx and https://msdn.microsoft.com/en-us/library/dd183494.aspx.
	// NOTE: Bitmap format is hard coded here for explicit buffer
	//	compatibility. %::CreateCompatibleBitmap is not fit for unknown
	//	windows.
	::BITMAPINFO bmi{ { sizeof(::BITMAPINFOHEADER), CheckPositive<long>(
		s.Width, "width"), -CheckPositive<long>(s.Height,
			"height") - 1, 1, 32, BI_RGB, static_cast<unsigned long>(
				sizeof(Pixel) * s.Width * s.Height), 0, 0, 0, 0 },{} };
	void* p_buf{};
	const auto h(LCL_CallF_Win32(CreateDIBSection, {}, &bmi, DIB_RGB_COLORS,
		&p_buf, {}, 0));

	if LB_LIKELY(p_buf)
		p_buffer = static_cast<BitmapPtr>(p_buf);
	else
		// XXX: This should not occur unless there are bugs in implementation of
		//	%::CreateCompatibleDIBSection.
		throw std::runtime_error("Failed writing pointer"
			" @ CreateCompatibleDIBSection.");
	return h;
}

ScreenBuffer::ScreenBuffer(const Size& s)
	: size(s), p_bitmap(CreateCompatibleDIBSection(s, p_buffer))
{}
ScreenBuffer::ScreenBuffer(ScreenBuffer&&) lnothrow = default;

ImplDeDtor(ScreenBuffer)

void
ScreenBuffer::Resize(const Size& s)
{
	if (s != size)
		*this = ScreenBuffer(s);
}

void
ScreenBuffer::Premultiply(ConstBitmapPtr p_buf) lnothrow
{
	LAssertNonnull(p_buf);
	// NOTE: Since the stride is guaranteed equal to the width, the storage for
	//	pixels can be supposed to be contiguous.
	std::transform(p_buf, p_buf + size.Width * size.Height, p_buffer,
		[](const Pixel& pixel) lnothrow{
		const auto a(PixelGetA(pixel));

	return PixelCtor(MonoType(PixelGetB(pixel) * a / 0xFF),
		MonoType(PixelGetG(pixel) * a / 0xFF),
		MonoType(PixelGetR(pixel) * a / 0xFF), a);
	});
}

void
ScreenBuffer::UpdateFromBounds(ConstBitmapPtr p_buf, const Rect& r) lnothrow
{
	BlitLines<false, false>(CopyLine<true>(), GetBufferPtr(), Nonnull(p_buf),
		size, size, r.GetPoint(), r.GetPoint(), r.GetSize());
}

void
ScreenBuffer::UpdatePremultipliedTo(NativeWindowHandle h_wnd, AlphaType a,
	const Point& pt)
{
	GSurface<> sf(h_wnd);

	sf.UpdatePremultiplied(*this, h_wnd, a, pt);
}


WindowMemorySurface::WindowMemorySurface(::HDC h_dc)
	: h_owner_dc(h_dc), h_mem_dc(::CreateCompatibleDC(h_dc))
{}
WindowMemorySurface::~WindowMemorySurface()
{
	::DeleteDC(h_mem_dc);
}

void
WindowMemorySurface::UpdateBounds(ScreenBuffer& sbuf, const Rect& r,
	const Point& sp) lnothrow
{
	// NOTE: %Nonnull is for invariant check.
	const auto h_old(::SelectObject(h_mem_dc, Nonnull(sbuf.GetNativeHandle())));

	if (h_old)
	{
		LFL_CallWin32F_Trace(BitBlt, h_owner_dc, int(r.X), int(r.Y),
			int(r.Width), int(r.Height), h_mem_dc, int(sp.X), int(sp.Y),
			SRCCOPY);
		if LB_UNLIKELY(!::SelectObject(h_mem_dc, h_old))
			TraceDe(Err, "Restoring GDI object failed"
				" @ WindowMemorySurface::UpdateBounds.");
	}
	else
		TraceDe(Err, "Setting GDI object failed"
			" @ WindowMemorySurface::UpdateBounds.");
}

void
WindowMemorySurface::UpdatePremultiplied(ScreenBuffer& sbuf,
	NativeWindowHandle h_wnd, leo::Drawing::AlphaType a, const Point& sp)
	lnothrow
{
	const auto h_old(::SelectObject(h_mem_dc, sbuf.GetNativeHandle()));
	auto rect(FetchWindowRect(h_wnd));
	::SIZE size{ rect.right - rect.left, rect.bottom - rect.top };
	::POINT pt{ sp.X, sp.Y };
	::BLENDFUNCTION bfunc{ AC_SRC_OVER, 0, a, AC_SRC_ALPHA };

	LFL_CallWin32F_Trace(UpdateLayeredWindow, h_wnd, h_owner_dc,
		leo::aligned_store_cast<::POINT*>(&rect), &size, h_mem_dc, &pt, 0,
		&bfunc, ULW_ALPHA);
	::SelectObject(h_mem_dc, h_old);
}


WindowDeviceContext::WindowDeviceContext(NativeWindowHandle h_wnd)
// NOTE: Painting using %::GetDC and manually managing clipping areas
//	instead of %::GetDCEx, for performance and convenience calculation
//	of input boundary.
	: WindowDeviceContextBase(h_wnd, ::GetDC(Nonnull(h_wnd)))
{
	if LB_UNLIKELY(!hDC)
		throw LoggedEvent("Retrieving device context failure.");
}
WindowDeviceContext::~WindowDeviceContext()
{
	::ReleaseDC(hWindow, hDC);
}


struct PaintStructData::Data final : ::PAINTSTRUCT
{};


PaintStructData::PaintStructData()
	: pun(leo::default_init, &ps)
{
	static_assert(leo::is_aligned_storable<decltype(ps), Data>(),
		"Invalid buffer found.");
}
ImplDeDtor(PaintStructData)


WindowRegionDeviceContext::WindowRegionDeviceContext(NativeWindowHandle h_wnd)
	: PaintStructData(),
	WindowDeviceContextBase(h_wnd, ::BeginPaint(Nonnull(h_wnd), &Get()))
{
	if LB_UNLIKELY(!hDC)
		throw LoggedEvent("Device context initialization failure.");
}
WindowRegionDeviceContext::~WindowRegionDeviceContext()
{
	// NOTE: Return value is ignored since it is always %TRUE.
	::EndPaint(hWindow, &Get());
}

bool
WindowRegionDeviceContext::IsBackgroundValid() const lnothrow
{
	return !bool(Get().fErase);
}

Rect
WindowRegionDeviceContext::GetInvalidatedArea() const
{
	return FetchRectFromBounds(Get().rcPaint);
}

void
ScreenBuffer::UpdateToBounds(NativeWindowHandle h_wnd, const Rect& r,
	const Point& sp) lnothrow
{
	GSurface<>(h_wnd).UpdateBounds(*this, r, sp);
}


void
ScreenBuffer::UpdateFrom(ConstBitmapPtr p_buf)
{
	// NOTE: Since the pitch is guaranteed equal to the width, the storage for
	//	pixels can be supposed to be contiguous.
	//CopyBitmapBuffer(GetBufferPtr(), p_buf, size);
	throw leo::unimplemented();
}



void
ScreenBuffer::UpdateTo(NativeWindowHandle h_wnd, const Point& pt) lnothrow
{
	GSurface<>(h_wnd).UpdateBounds(*this, { pt, GetSize() });
}



		void
			swap(ScreenBuffer& x, ScreenBuffer& y) lnothrow
		{
			std::swap(x.size, y.size),
				std::swap(x.p_buffer, y.p_buffer),
				std::swap(x.p_bitmap, y.p_bitmap);
		}
	}
}

#endif


#endif