#include "HostGUI.h"
#include "NativeAPI.h"
#include "../Core/LCoreUtilities.h"

#if LFL_Win32
#include "../Win32/LCLib/Mingw32.h"
#include "../Win32/LCLib/NLS.h"
#	include <Shellapi.h> // for ::ShellExecuteW;
#include "Input.h"
#include <LBase/mixin.hpp>
#endif

using namespace leo;
using namespace Drawing;
using leo::string;
using platform::string_view;

#if LF_Hosted

namespace platform_ex
{
	using namespace Windows;

	void
		HostWindowDelete::operator()(pointer p) const lnothrow
	{
#	if LFL_HostedUI_XCB
		default_delete<XCB::WindowData>()(p.get());
#	elif LFL_Win32
		// NOTE: The window could be already destroyed in window procedure.
		if (::IsWindow(p))
			// XXX: Error ignored.
			::DestroyWindow(p);
#	elif LFL_Android
		// XXX: Error ignored.
		::ANativeWindow_release(p);
#	endif
	}

	namespace
	{

#	if LFL_HostedUI_XCB || LFL_Android
		SDst
			CheckStride(SDst buf_stride, SDst w)
		{
			if (buf_stride < w)
				// XXX: Use more specified exception type.
				throw std::runtime_error("Stride is small than width.");
			return buf_stride;
		}
#	elif LFL_Win32
		void
			MoveWindow(::HWND h_wnd, SPos x, SPos y)
		{
			LCL_CallF_Win32(SetWindowPos, h_wnd, {}, x, y, 0, 0, SWP_ASYNCWINDOWPOS
				| SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSENDCHANGING
				| SWP_NOSIZE | SWP_NOZORDER);
		}

		void
			ResizeWindow(::HWND h_wnd, SDst w, SDst h)
		{
			LCL_CallF_Win32(SetWindowPos, h_wnd, {}, 0, 0, int(w), int(h),
				SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER
				| SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER);
		}

		::RECT
			FetchWindowRect(::HWND h_wnd)
		{
			::RECT rect;

			LCL_CallF_Win32(GetWindowRect, h_wnd, &rect);
			return rect;
		}

		//@{
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

		//! \since build 639
		inline WindowStyle
			FetchWindowStyle(::HWND h_wnd)
		{
			return WindowStyle(::GetWindowLongW(h_wnd, GWL_STYLE));
		}

		void
			AdjustWindowBounds(::RECT& rect, ::HWND h_wnd, bool b_menu = {})
		{
			LCL_CallF_Win32(AdjustWindowRect, &rect, FetchWindowStyle(h_wnd), b_menu);
			FetchSizeFromBounds(rect);
		}

		void
			SetWindowBounds(::HWND h_wnd, int x, int y, SDst w, SDst h)
		{
			LCL_CallF_Win32(SetWindowPos, h_wnd, {}, x, y, int(w), int(h),
				SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOOWNERZORDER
				| SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER);
		}
		//@}


		inline unique_ptr_from<GlobalDelete>
			MakeMoveableGlobalMemory(size_t size)
		{
			auto p(unique_raw(::GlobalAlloc(GMEM_MOVEABLE, size), GlobalDelete()));

			if (LB_UNLIKELY(!p))
				// TODO: Use inherited class of exception.
				throw std::bad_alloc();
			return p;
		}

		template<typename _tChar, class _tString>
		LB_ALLOCATOR LB_ATTR_returns_nonnull void*
			CopyGlobalString(const _tString& str)
		{
			const auto len(str.length());
			auto p(MakeMoveableGlobalMemory((len + 1) * sizeof(_tChar)));
			{
				const GlobalLocked gl(Nonnull(p));
				const auto p_buf(gl.GetPtr<_tChar>().get());

				leo::ntctscpy(p_buf, str.data(), len);
			}
			return p.release();
		}
#	endif

	} // unnamed namespace;


#	if LFL_Win32
	void
		BindDefaultWindowProc(NativeWindowHandle h_wnd, MessageMap& m, unsigned msg,
			EventPriority prior)
	{
		m[msg].Add(
			[=](::WPARAM w_param, ::LPARAM l_param, ::LRESULT& res) lnothrow{
			res = ::DefWindowProcW(h_wnd, msg, w_param, l_param);
		}, prior);
	}
#	endif


#	if LFL_HostedUI_XCB
	XCB::WindowData&
		WindowReference::Deref() const
	{
		if (const auto h = GetNativeHandle().get())
			return *h;
		throw std::runtime_error("Null window reference found.");
	}
#	elif LFL_Win32
	bool
		WindowReference::IsMaximized() const lnothrow
	{
		return ::IsZoomed(GetNativeHandle());
	}
	bool
		WindowReference::IsMinimized() const lnothrow
	{
		return ::IsIconic(GetNativeHandle());
	}
	bool
		WindowReference::IsVisible() const lnothrow
	{
		return ::IsWindowVisible(GetNativeHandle());
	}

