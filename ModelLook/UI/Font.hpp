#ifndef UI_Font_h
#define UI_Font_h

#include "UI.h"
#include <leoint.hpp>

LEO_DRAW_BEGIN

using FontSize = std::uint8_t;

enum class FontStyle : std::uint8_t
{
	Regular = 0, //!< 常规字体。
};

/*!
\brief 字体：字模，包含样式和大小。
\todo 字型
*/
class LB_API Font final{
public:
	static lconstexpr const FontSize DefaultSize = 12,
		MinimalSize = 4, MaximalSize = 96;

private:
private:
	FontSize font_size;
	/*!
	\brief 字体样式。
	*/
	FontStyle style;
public:
	explicit
	Font(FontSize = DefaultSize,
			FontStyle = FontStyle::Regular);

	DefGetter(const lnothrow, FontSize, Size, font_size)
	DefGetter(const lnothrow, FontStyle, Style, style)
public:
	/*!
	\brief 设置字体大小。
	*/
	void
		SetSize(FontSize = DefaultSize);
	/*!
	\brief 设置样式。
	\note 仅当存在字型时设置样式。
	*/
	bool
		SetStyle(FontStyle);
};

LEO_DRAW_END

#endif
