#pragma warning(push)
#pragma warning(disable:4244)

#include "Font.hpp"
#include <Singleton.hpp>
#include <DebugOutput.hpp>
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

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


LEO_DRAW_BEGIN

using std::pair;

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

	/*!
	\see ::FT_GlyphSlot_Oblique ÊµÏÖ¡£
	*/
	::FT_Matrix italic_matrix{ 0x10000L, 0x0366AL, 0x0000L, 0x10000L };
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

FontFamily::FontFamily(FontCache& cache, const FamilyName& name)
	: Cache(cache), family_name(name), mFaces()
{}

void
FontFamily::operator+=(Typeface& face)
{
	mFaces.emplace(face.GetStyleName(), &face);
}

bool
FontFamily::operator-=(Typeface& face) lnothrow
{
	return mFaces.erase(face.GetStyleName()) != 0;
}

Typeface*
FontFamily::GetTypefacePtr(FontStyle fs) const
{
	Typeface* const p(GetTypefacePtr(FetchName(fs)));

	return p ? p : (fs == FontStyle::Regular ? nullptr
		: GetTypefacePtr("Regular"));
}
Typeface*
FontFamily::GetTypefacePtr(const StyleName& style_name) const
{
	const auto i(mFaces.find(style_name));

	return (i == mFaces.cend()) ? nullptr : i->second;
}
Typeface&
FontFamily::GetTypefaceRef(FontStyle fs) const
{
	const auto p(GetTypefacePtr(fs));

	if (LB_UNLIKELY(!p))
		throw logged_event("No matching face found.",record_level::Critical);
	return *p;
}
Typeface&
FontFamily::GetTypefaceRef(const StyleName& style_name) const
{
	const auto p(GetTypefacePtr(style_name));

	if (LB_UNLIKELY(!p))
		throw logged_event("No matching face found.", record_level::Critical);
	return *p;
}

Typeface::SmallBitmapData::SmallBitmapData(::FT_GlyphSlot slot, FontStyle style)
{
	if (slot && slot->format == FT_GLYPH_FORMAT_BITMAP)
	{
		auto& bitmap(slot->bitmap);

		if (bool(style & FontStyle::Bold))
		{
			const auto library(slot->library);
			const auto face(slot->face);
			::FT_Pos xstr(FT_MulFix(face->units_per_EM,
				face->size->metrics.y_scale) / 24 & ~63), ystr(xstr);

			if (xstr == 0)
				xstr = 1 << 6;
			if (::FT_GlyphSlot_Own_Bitmap(slot) == FT_Err_Ok
				&& ::FT_Bitmap_Embolden(library, &bitmap, xstr, ystr)
				== FT_Err_Ok)
			{
				if (slot->advance.x)
					slot->advance.x += xstr;
				if (slot->advance.y)
					slot->advance.y += ystr;
				{
					auto& metrics(slot->metrics);

					lunseq(metrics.width += xstr, metrics.height += ystr,
						metrics.horiAdvance += xstr,
						metrics.vertAdvance += ystr);
				}
				slot->bitmap_top += ::FT_Int(ystr >> 6);
			}
		}

		const ::FT_Pos xadv((slot->advance.x + 32) >> 6),
			yadv((slot->advance.y + 32) >> 6);

#define SBIT_CHECK_CHAR(d) (::FT_Int(::FT_Char(d)) == d)
#define SBIT_CHECK_BYTE(d) (::FT_Int(::FT_Byte(d)) == d)
		if (SBIT_CHECK_BYTE(bitmap.rows) && SBIT_CHECK_BYTE(bitmap.width)
			&& SBIT_CHECK_CHAR(bitmap.pitch)
			&& SBIT_CHECK_CHAR(slot->bitmap_left)
			&& SBIT_CHECK_CHAR(slot->bitmap_top) && SBIT_CHECK_CHAR(xadv)
			&& SBIT_CHECK_CHAR(yadv))
		{
			lunseq(
				width = byte(bitmap.width),
				height = byte(bitmap.rows),
				left = static_cast<signed char>(slot->bitmap_left),
				top = static_cast<signed char>(slot->bitmap_top),
				format = byte(bitmap.pixel_mode),
				max_grays = byte(bitmap.num_grays - 1),
				pitch = static_cast<signed char>(bitmap.pitch),
				xadvance = static_cast<signed char>(xadv),
				yadvance = static_cast<signed char>(yadv),
				buffer = bitmap.buffer
				);
			bitmap.buffer = {};
			// XXX: Moving instead of copying should be safe if the library
			//	memory handlers are not customized.
			// NOTE: Be cautious for DLLs. For documented default behavior, see:
			//	http://www.freetype.org/freetype2/docs/design/design-4.html .
			return;
		}
#undef SBIT_CHECK_CHAR
#undef SBIT_CHECK_BYTE
	}
}
Typeface::SmallBitmapData::SmallBitmapData(SmallBitmapData&& sbit_dat)
	: width(sbit_dat.width), height(sbit_dat.height), left(sbit_dat.left),
	top(sbit_dat.top), format(sbit_dat.format), max_grays(sbit_dat.width),
	pitch(sbit_dat.pitch), xadvance(sbit_dat.xadvance),
	yadvance(sbit_dat.yadvance), buffer(sbit_dat.buffer)
{
	sbit_dat.buffer = {};
}
Typeface::SmallBitmapData::~SmallBitmapData()
{
	// NOTE: See constructor.
	std::free(buffer);
}


