//基于freetype的字体渲染实现
#ifndef UI_Font_h
#define UI_Font_h

#include "UI.h"
#include <cstddef>
#include <set>
#include <leoint.hpp>
#include <id.hpp>
#include <cache.hpp>
#include <ref.hpp>
#include <exception.hpp>
#include <container.hpp>
#include <string.hpp>
#include <functional.hpp>

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

class Font;
class FontCache;
class FontFamily;
class Typeface;

using FontSize = std::uint8_t;
using FontPath = std::string;
using FamilyName = std::string;
using StyleName = std::string;

enum class FontStyle : std::uint8_t
{
	Regular = 0, //!< 常规字体。
	Bold = 1, //!< 粗体。
	Italic = 2, //!< 斜体。
	Underline = 4, //!< 下划线。
	Strikeout = 8 //!< 删除线。
};

DefBitmaskEnum(FontStyle)

lconstfn PDefH(const char*, FetchName, FontStyle style) lnothrow
ImplRet(style == FontStyle::Bold ? "Bold" : (style == FontStyle::Italic
	? "Italic" : (style == FontStyle::Underline ? "Underline" : (style
		== FontStyle::Strikeout ? "Strikeout" : "Regular"))))

/*!
\brief 字体异常。
*/
class LB_API FontException : public logged_event
{
public:
	//! \note 和 \c ::FT_Error 一致。
	using FontError = int;

private:
	FontError err;

public:
	FontException(FontError e, const std::string& msg = {})
		: logged_event(msg),
		err(e)
	{}
	DefDeCopyCtor(FontException)
	/*!
	\brief 虚析构：类定义外默认实现。
	*/
	~FontException() override;

	DefGetter(const lnothrow, FontError, ErrorCode, err)
};

/*!
\brief 本机字体大小。
*/
class LB_API NativeFontSize final : private noncopyable
{
private:
	::FT_Size size;

public:
	NativeFontSize(::FT_FaceRec&, FontSize);
	NativeFontSize(NativeFontSize&&) lnothrow;
	~NativeFontSize();

	::FT_SizeRec&
		GetSizeRec() const;

	/*!
	\brief 激活当前大小。
	\note 替代 \c ::FT_Activate_Size 。
	*/
	void
		Activate() const;
};

/*!
\brief 字型家族 (Typeface Family) 标识。
*/
class LB_API FontFamily final : private noncopyable
{
public:
	using FaceMap = std::map<const StyleName, Typeface*>; //!< 字型组索引类型。

	FontCache& Cache;

private:
	FamilyName family_name;

protected:
	FaceMap mFaces; //!< 字型组索引类型。

public:
	/*!
	\brief 使用字体缓存引用和名称构造字型家族。
	*/
	FontFamily(FontCache&, const FamilyName&);

	/*!
	\brief 向字型组和字型组索引添加字型对象。
	*/
	void
		operator+=(Typeface&);
	/*!
	\brief 从字型组和字型组索引中移除指定字型对象。
	\since build 572
	*/
	bool
		operator-=(Typeface&) lnothrow;

	DefGetter(const lnothrow, const FamilyName&, FamilyName, family_name)
		/*!
		\brief 取指定样式的字型指针。
		\note 若非 Regular 样式失败则尝试取 Regular 样式的字型指针。
		*/
		Typeface*
		GetTypefacePtr(FontStyle) const;
	/*!
	\brief 取指定样式名称的字型指针。
	*/
	Typeface*
		GetTypefacePtr(const StyleName&) const;
	Typeface&
		GetTypefaceRef(FontStyle) const;
	Typeface&
		GetTypefaceRef(const StyleName&) const;
};


/*!
\brief 字型标识。
*/
class LB_API Typeface final : private noncopyable, private nonmovable
{
	friend class Font;
	friend class CharBitmap;

public:
	const FontPath Path;

private:
	//@{
	struct BitmapKey
	{
		unsigned Flags;
		unsigned GlyphIndex;
		FontSize Size;
		FontStyle Style;

		PDefHOp(bool, == , const BitmapKey& key) const lnothrow
			ImplRet(Flags == key.Flags && GlyphIndex == key.GlyphIndex
				&& Size == key.Size && Style == key.Style)
	};

	struct BitmapKeyHash
	{
		PDefHOp(size_t, (), const BitmapKey& key) const lnothrow
			ImplRet(hash_combine_seq(size_t(key.Style), key.Size,
				key.GlyphIndex, key.Flags))
	};

	class SmallBitmapData
	{
		friend class CharBitmap;

	private:
		/*!
		\sa ::FTC_SBitRec_
		*/
		//@{
		byte width = 255, height = 0;
		signed char left = 0, top = 0;
		byte format = 0, max_grays = 0;
		short pitch = 0;
		signed char xadvance = 0, yadvance = 0;
		byte* buffer = {};
		//@}

	public:
		//! \since build 421
		SmallBitmapData(::FT_GlyphSlot, FontStyle);
		SmallBitmapData(SmallBitmapData&&);
		~SmallBitmapData();
	};
	//@}

