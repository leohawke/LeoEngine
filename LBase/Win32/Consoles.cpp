#include <LBase/Platform.h>
#if LFL_Win32
#include <LBase/Win32/Consoles.h>  // for UTF8ToWCS;
#include <LBase/LCoreUtilities.h> // for leo::CheckPositiveScalar;
using namespace leo;
#endif

namespace platform_ex
{
#if LFL_Win32

	inline namespace Windows
	{
		WConsole::WConsole(unsigned long dev)
			: WConsole(::GetStdHandle(dev))
		{}
		WConsole::WConsole(::HANDLE h)
			: h_std(h), saved_attr(GetScreenBufferInfo().wAttributes),
			Attributes(saved_attr)
		{
			LAssert(h_std && h_std != INVALID_HANDLE_VALUE, "Invalid handle found.");
		}
		WConsole::~WConsole()
		{
			FilterExceptions(std::bind(&WConsole::RestoreAttributes, this), lfsig);
		}

		::CONSOLE_SCREEN_BUFFER_INFO
			WConsole::GetScreenBufferInfo() const
		{
			::CONSOLE_SCREEN_BUFFER_INFO info;

			LFL_CallWin32F(GetConsoleScreenBufferInfo, h_std, &info);
			return info;
		}

		void
			WConsole::SetSystemColor(std::uint8_t color)
		{
			char cmd[9];

			std::sprintf(cmd, "COLOR %02x", int(color));
			std::system(cmd);
		}

		void
			WConsole::SetBackColor(std::uint8_t bc) lnothrow
		{
			Attributes = ComposeAttributes(FetchForeColor(Attributes), bc);
		}
		void
			WConsole::SetCursorPosition(::COORD pos)
		{
			// NOTE: %::SetConsoleCursorPosition expects 1-based.
			LFL_CallWin32F(SetConsoleCursorPosition, h_std, { short(pos.X + 1), short(pos.Y + 1) });
		}
		void
			WConsole::SetForeColor(std::uint8_t fc) lnothrow
		{
			Attributes = ComposeAttributes(fc, FetchBackColor(Attributes));
		}

		::WORD
			WConsole::ComposeAttributes(std::uint8_t fore, std::uint8_t back) lnothrow
		{
			return (fore & 15) | ((back & 15) << 4);
		}

		void
			WConsole::CursorUp(size_t num_rows)
		{
			if (num_rows != 0)
			{
				const auto pos(GetCursorPosition());

				// XXX: Conversion to 'short' might be implementation-defined.
				SetCursorPosition({ short(pos.Y - short(num_rows)), pos.X });
			}
		}

		void
			WConsole::Erase(wchar_t c)
		{
			const auto size(GetScreenBufferInfo().dwSize);

			Fill({ short(), short() }, CheckPositiveScalar<unsigned long>(size.X)
				* CheckPositiveScalar<unsigned long>(size.Y), c);
		}

		void
			WConsole::Fill(::COORD coord, unsigned long n, wchar_t c)
		{
			LFL_CallWin32F(FillConsoleOutputCharacterW, h_std, c, n, coord, {});
			LFL_CallWin32F(FillConsoleOutputAttribute, h_std, Attributes, n, coord, {});
			LFL_CallWin32F(SetConsoleCursorPosition, h_std, { coord.X, coord.Y });
		}

		void
			WConsole::RestoreAttributes()
		{
			//	SetColor();
			Update(GetSavedAttributes());
		}

		void
			WConsole::Update()
		{
			Update(Attributes);
		}
		void
			WConsole::Update(::WORD value)
		{
			LFL_CallWin32F(SetConsoleTextAttribute, h_std, value);
		}

		void
			WConsole::UpdateBackColor(std::uint8_t fc)
		{
			SetBackColor(fc);
			Update();
		}

		void
			WConsole::UpdateForeColor(std::uint8_t fc)
		{
			SetForeColor(fc);
			Update();
		}

		size_t
			WConsole::WriteString(string_view sv)
		{
			LAssertNonnull(sv.data());
			// FIXME: Support for non-BMP characters.
			return WriteString(UTF8ToWCS(sv));
		}
		size_t
			WConsole::WriteString(string_view sv, unsigned cp)
		{
			LAssertNonnull(sv.data());
			return WriteString(MBCSToWCS(sv, cp));
		}
		size_t
			WConsole::WriteString(wstring_view sv)
		{
			LAssertNonnull(sv.data());

			unsigned long n;

			LFL_CallWin32F(WriteConsoleW, h_std, sv.data(),
				static_cast<unsigned long>(sv.length()), &n, limpl({}));
			return size_t(n);
		}

		unique_ptr<WConsole>
			MakeWConsole(unsigned long h)
		{
			unique_ptr<WConsole> p_con;

			TryExpr(p_con.reset(new WConsole(h)))
				CatchIgnore(Win32Exception&)
				return p_con;
		}

	} // inline namespace Windows;

#endif

} // namespace platform_ex;