Typeface::Typeface(FontCache& cache, const FontPath& path, std::uint32_t i)
// XXX: Conversion to 'long' might be implementation-defined.
	: Path(path), face_index(long(i)), cmap_index(-1), style_name(),
	ref([&, this] {
	if (LB_UNLIKELY(exists(cache.sFaces, this)))
		throw logged_event("Duplicate typeface found.", record_level::Critical);

	::FT_Face face;
	auto error(::FT_New_Face(cache.library, Path.c_str(), face_index,
		&face));

	if (LB_LIKELY(!error))
		if (LB_LIKELY(!(error = ::FT_Select_Charmap(face,
			FT_ENCODING_UNICODE)) && face))
			cmap_index = face->charmap
			? ::FT_Get_Charmap_Index(face->charmap) : 0;
	if (LB_UNLIKELY(error))
		throw logged_event(sfmt("Face loading failed"
			" with face request error: %08x\n.", error), record_level::Critical);

	const FamilyName family_name(face->family_name);
	auto& p_ff(cache.mFamilies[family_name]);

	if (!p_ff)
		p_ff.reset(new FontFamily(cache, family_name));
	return pair<lref<FontFamily>, lref<::FT_FaceRec_>>(*p_ff.get(), *face);
}()), bitmap_cache(2047U), glyph_index_cache()
{
	LAssert(::FT_UInt(cmap_index) < ::FT_UInt(ref.second.get().num_charmaps),
		"Invalid CMap index found.");
	style_name = ref.second.get().style_name;
	ref.first.get() += *this;
}
Typeface::~Typeface()
{
	size_cache.clear();
	glyph_index_cache.clear();
	bitmap_cache.clear();
	ref.first.get() -= *this;

	const auto face(&ref.second.get());

#if LB_Impl_Use_FT_Internal
	YAssert(Deref(face).internal->refcount == 1,
		"Invalid face reference count found.");
	// XXX: Hack for using %ttmtx.c and %sfobjs.c of FreeType 2.4.11.
	if (FT_IS_SFNT(face))
	{
		const auto ttface(reinterpret_cast<::TT_Face>(face));

		// NOTE: See %Typeface::SmallBitmapData::SmallBitmapData.
		// NOTE: %sfnt_done_face in "sfobjs.c" still releases vertical metrics.
		std::free(ttface->horizontal.long_metrics),
			std::free(ttface->horizontal.short_metrics);
	}
#endif
	//::FT_Done_Face(face);
}