	Rect
		WindowReference::GetBounds() const
	{
		return FetchRectFromBounds(FetchWindowRect(GetNativeHandle()));
	}
	Rect
		WindowReference::GetClientBounds() const
	{
		return { GetClientLocation(), GetClientSize() };
	}
	Point
		WindowReference::GetClientLocation() const
	{
		::POINT point{ 0, 0 };

		LCL_CallF_Win32(ClientToScreen, GetNativeHandle(), &point);
		return { point.x, point.y };
	}
	Size
		WindowReference::GetClientSize() const
	{
		::RECT rect;

		LCL_CallF_Win32(GetClientRect, GetNativeHandle(), &rect);
		return { rect.right, rect.bottom };
	}
	Point
		WindowReference::GetCursorLocation() const
	{
		::POINT cursor;

		LCL_CallF_Win32(GetCursorPos, &cursor);
		LCL_CallF_Win32(ScreenToClient, GetNativeHandle(), &cursor);
		return { cursor.x, cursor.y };
	}
	Point
		WindowReference::GetLocation() const
	{
		const auto& rect(FetchWindowRect(GetNativeHandle()));

		return { rect.left, rect.top };
	}
	leo::Drawing::AlphaType
		WindowReference::GetOpacity() const
	{
		byte a;

		LCL_CallF_Win32(GetLayeredWindowAttributes, GetNativeHandle(), {}, &a, {});
		return a;
	}
	Size
		WindowReference::GetSize() const
	{
		return FetchSizeFromBounds(FetchWindowRect(GetNativeHandle()));
	}

	void
		WindowReference::SetBounds(const Rect& r)
	{
		SetWindowBounds(GetNativeHandle(), r.X, r.Y, r.Width, r.Height);
	}
	void
		WindowReference::SetClientBounds(const Rect& r)
	{
		::RECT rect{ r.X, r.Y, CheckArithmetic<SPos>(r.GetRight(), "width"),
			CheckArithmetic<SPos>(r.GetBottom(), "height") };
		const auto h_wnd(GetNativeHandle());

		AdjustWindowBounds(rect, h_wnd);
		SetWindowBounds(h_wnd, rect.left, rect.top, SDst(rect.right - rect.left),
			SDst(rect.bottom - rect.top));
	}
	void
		WindowReference::SetOpacity(leo::Drawing::AlphaType a)
	{
		LCL_CallF_Win32(SetLayeredWindowAttributes, GetNativeHandle(), 0, a,
			LWA_ALPHA);
	}
	WindowReference
		WindowReference::GetParent() const
	{
		// TODO: Implementation.
		throw leo::unimplemented();
	}

	void
		WindowReference::SetText(const wchar_t* str)
	{
		TraceDe(Debug, "Setting text '%p' to window reference '%p'...",
			leo::pvoid(str), leo::pvoid(this));
		LCL_CallF_Win32(SetWindowTextW, GetNativeHandle(), str);
	}

	void
		WindowReference::Close()
	{
		TraceDe(Debug, "Closing window reference '%p'...", leo::pvoid(this));
		LCL_CallF_Win32(SendNotifyMessageW, GetNativeHandle(), WM_CLOSE, 0, 0);
	}

	void
		WindowReference::Invalidate()
	{
		TraceDe(Debug, "Invalidating window reference '%p'...",
			leo::pvoid(this));
		LCL_CallF_Win32(InvalidateRect, GetNativeHandle(), {}, {});
	}

