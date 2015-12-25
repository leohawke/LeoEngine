//基于freetype的字体渲染实现
#ifndef UI_Font_h
#define UI_Font_h

#include "UI.h"
#include <leoint.hpp>
#include <id.hpp>
#include <cache.hpp>

struct FT_SizeRec_;
using FT_Size = ::FT_SizeRec_*;
using FT_SizeRec = ::FT_SizeRec_;
struct FT_FaceRec_;
using FT_FaceRec = ::FT_FaceRec_;
struct FT_GlyphSlotRec_;
using FT_GlyphSlot = ::FT_GlyphSlotRec_*;
struct FT_LibraryRec_;
using FT_Library = ::FT_LibraryRec_*;
struct FT_Size_Metrics_;
using FT_Size_Metrics = FT_Size_Metrics_;

LEO_DRAW_BEGIN

using FontSize = std::uint8_t;

enum class FontStyle : std::uint8_t
{
	Regular = 0, //!< 常规字体。
};

namespace {

	class LB_API FontFamily final : private noncopyable{
	} nullFontFamily;

	class LB_API FontCache final : private noncopyable{
	} nullFontCache;
}
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
	Font()
		:Font(DefaultSize, FontStyle::Regular)
	{}

	Font(FontSize,
			FontStyle = FontStyle::Regular);

	DefGetter(const lnothrow, FontSize, Size, font_size)
	DefGetter(const lnothrow, FontStyle, Style, style)

	DefGetter(const lnothrow, FontCache&, Cache, nullFontCache)
	DefGetter(const lnothrow, const FontFamily&, FontFamily, nullFontFamily)

		/*!
		\brief 取跨距。
		*/
		std::int8_t
		GetAdvance(char32_t) const;
	/*!
	\brief 取升部。
	*/
	std::int8_t
		GetAscender() const;
	/*!
	\brief 取降部。
	*/
	std::int8_t
		GetDescender() const;


	/*!
	\brief 取字体对应的字符高度。
	*/
	FontSize
		GetHeight() const lnothrow;
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