	long face_index;
	int cmap_index;
	StyleName style_name;
	std::pair<lref<FontFamily>, lref<::FT_FaceRec_>> ref;
	mutable used_list_cache<BitmapKey, SmallBitmapData, BitmapKeyHash>
		bitmap_cache;
	mutable std::unordered_map<ucs4_t, unsigned> glyph_index_cache;
	mutable std::unordered_map<FontSize, NativeFontSize> size_cache;

public:
	/*!
	\brief 使用字体缓存引用在指定字体文件路径读取指定索引的字型并构造对象。
	\post 断言： \c cmap_index 在 face 接受的范围内。
	*/
	Typeface(FontCache&, const FontPath&, std::uint32_t = 0);
	~Typeface();

	/*!
	\brief 比较：相等关系。
	*/
	bool
		operator==(const Typeface&) const;
	/*!
	\brief 比较：严格递增偏序关系。
	*/
	bool
		operator<(const Typeface&) const;

	DefGetterMem(const lnothrow, FamilyName, FamilyName, GetFontFamily())
		/*!
		\brief 取字型家族。
		*/
		DefGetter(const lnothrow, const FontFamily&, FontFamily, ref.first)
		DefGetter(const lnothrow, const StyleName&, StyleName, style_name)
		/*!
		\brief 取字符映射索引号。
		*/
		DefGetter(const lnothrow, int, CMapIndex, cmap_index)

private:
	//@{
	SmallBitmapData&
		LookupBitmap(const BitmapKey&) const;

	unsigned
		LookupGlyphIndex(ucs4_t) const;

	NativeFontSize&
		LookupSize(FontSize) const;

public:
	PDefH(void, ClearBitmapCache,)
		ImplExpr(bitmap_cache.clear())

		PDefH(void, ClearGlyphIndexCache,)
		ImplExpr(glyph_index_cache.clear())
		//@}

		PDefH(void, ClearSizeCache,)
		ImplExpr(size_cache.clear())
};


/*!
\brief 取默认字型引用。
\throw logged_event 严重：异常事件。
*/
LB_API const Typeface&
FetchDefaultTypeface();


/*!
\brief 字符位图。
\warning 若为空时调用成员函数时行为未定义。
*/
class LB_API CharBitmap final
{
public:
	//! \note 和 \c ::FT_Byte* 一致。
	using BufferType = byte*;
	/*!
	\note 值兼容于 \c ::FT_Pixel_Mode 。
	*/
	enum FormatType
	{
		None = 0,
		Mono,
		Gray,
		Gray2,
		Gray4,
		LCD,
		LCD_V
	};
	//! \brief 本机类型对象：指针类型。
	using NativeType = Typeface::SmallBitmapData*;
	/*!
	\note 和 \c ::FT_Short 一致。
	*/
	using PitchType = short;
	//! \note 和 \c ::FT_Byte 一致。
	using ScaleType = byte;
	//! \note 和 \c ::FT_Char 一致。
	using SignedScaleType = signed char;

private:
	NativeType bitmap = {};

public:
	//@{
	//! \brief 构造：空位图。
	DefDeCtor(CharBitmap)
		//! \brief 构造：使用本机类型对象。
		lconstfn
		CharBitmap(NativeType b)
		: bitmap(b)
	{}
	//@}

	lconstfn DefCvt(const lnothrow, NativeType, bitmap)

		/*!
		\pre 间接断言：位图本机类型对象非空。
		*/
		//@{
		lconstfn DefGetter(const lnothrowv, BufferType, Buffer,
			Deref(bitmap).buffer)
		lconstfn DefGetter(const lnothrowv, FormatType, Format,
			FormatType(Deref(bitmap).format))
		lconstfn DefGetter(const lnothrowv, ScaleType, GrayLevel,
			Deref(bitmap).max_grays)
		lconstfn DefGetter(const lnothrowv, ScaleType, Height, Deref(bitmap).height)
		lconstfn DefGetter(const lnothrowv, SignedScaleType, Left,
			Deref(bitmap).left)
		lconstfn DefGetter(const lnothrowv, PitchType, Pitch, Deref(bitmap).pitch)
		lconstfn DefGetter(const lnothrowv, SignedScaleType, Top, Deref(bitmap).top)
		lconstfn DefGetter(const lnothrowv, ScaleType, Width, Deref(bitmap).width)
		lconstfn DefGetter(const lnothrowv, SignedScaleType, XAdvance,
			Deref(bitmap).xadvance)
		lconstfn DefGetter(const lnothrowv, SignedScaleType, YAdvance,
			Deref(bitmap).yadvance)
		//@}
};