	void
		WindowReference::Move(const Point& pt)
	{
		MoveWindow(GetNativeHandle(), pt.X, pt.Y);
	}

	void
		WindowReference::MoveClient(const Point& pt)
	{
		::RECT rect{ pt.X, pt.Y, pt.X, pt.Y };
		const auto h_wnd(GetNativeHandle());

		AdjustWindowBounds(rect, h_wnd);
		MoveWindow(h_wnd, rect.left, rect.top);
	}

	void
		WindowReference::Resize(const Size& s)
	{
		ResizeWindow(GetNativeHandle(), s.Width, s.Height);
	}

	void
		WindowReference::ResizeClient(const Size& s)
	{
		::RECT rect{ 0, 0, CheckArithmetic<SPos>(s.Width, "width"),
			CheckArithmetic<SPos>(s.Height, "height") };
		const auto h_wnd(GetNativeHandle());

		AdjustWindowBounds(rect, h_wnd);
		ResizeWindow(h_wnd, SDst(rect.right - rect.left),
			SDst(rect.bottom - rect.top));
	}

	bool
		WindowReference::Show(int n_cmd_show) lnothrow
	{
		return ::ShowWindowAsync(GetNativeHandle(), n_cmd_show) != 0;
	}
#	elif LFL_Android
	SDst
		WindowReference::GetWidth() const
	{
		return CheckPositive<SDst>(
			::ANativeWindow_getWidth(GetNativeHandle()), "width");
	}
	SDst
		WindowReference::GetHeight() const
	{
		return CheckPositive<SDst>(::ANativeWindow_getHeight(
			GetNativeHandle()), "height");
	}
#	endif


#	if LFL_HostedUI_XCB
	void
		UpdateContentTo(NativeWindowHandle h_wnd, const Rect& r, const ConstGraphics& g)
	{
		XCB::UpdatePixmapBuffer(Deref(h_wnd.get()), r, g);
	}
#	elif LFL_Win32


	NativeWindowHandle
		CreateNativeWindow(const wchar_t* class_name, const Drawing::Size& s,
			const wchar_t* title, WindowStyle wstyle, WindowStyle wstyle_ex)
	{
		::RECT rect{ 0, 0, CheckArithmetic<SPos>(s.Width, "width"),
			CheckArithmetic<SPos>(s.Height, "height") };

		::AdjustWindowRect(&rect, wstyle, false);
		return ::CreateWindowExW(wstyle_ex, class_name, title, wstyle,
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom
			- rect.top, {}/*HWND_DESKTOP*/, {}, ::GetModuleHandleW({}), {});
	}
#	elif LFL_Android
	void
		UpdateContentTo(NativeWindowHandle h_wnd, const Rect& r, const ConstGraphics& g)
	{
		::ANativeWindow_Buffer abuf;
		::ARect arect{ r.X, r.Y, r.X + r.Width, r.Y + r.Height };
		const auto res(::ANativeWindow_lock(Nonnull(h_wnd), &abuf, &arect));

		if (LB_LIKELY(res == 0))
		{
			BlitLines<false, false>(CopyLine<true>(), BitmapPtr(abuf.bits),
				g.GetBufferPtr(), WindowReference(h_wnd).GetSize(), g.GetSize(),
				r.GetPoint(), {}, r.GetSize());
			::ANativeWindow_unlockAndPost(h_wnd);
		}
		else
			TraceDe(Warning, "::ANativeWindow_lock failed, error = %d.", int(res));
	}
#	endif


#	if LFL_HostedUI_XCB || LFL_Android
	class ScreenBufferData : public CompactPixmap
	{
	public:
		ScreenBufferData(const Size&, SDst);

		DefDeMoveCtor(ScreenBufferData)
	};

	ScreenBufferData::ScreenBufferData(const Size& s, SDst buf_stride)
		: CompactPixmap({}, CheckStride(buf_stride, s.Width), s.Height)
	{}
#	endif


#	if LFL_HostedUI_XCB || LFL_Android
	ScreenBuffer::ScreenBuffer(const Size& s)
		: ScreenBuffer(s, s.Width)
	{}
	ScreenBuffer::ScreenBuffer(const Size& s, SDst buf_stride)
		: p_impl(new ScreenBufferData(s, buf_stride)), width(s.Width)
	{}
	ScreenBuffer::ScreenBuffer(ScreenBuffer&& sbuf) lnothrow
		: p_impl(new ScreenBufferData(std::move(*sbuf.p_impl))), width(sbuf.width)
	{
		sbuf.width = 0;
	}