bool
Typeface::operator==(const Typeface& rhs) const
{
	return Path == rhs.Path && face_index == rhs.face_index;
}
bool
Typeface::operator<(const Typeface& rhs) const
{
	return Path < rhs.Path
		|| (Path == rhs.Path && face_index < rhs.face_index);
}

Typeface::SmallBitmapData&
Typeface::LookupBitmap(const Typeface::BitmapKey& key) const
{
	return cache_lookup(bitmap_cache, key, [&] {
		LookupSize(key.Size).Activate();
		::FT_Set_Transform(&ref.second.get(),
			bool(key.Style & FontStyle::Italic) ? &italic_matrix : nullptr, {});

		return SmallBitmapData(::FT_Load_Glyph(&ref.second.get(),
			key.GlyphIndex, std::int32_t(key.Flags | FT_LOAD_RENDER)) == 0
			? ref.second.get().glyph : nullptr, key.Style);
	});
}

::FT_UInt
Typeface::LookupGlyphIndex(ucs4_t c) const
{
	auto i(glyph_index_cache.find(c));

	if (i == glyph_index_cache.end())
	{
		if (cmap_index > 0)
			::FT_Set_Charmap(&ref.second.get(),
				ref.second.get().charmaps[cmap_index]);

		const auto pr(glyph_index_cache.emplace(c, ::FT_Get_Char_Index(
			&ref.second.get(), ::FT_ULong(c))));

		if (LB_UNLIKELY(!pr.second))
			throw logged_event("Glyph index cache insertion failed.",record_level::Alert);
		i = pr.first;
	}
	return i->second;
}

NativeFontSize&
Typeface::LookupSize(FontSize s) const
{
	auto i(size_cache.find(s));

	if (i == size_cache.end())
	{
		const auto pr(size_cache.emplace(s, NativeFontSize(ref.second, s)));

		if (LB_UNLIKELY(!pr.second))
			throw logged_event("Bitmap cache insertion failed.",record_level::Alert);
		i = pr.first;
	}
	return i->second;
}


const Typeface&
FetchDefaultTypeface()
{
	const Typeface* const pDefaultTypeface(
		FetchDefaultFontCache().GetDefaultTypefacePtr());

	if (LB_UNLIKELY(!pDefaultTypeface))
		throw logged_event("Null default font face pointer found.", record_level::Critical);
	return *pDefaultTypeface;
}


FontCache::FontCache(size_t /*cache_size*/)
	: pDefaultFace()
{
	::FT_Error error;

	if (LB_LIKELY((error = ::FT_Init_FreeType(&library)) == 0))
		LTraceDe(Informative, "FreeType library instance initialized.\n");
	else
	{
		// TODO: Format without allocating memory.
		throw logged_event(
			sfmt("Font init failed: %08x\n;", error).c_str(),record_level::Alert);
	}
}
FontCache::~FontCache()
{
	ClearContainers();
	::FT_Done_FreeType(library);
}

const FontFamily*
FontCache::GetFontFamilyPtr(const FamilyName& family_name) const
{
	const auto i(mFamilies.find(family_name));

	return (i == mFamilies.cend()) ? nullptr : i->second.get();
}

const Typeface*
FontCache::GetDefaultTypefacePtr() const
{
	// NOTE: Guaranteed to be non-null for default typeface in default cache.
	return pDefaultFace ? pDefaultFace
		: FetchDefaultFontCache().GetDefaultTypefacePtr();
}
const Typeface*
FontCache::GetTypefacePtr(const FamilyName& family_name,
	const StyleName& style_name) const
{
	const FontFamily* f(GetFontFamilyPtr(family_name));

	if (LB_UNLIKELY(!f))
		return{};
	return f->GetTypefacePtr(style_name);
}

void
FontCache::operator+=(std::unique_ptr<FontFamily> p_family)
{
	mFamilies.emplace(p_family->GetFamilyName(), std::move(p_family));
}
void
FontCache::operator+=(Typeface& face)
{
	sFaces.insert(&face);
}