/*!
\brief 字体缓存。
*/
class LB_API FontCache final : private noncopyable,
	private OwnershipTag<Typeface>, private OwnershipTag<FontFamily>
{
	friend class Typeface;
/*!
\brief 友元类：访问 scaler 等对象。
*/
friend class Font;

public:
	using FaceSet = std::set<Typeface*,deref_comp<const Typeface>>; \
		//!< 字型组类型。
		//! \brief 字型家族组索引类型。
		using FamilyMap = std::unordered_map<FamilyName, std::unique_ptr<FontFamily>>;

	/*!
	\brief 字形缓冲区大小。
	\note 单位为字节。
	*/
	static lconstexpr const std::size_t DefaultGlyphCacheSize = 128U << 10;

private:
	::FT_Library library; //!< 库实例。

protected:
	FaceSet sFaces; //!< 字型组。
	FamilyMap mFamilies; //!< 字型家族组索引。

	Typeface* pDefaultFace; //!< 默认字型指针。

public:
	/*!
	\brief 构造：分配指定大小的字形缓存空间。
	\note 当前暂时忽略参数。
	*/
	explicit
		FontCache(std::size_t = DefaultGlyphCacheSize);
	/*!
	\brief 析构：释放空间。
	*/
	~FontCache();

public:
	/*!
	\brief 取字型组。
	*/
	DefGetter(const lnothrow, const FaceSet&, Faces, sFaces)
		DefGetter(const lnothrow, const FamilyMap&, FamilyIndices, mFamilies) \
		//!< 取字型家族组索引。
		//	Font*
		//	GetFontPtr() const;
		/*!
		\brief 取指定名称的字型家族指针。
		*/
		const FontFamily*
		GetFontFamilyPtr(const FamilyName&) const;
	/*!
	\brief 取默认字型指针。
	\throw logged_event 记录异常事件。
	*/
	const Typeface*
		GetDefaultTypefacePtr() const;
	//	Typeface*
	//	GetTypefacePtr(u16) const; //!< 取字型组储存的指定索引的字型指针。
	/*!
	\brief 取指定名称的字型指针。
	*/
	const Typeface*
		GetTypefacePtr(const FamilyName&, const StyleName&) const;

private:
	/*!
	\brief 向字型家族组添加字型家族。
	*/
	void
		operator+=(std::unique_ptr<FontFamily>);
	/*!
	\brief 向字型组添加字型对象。
	*/
	void
		operator+=(Typeface&);

	/*!
	\brief 从字型家族组中移除指定字型对象。
	*/
	bool
		operator-=(FontFamily&) lnothrow;
	/*!
	\brief 从字型组中移除指定字型对象。
	*/
	bool
		operator-=(Typeface&) lnothrow;

	/*!
	\brief 清除容器。
	*/
	void
		ClearContainers() lnothrow;

public:
	/*!
	\brief 从字体文件组中载入字型信息。
	\return 成功载入的字型数。
	*/
	std::size_t
		LoadTypefaces(const FontPath&);

public:
	/*!
	\brief 初始化默认字型。
	*/
	void
		InitializeDefaultTypeface();
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
	lref<Typeface> typeface;

	FontSize font_size;
	/*!
	\brief 字体样式。
	*/
	FontStyle style;

public:
	Font()
		:Font(FetchDefaultTypeface().GetFontFamily())
	{}

	Font(const FontFamily&, FontSize = DefaultSize,
		FontStyle = FontStyle::Regular);

	DefPred(const lnothrow, Bold, bool(style & FontStyle::Bold))
	DefPred(const lnothrow, Italic, bool(style & FontStyle::Italic))
	DefPred(const lnothrow, Underline, bool(style & FontStyle::Underline))
	DefPred(const lnothrow, Strikeout, bool(style & FontStyle::Strikeout))

		/*!
		\brief 取跨距。
		*/
		std::int8_t
		GetAdvance(ucs4_t, CharBitmap = {}) const;

	/*!
	\brief 取升部。
	*/
	std::int8_t
		GetAscender() const;
	DefGetter(const lnothrow, FontCache&, Cache, GetFontFamily().Cache)
	/*!
	\brief 取降部。
	*/
	std::int8_t
		GetDescender() const;

	DefGetterMem(const lnothrow, const FamilyName&, FamilyName,
		GetFontFamily())
		DefGetterMem(const lnothrow, const FontFamily&, FontFamily, GetTypeface())
		DefGetter(const lnothrow, FontSize, Size, font_size)
		DefGetter(const lnothrow, FontStyle, Style, style)

		
		/*!
		\brief 取当前字型和大小渲染的指定字符的字形。
		\param c 指定需要被渲染的字符。
		\param flags FreeType 渲染标识。
		\note 默认参数为 FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL 。
		\warning 返回的位图在下一次调用 FontCache 方法或底层 FreeType 缓存时不保证有效。
		\warning flags 可能被移除，应仅用于内部实现。
		*/
		CharBitmap
		GetGlyph(ucs4_t c, unsigned flags = 4UL) const;
	/*!
	\brief 取字体对应的字符高度。
	*/
	FontSize
		GetHeight() const lnothrow;
	DefGetter(const lnothrow, StyleName, StyleName, FetchName(style))
private:
	/*!
	\brief 取内部信息。
	*/
	::FT_Size_Metrics
		GetInternalInfo() const;

public:
	/*!
	\brief 取字型引用。
	\since build 280
	*/
	DefGetter(const lnothrow, Typeface&, Typeface, typeface)

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

/*!
\brief 取默认字体缓存。
\exception FatalError 字体缓存初始化失败。
\todo Initialization.h
*/
LB_API Drawing::FontCache&
FetchDefaultFontCache();


LEO_DRAW_END

#endif