	ImplDeDtor(ScreenBuffer)
#	endif

#	if LFL_HostedUI_XCB || LFL_Android
		BitmapPtr
		ScreenBuffer::GetBufferPtr() const lnothrow
	{
		return Deref(p_impl).GetBufferPtr();
	}
	leo::Drawing::Graphics
		ScreenBuffer::GetContext() const lnothrow
	{
		return Deref(p_impl).GetContext();
	}
	Size
		ScreenBuffer::GetSize() const lnothrow
	{
		return { width, Deref(p_impl).GetHeight() };
	}
	leo::SDst
		ScreenBuffer::GetStride() const lnothrow
	{
		return Deref(p_impl).GetWidth();
	}

	void
		ScreenBuffer::Resize(const Size& s)
	{
		// TODO: Expand stride for given width using a proper strategy.
		Deref(p_impl).SetSize(s);
		width = s.Width;
	}


	void
		ScreenBuffer::UpdateFrom(ConstBitmapPtr p_buf) lnothrow
	{
		// TODO: Expand stride for given width using a proper strategy.
		CopyBitmapBuffer(Deref(p_impl).GetBufferPtr(), p_buf, GetSize());
	}

	void
		ScreenBuffer::UpdateTo(NativeWindowHandle h_wnd, const Point& pt) lnothrow
	{
		UpdateContentTo(h_wnd, { pt, GetSize() }, GetContext());

	}


	void
		swap(ScreenBuffer& x, ScreenBuffer& y) lnothrow
	{
		swap(Deref(x.p_impl), Deref(y.p_impl)),
			std::swap(x.width, y.width);
	}
#	endif




#ifdef LFL_Win32

	WindowClass::WindowClass(const wchar_t* class_name, ::WNDPROC wnd_proc,
		unsigned style, ::HBRUSH h_bg, ::HINSTANCE h_inst)
		// NOTE: Intentionally no %CS_OWNDC or %CS_CLASSDC, so %::ReleaseDC
		//	is always needed.
		: WindowClass(::WNDCLASSW{ style, wnd_proc ? wnd_proc
			: HostWindow::WindowProcedure, 0, 0, h_inst
			? h_inst : ::GetModuleHandleW({}), ::LoadIconW({}, IDI_APPLICATION),
			::LoadCursorW({}, IDC_ARROW), h_bg, nullptr, Nonnull(class_name) })
	{}
	WindowClass::WindowClass(const ::WNDCLASSW& wc)
		: WindowClass(wc.lpszClassName, LCL_CallF_Win32(RegisterClassW, &wc),
			wc.hInstance)
	{}
	WindowClass::WindowClass(const ::WNDCLASSEXW& wc)
		: WindowClass(wc.lpszClassName, LCL_CallF_Win32(RegisterClassExW, &wc),
			wc.hInstance)
	{}
	WindowClass::WindowClass(wstring_view class_name,
		unsigned short class_atom, ::HINSTANCE h_inst)
		: name((Nonnull(class_name.data()), class_name)), atom(class_atom),
		h_instance(h_inst)
	{
		if (LB_UNLIKELY(atom == 0))
			throw std::invalid_argument("Invalid atom value found.");
		TraceDe(Notice, "Window class '%s' of atom '%hu' registered.",
			name.empty() ? "<unknown>" : WCSToUTF8(name).c_str(), atom);
	}
	WindowClass::~WindowClass()
	{
		::UnregisterClassW(reinterpret_cast<const wchar_t*>(atom), h_instance);
		FilterExceptions([this] {
			TraceDe(Notice, "Window class '%s' of atom '%hu' unregistered.",
				name.empty() ? "<unknown>" : WCSToUTF8(name).c_str(), atom);
		}, lfsig);
	}
#endif