bool
FontCache::operator-=(FontFamily& family) lnothrow
{
	return mFamilies.erase(family.GetFamilyName()) != 0;
}
bool
FontCache::operator-=(Typeface& face) lnothrow
{
	return &face != pDefaultFace && sFaces.erase(&face) != 0;
}

void
FontCache::ClearContainers() lnothrow
{
	std::for_each(sFaces.cbegin(), sFaces.cend(), delete_obj());
	sFaces.clear();
	mFamilies.clear();
}

size_t
FontCache::LoadTypefaces(const FontPath& path)
{
	//if (ufexists(path.c_str()) && !IO::VerifyDirectory(path))
	{
		::FT_Face face(nullptr);

		if (::FT_New_Face(library, path.c_str(), -1, &face) != 0)
			return 0;

		const auto face_num(face->num_faces);

		::FT_Done_Face(face);
		LTraceDe(Informative, "Loaded faces num '%ld' from path '%s'.\n",
			face_num, path.c_str());
		if (face_num < 0)
			return 0;
		for (long i(0); i < face_num; ++i)
			// XXX: Conversion to 'long' might be implementation-defined.
			TryExpr(*this += *(new Typeface(*this, path, std::uint32_t(i))))
			CatchExpr(..., LTraceDe(Warning, "Failed loading face of path"
				" '%s', index '%ld'.", path.c_str(), i))
			return size_t(face_num);
	}
	return 0;
}

void
FontCache::InitializeDefaultTypeface()
{
	if (LB_LIKELY(!(pDefaultFace || sFaces.empty())))
		pDefaultFace = *sFaces.cbegin();
}


Font::Font(const FontFamily& family,FontSize s,
	FontStyle fs)
	:typeface(family.GetTypefaceRef(fs)), font_size(s), style(fs)
{}

std::int8_t
Font::GetAdvance(ucs4_t c, CharBitmap sbit) const
{
	if (!sbit)
		sbit = GetGlyph(c, FT_LOAD_DEFAULT);
	return LB_LIKELY(sbit) ? sbit.GetXAdvance() : 0;
}
std::int8_t
Font::GetAscender() const
{
	return GetInternalInfo().ascender >> 6;
}
std::int8_t
Font::GetDescender() const
{
	return GetInternalInfo().descender >> 6;
}
CharBitmap
Font::GetGlyph(ucs4_t c, unsigned flags) const
{
	static_assert((FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) == 4L,
		"Invalid default argument found.");
	const auto& face(GetTypeface());

	return &face.LookupBitmap(Typeface::BitmapKey{ flags,
		face.LookupGlyphIndex(c), font_size, style });
}
FontSize
Font::GetHeight() const lnothrow
{
	return GetInternalInfo().height >> 6;
}
::FT_Size_Metrics
Font::GetInternalInfo() const
{
	return GetTypeface().LookupSize(GetSize()).GetSizeRec().metrics;
}

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

std::unique_ptr<FontCache> p_font_cache;

class FontCacheDelegate :leo::Singleton<FontCacheDelegate>
{
public:
	FontCacheDelegate()
	{};

	~FontCacheDelegate()
	{
		p_font_cache.reset();
	}
};

void
InitializeSystemFontCache(FontCache& fc, const std::string& fong_file,
	const std::string& font_dir)
{
	try {
		fc.LoadTypefaces(fong_file);
		if(!font_dir.empty())
		try
		{
			for (auto &p : fs::directory_iterator(font_dir))
			{
				FontPath path(font_dir + p.path().filename().string());

				if (path != fong_file)
					fc.LoadTypefaces(path);
			}
		}
		catch(...)
		{ }
		fc.InitializeDefaultTypeface();
	}
	catch(...)
	{ }
}

FontCache&
FetchDefaultFontCache()
{
	if (LB_UNLIKELY(!p_font_cache))
	{
		p_font_cache.reset(new FontCache());

		InitializeSystemFontCache(*p_font_cache,
			"./Fonts/SourceHanSans-Regular.otf",
			"./Fonts/");
	}
	return *p_font_cache;
}

LEO_DRAW_END

#pragma warning(pop)

