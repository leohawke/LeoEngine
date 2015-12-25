#include "Font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_SIZES_H
#include FT_BITMAP_H
//#include FT_GLYPH_H
//#include FT_OUTLINE_H
//#include FT_SYNTHESIS_H
#if defined(FT_CONFIG_OPTION_OLD_INTERNALS) \
	&& (FREETYPE_MAJOR * 10000 + FREETYPE_MINOR * 100 + FREETYPE_PATCH >= 20500)
#	define LB_Impl_Use_FT_Internal 1
#	include <internal/internal.h> // for FreeType internal macros;
#	include FT_INTERNAL_TRUETYPE_TYPES_H // for TT_Face, TT_FaceRec_;
#endif

LEO_DRAW_BEGIN

ImplDeDtor(FontException)

namespace
{
	::FT_Error
		N_SetPixelSizes(::FT_FaceRec& face, ::FT_UInt s) lnothrow
	{
		::FT_Size_RequestRec req{ FT_SIZE_REQUEST_TYPE_NOMINAL, ::FT_Long(s << 6),
			::FT_Long(s << 6), 0, 0 };

		return ::FT_Request_Size(&face, &req);
	}
}


NativeFontSize::NativeFontSize(::FT_FaceRec& face, FontSize s)
	: size()
{
	if (const auto err = ::FT_New_Size(&face, &size))
		throw FontException(err, "Native font size creation failed.");
	Activate();
	if (const auto err = N_SetPixelSizes(face, s))
		throw FontException(err, "Native font setting size failed.");
}
NativeFontSize::NativeFontSize(NativeFontSize&& ns) lnothrow
	: size(ns.size)
{
	ns.size = {};
}
NativeFontSize::~NativeFontSize()
{
	::FT_Done_Size(size);
}

::FT_SizeRec&
NativeFontSize::GetSizeRec() const
{
	if (LB_UNLIKELY(!size))
		throw logged_event("Invalid native size found.",record_level::Critical);
	return *size;
}

void
NativeFontSize::Activate() const
{
	Deref(Deref(size).face).size = size;
}


Font::Font(const FontFamily& family,FontSize s,
	FontStyle fs)
	:typeface(family.GetTypefaceRef(fs)), font_size(s), style(fs)
{}

void
Font::SetSize(FontSize s)
{
	if (LB_LIKELY(s >= MinimalSize && s <= MaximalSize))
		font_size = s;
}
bool
Font::SetStyle(FontStyle fs)
{
	style = fs;
	return true;
}

LEO_DRAW_END