	HostWindow::HostWindow(NativeWindowHandle h)
		: WindowReference(Nonnull(h))
#	if LFL_HostedUI_XCB
		, WM_PROTOCOLS(platform::Deref(h.get()).LookupAtom("WM_PROTOCOLS"))
		, WM_DELETE_WINDOW(h.get()->LookupAtom("WM_DELETE_WINDOW"))
#	endif
#	if LFL_HostedUI_XCB || LFL_Win32
		, MessageMap()
#	endif
	{
#	if LFL_HostedUI_XCB
		h.get()->GetConnectionRef().Check();
#	elif LFL_Win32
		LAssert(::IsWindow(h), "Invalid window handle found.");
		LAssert(::GetWindowThreadProcessId(h, {}) == ::GetCurrentThreadId(),
			"Window not created on current thread found.");
		LAssert(::GetWindowLongPtrW(h, GWLP_USERDATA) == 0,
			"Invalid user data of window found.");

		wchar_t buf[size(WindowClassName)];

		LCL_CallF_Win32(GetClassNameW, GetNativeHandle(), buf,
			int(size(WindowClassName)));
		if (std::wcscmp(buf, WindowClassName) != 0)
			throw GeneralEvent("Wrong windows class name found.");
		::SetLastError(0);
		if (LB_UNLIKELY(::SetWindowLongPtrW(GetNativeHandle(), GWLP_USERDATA,
			::LONG_PTR(this)) == 0 && GetLastError() != 0))
			LCL_Raise_Win32E("SetWindowLongPtrW", lfsig);
		LCL_CallF_Win32(SetWindowPos, GetNativeHandle(), {}, 0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW
			| SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER);

		::RAWINPUTDEVICE rid{ 0x01, 0x02, 0, nullptr };

		LCL_CallF_Win32(RegisterRawInputDevices, &rid, 1, sizeof(rid));
		MessageMap[WM_DESTROY] += []() lnothrow{
			::PostQuitMessage(0);
		TraceDe(Debug, "Host quit message posted.");
		};
#	elif LFL_Android
		::ANativeWindow_acquire(GetNativeHandle());
#	endif
	}

	HostWindow::~HostWindow()
	{
		const unique_ptr_from<HostWindowDelete> p_wnd(GetNativeHandle());

#	if LFL_Win32
		// XXX: Error ignored.
		::SetWindowLongPtrW(p_wnd.get(), GWLP_USERDATA, ::LONG_PTR());
#	endif
	}

#	if LFL_Win32
	Point
		HostWindow::MapPoint(const Point& pt) const
	{
		return pt;
	}

	::LRESULT __stdcall
		HostWindow::WindowProcedure(::HWND h_wnd, unsigned msg, ::WPARAM w_param,
			::LPARAM l_param) lnothrowv
	{
		if (const auto p = reinterpret_cast<HostWindow*>(
			::GetWindowLongPtrW(h_wnd, GWLP_USERDATA)))
		{
			auto& m(p->MessageMap);

			try
			{
				const auto i(m.find(msg));

				if (i != m.cend())
				{
					::LRESULT res(0);

					i->second(w_param, l_param, res);
					return res;
				}
			}
			CatchExpr(..., TraceDe(Warning, "HostWindow::WindowPrecedure failed."))
		}
		return ::DefWindowProcW(h_wnd, msg, w_param, l_param);
	}
#	endif


