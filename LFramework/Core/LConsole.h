/*!	\file LConsole.h
\ingroup LBase
\brief ����̨��
*/

#ifndef FrameWork_LConsole_h
#define FrameWork_LConsole_h 1

namespace leo
{
	namespace Consoles
	{
		/*!
		\brief ����̨��ɫö�١�
		*/
		enum Color
		{
			Black = 0,
			DarkBlue,
			DarkGreen,
			DarkCyan,
			DarkRed,
			DarkMagenta,
			DarkYellow,
			Gray,
			DarkGray,
			Blue,
			Green,
			Cyan,
			Red,
			Magenta,
			Yellow,
			White
		};

		/*!
		\brief ����̨��ɫ��
		\note ˳��� Consoles::Color ��Ӧ��
		*/
		/*lconstexpr const ColorSpace::ColorSet ConsoleColors[]{ ColorSpace::Black,
			ColorSpace::Navy, ColorSpace::Green, ColorSpace::Teal, ColorSpace::Maroon,
			ColorSpace::Purple, ColorSpace::Olive, ColorSpace::Silver, ColorSpace::Gray,
			ColorSpace::Blue, ColorSpace::Lime, ColorSpace::Aqua, ColorSpace::Red,
			ColorSpace::Yellow, ColorSpace::Fuchsia, ColorSpace::White };*/
	}
}
#endif