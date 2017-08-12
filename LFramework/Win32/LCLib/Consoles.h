/*!	\file Consoles.h
\ingroup Framework
\ingroup Win32
\brief 控制台。
*/

#ifndef FrameWork_Win32_Consoles_h
#define FrameWork_Win32_Consoles_h 1

#include <LFramework/Win32/LCLib/NLS.h>
#include <cstdlib> // for std::system;
#include <LFramework/LCLib/FContainer.h> // for array;
#include <wincon.h>

namespace platform_ex
{
	namespace Windows
	{
		/*!
		\build Windows 控制台对象。
		*/
		class LB_API WConsole
		{
		public:
			/*!
			\build Windows 控制台文本样式。
			*/
			enum Style
			{
				//! brief 暗淡文本，暗淡背景。
				Normal = 0,
				//! brief 明亮文本，暗淡背景。
				Bright = 0x08
			};

		private:
			::HANDLE h_std;
			::WORD saved_attr;

		public:
			::WORD Attributes;

			//! \pre 断言：句柄非空有效。
			//@{
			WConsole(unsigned long h = STD_OUTPUT_HANDLE);
			/*!
			\brief 构造：使用已知控制台句柄。
			*/
			WConsole(::HANDLE);
			//@}
			//! \note 析构：重置属性。
			~WConsole();

			//@{
			//! \throw Win32Exception 操作失败。
			//@{
			DefGetter(const, ::COORD, CursorPosition,
				GetScreenBufferInfo().dwCursorPosition)
				::CONSOLE_SCREEN_BUFFER_INFO
				GetScreenBufferInfo() const;
			//@}
			DefGetter(const lnothrow, ::WORD, SavedAttributes, saved_attr)
				//@}

				void
				SetBackColor(std::uint8_t) lnothrow;
			void
				SetCursorPosition(::COORD);
			void
				SetForeColor(std::uint8_t) lnothrow;
			static PDefH(void, SetSystemColor, )
				ImplExpr(std::system("COLOR"))
				static void
				SetSystemColor(std::uint8_t);

			static PDefH(void, Clear, )
				ImplExpr(std::system("CLS"))

				static ::WORD
				ComposeAttributes(std::uint8_t, std::uint8_t) lnothrow;

			//@{
			void
				CursorUp(size_t);

			static PDefH(array<std::uint8_t LPP_Comma 2>, DecomposeAttributes,
				::WORD val) lnothrow
				ImplRet({ { FetchForeColor(val), FetchBackColor(val) } })

				//! \throw Win32Exception 操作失败。
				//@{
				void
				Erase(wchar_t = L' ');

			static PDefH(std::uint8_t, FetchBackColor, ::WORD val) lnothrow
				ImplRet(std::uint8_t((val >> 4) & 15))

				static PDefH(std::uint8_t, FetchForeColor, ::WORD val) lnothrow
				ImplRet(std::uint8_t(val & 15))

				void
				Fill(::COORD, unsigned long, wchar_t = L' ');

			/*!
			\brief 重置属性。
			*/
			void
				RestoreAttributes();

			void
				Update();
			void
				Update(::WORD);

			void
				UpdateBackColor(std::uint8_t);

			void
				UpdateForeColor(std::uint8_t);
			//@}

			/*!
			\brief 输出字符串。
			\pre 第一参数的数据指针非空。
			\return 输出的字符数。
			*/
			//@{
			size_t
				WriteString(string_view);
			//! \note 第二参数为代码页。
			size_t
				WriteString(string_view, unsigned);
			size_t
				WriteString(wstring_view);
			PDefH(size_t, WriteString, u16string_view sv)
				ImplRet(WriteString({ platform::wcast(sv.data()), sv.length() }))
				//@}
				//@}
		};

		/*!
		\ingroup helper_functions
		\brief 创建控制台对象。
		\return 指向新创建的控制台对象的指针，若失败则为空。
		\note 不抛出 Win32Exception 。
		\relates WConsole
		*/
		LB_API unique_ptr<WConsole>
			MakeWConsole(unsigned long = STD_OUTPUT_HANDLE);

	} // inline namespace Windows;

} // namespace platform_ex;

#endif