	WindowInputHost::WindowInputHost(HostWindow& wnd)
		: Window(wnd)
#	if LFL_Win32
		, has_hosted_caret(::CreateCaret(wnd.GetNativeHandle(), {}, 1, 1))
#	endif
	{
#	if LFL_Win32
		auto& m(wnd.MessageMap);

		lunseq(
			m[WM_MOVE] += [this] {
			UpdateCandidateWindowLocation();
		},
			m[WM_KILLFOCUS] += [] {
			ClearKeyStates();
		},
			m[WM_INPUT] += [this](::WPARAM, ::LPARAM l_param) lnothrow{
			::RAWINPUT ri;
		unsigned size(sizeof(ri));

		// TODO: Use '{}' to simplify initialization after CWG 1368 resolved by
		//	C++14. See $2015-09 @ %Documentation::Workflow::Annual2015.
		leo::trivially_fill_n(&ri);
		if (LB_LIKELY(::GetRawInputData(::HRAWINPUT(l_param), RID_INPUT, &ri,
			&size, sizeof(::RAWINPUTHEADER)) != unsigned(-1) && ri.header.dwType
			== RIM_TYPEMOUSE) && ri.data.mouse.usButtonFlags == RI_MOUSE_WHEEL)
			// NOTE: This value is safe to cast because it is
			//	specified as a signed value, see https://msdn.microsoft.com/en-us/library/windows/desktop/ms645578(v=vs.85).aspx.
			RawMouseButton = short(ri.data.mouse.usButtonData);
		},
			m[WM_CHAR] += [this](::WPARAM w_param, ::LPARAM l_param) {
			lock_guard<recursive_mutex> lck(input_mutex);
			size_t n(l_param & 0x7FFF);

			while (n-- != 0)
				comp_str += char16_t(w_param);
		},
			m[WM_IME_COMPOSITION] += [this] {
			UpdateCandidateWindowLocation();
		}
		);
		leo::unseq_apply([&](int msg) {
			BindDefaultWindowProc(wnd.GetNativeHandle(), m, unsigned(msg));
		}, WM_MOVE, WM_IME_COMPOSITION);
#	endif
	}
	WindowInputHost::~WindowInputHost()
	{
#	if LFL_Win32
		if (has_hosted_caret)
			::DestroyCaret();
#	endif
	}

#	if LFL_Win32
	void
		WindowInputHost::UpdateCandidateWindowLocation()
	{
		lock_guard<recursive_mutex> lck(input_mutex);

		UpdateCandidateWindowLocationUnlocked();
	}
	void
		WindowInputHost::UpdateCandidateWindowLocation(const Point& pt)
	{
		if (pt != Point::Invalid)
		{
			lock_guard<recursive_mutex> lck(input_mutex);

			caret_location = pt;
			UpdateCandidateWindowLocationUnlocked();
		}
	}
	void
		WindowInputHost::UpdateCandidateWindowLocationUnlocked()
	{
		if (LB_LIKELY(caret_location != Point::Invalid))
		{
			TraceDe(Informative, "Update composition form position: %s.",
				to_string(caret_location).c_str());

			const auto h_wnd(Nonnull(Window.GetNativeHandle()));

			if (const auto h_imc = ::ImmGetContext(h_wnd))
			{
				// FIXME: Correct location?
				const auto client_pt(caret_location + Window.GetClientLocation()
					- Window.GetClientLocation());

				::CANDIDATEFORM cand_form{ 0, CFS_CANDIDATEPOS,
				{ client_pt.X, client_pt.Y },{ 0, 0, 0, 0 } };

				// TODO: Error handling.
				::ImmSetCandidateWindow(h_imc, &cand_form);
				::ImmReleaseContext(h_wnd, h_imc);
			}
			// FIXME: Correct implementation for non-Chinese IME.
			// NOTE: See comment on %IMM32Manager::MoveImeWindow in
			//	https://src.chromium.org/viewvc/chrome/trunk/src/ui/base/ime/win/imm32_manager.cc.
			::SetCaretPos(caret_location.X, caret_location.Y);
		}
	}
#	endif


#	if LFL_Win32
	class Clipboard::Data : public GlobalLocked
	{
		using GlobalLocked::GlobalLocked;
	};


	Clipboard::Clipboard(NativeWindowHandle h_wnd)
	{
		// FIXME: Spin for remote desktops?
		LCL_CallF_Win32(OpenClipboard, h_wnd);
	}
	Clipboard::~Clipboard()
	{
		LFL_CallWin32F_Trace(CloseClipboard, );
	}

	bool
		Clipboard::IsAvailable(FormatType fmt) lnothrow
	{
		return bool(::IsClipboardFormatAvailable(fmt));
	}

	void
		Clipboard::CheckAvailable(FormatType fmt)
	{
		LCL_CallF_Win32(IsClipboardFormatAvailable, fmt);
	}

	void
		Clipboard::Clear() lnothrow
	{
		LFL_CallWin32F_Trace(EmptyClipboard, );
	}

	NativeWindowHandle
		Clipboard::GetOpenWindow() lnothrow
	{
		return ::GetOpenClipboardWindow();
	}

	bool
		Clipboard::Receive(leo::string& str)
	{
		return ReceiveRaw(CF_TEXT, [&](const Data& d) lnothrowv{
			str = d.GetPtr<char>().get();
		});
	}
	bool
		Clipboard::Receive(leo::String& str)
	{
		return ReceiveRaw(CF_UNICODETEXT, [&](const Data& d) lnothrowv{
			str = d.GetPtr<char16_t>().get();
		});
	}

	bool
		Clipboard::ReceiveRaw(FormatType fmt, std::function<void(const Data&)> f)
	{
		if (IsAvailable(fmt))
			if (const auto h = ::GetClipboardData(fmt))
			{
				const Data d(h);

				Nonnull(f)(d);
				return true;
			}
		return {};
	}

	void
		Clipboard::Send(string_view sv)
	{
		LAssertNonnull(sv.data());
		SendRaw(CF_TEXT, CopyGlobalString<char>(sv));
	}
	void
		Clipboard::Send(u16string_view sv)
	{
		LAssertNonnull(sv.data());
		SendRaw(CF_UNICODETEXT, CopyGlobalString<char16_t>(sv));
	}

	void
		Clipboard::SendRaw(FormatType fmt, void* h)
	{
		Clear();
		LCL_CallF_Win32(SetClipboardData, fmt, h);
	}


	void
		ExecuteShellCommand(const wchar_t* cmd, const wchar_t* args, bool use_admin,
			const wchar_t* dir, int n_cmd_show, NativeWindowHandle h_parent)
	{
		// TODO: Set current working directory as %USERPROFILE%?
		auto res(int(std::intptr_t(::ShellExecuteW(h_parent,
			use_admin ? L"runas" : nullptr, Nonnull(cmd), args, dir, n_cmd_show))));

		switch (res)
		{
		case 0:
		case SE_ERR_OOM:
			// TODO: Use inherited class of exception.
			throw std::bad_alloc();
		case SE_ERR_SHARE:
		case SE_ERR_DLLNOTFOUND:
			res = SE_ERR_SHARE ? ERROR_SHARING_VIOLATION : ERROR_DLL_NOT_FOUND;
			LB_ATTR_fallthrough;
		case ERROR_FILE_NOT_FOUND: // NOTE: Same as %SE_ERR_FNF.
		case ERROR_PATH_NOT_FOUND: // NOTE: Same as %SE_ERR_PNF.
		case ERROR_ACCESS_DENIED: // NOTE: Same as %SE_ERR_ACCESSDENIED.
		case ERROR_BAD_FORMAT:
			throw Win32Exception(ErrorCode(res), "ShellExecuteW", Err);
		case SE_ERR_ASSOCINCOMPLETE:
		case SE_ERR_NOASSOC:
		case SE_ERR_DDETIMEOUT:
		case SE_ERR_DDEFAIL:
		case SE_ERR_DDEBUSY:
		{
			using boxed_exception
				= leo::mixin<std::runtime_error, leo::boxed_value<int>>;

			TryExpr(throw
				boxed_exception{ std::runtime_error("ShellExecuteW"), res })
				catch (boxed_exception& e)
			{
				const auto throw_ex([=](int ec) LB_ATTR(noreturn) {
					std::throw_with_nested(Win32Exception(ErrorCode(ec),
						leo::sfmt("ShellExecuteW: %d", res), Err));
				});

				switch (e.value)
				{
				case SE_ERR_ASSOCINCOMPLETE:
				case SE_ERR_NOASSOC:
					throw_ex(ERROR_NO_ASSOCIATION);
				case SE_ERR_DDETIMEOUT:
				case SE_ERR_DDEFAIL:
				case SE_ERR_DDEBUSY:
					throw_ex(ERROR_DDE_FAIL);
				default:
					break;
				}
			}
			LAssert(false, "Invalid state found.");
		}
		LB_ATTR_fallthrough;
		default:
			if (res > 32)
				TraceDe(Informative, "ExecuteShellCommand: ::ShellExecute call"
					" succeed with return value %d.", res);
			else
				throw LoggedEvent(leo::sfmt("ExecuteShellCommand:"
					" ::ShellExecuteW call failed" " with unknown error '%d'.",
					res), Err);
		}
	}
#	endif

} // namespace platform_ex;

#endif