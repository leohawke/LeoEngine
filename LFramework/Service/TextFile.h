/*!	\file TextFile.h
\ingroup Service
\brief 平台无关的文本文件抽象。
*/


#ifndef Framework_Service_TextFile_h_
#define Framework_Service_TextFile_h_ 1

#include <LFramework/Service/File.h>

namespace leo
{
	namespace Text
	{
		/*!
		\brief 验证编码。
		\note 第二参数和第三参数指定验证用的缓冲区，第四参数指定最大文本长度，
		第五参数参数指定验证的编码。
		\note 假定调用前流状态正常。
		*/
		//@{
		LB_NONNULL(1, 2) Text::Encoding
			VerifyEncoding(std::FILE*, char*, size_t, size_t, Encoding = CS_Default);
		//! \since build 743
		LB_NONNULL(2) Text::Encoding
			VerifyEncoding(std::streambuf&, char*, size_t, size_t, Encoding = CS_Default);
		LB_NONNULL(2) Text::Encoding
			VerifyEncoding(std::istream&, char*, size_t, size_t, Encoding = CS_Default);
		//@}


		/*!
		\brief Unicode 编码模式标记。

		Unicode Encoding Scheme Signatures BOM（byte-order mark ，字节顺序标记）常量。
		\note 在 Unicode 3.2 前作为零宽无间断空格字符在对应字符集的编码单字节序列。
		\note 适用于未明确字节序或字符集的流。
		*/
		//@{
		lconstexpr const char BOM_UTF_16LE[]{ "\xFF\xFE" };
		lconstexpr const char BOM_UTF_16BE[]{ "\xFE\xFF" };
		lconstexpr const char BOM_UTF_8[]{ "\xEF\xBB\xBF" };
		lconstexpr const char BOM_UTF_32LE[]{ "\xFF\xFE\x00\x00" };
		lconstexpr const char BOM_UTF_32BE[]{ "\x00\x00\xFE\xFF" };
		//@}

		//! \brief 检查第一参数是否具有指定的 BOM 。
		//@{
		/*!
		\pre 间接断言：指针参数非空。
		*/
		//@{
		inline LB_NONNULL(1, 2) PDefH(bool, CheckBOM, const char* buf, const char* str,
			size_t n)
			ImplRet(std::char_traits<char>::compare(Nonnull(buf), str, n) == 0)
			template<size_t _vN>
		inline LB_NONNULL(1) bool
			CheckBOM(const char* buf, const char(&str)[_vN])
		{
			return CheckBOM(buf, str, _vN - 1);
		}
		//@}
		//@{
		//! \pre 间接断言：参数的数据指针非空。
		template<size_t _vN>
		inline bool
			CheckBOM(string_view sv, const char(&str)[_vN])
		{
			return !(sv.length() < _vN - 1) && CheckBOM(sv.data(), str, _vN - 1);
		}
		/*!
		\pre 断言：第二参数表示的字符数不超过最长的 BOM 长度。
		\note 不检查读写位置，直接读取流然后检查读取的内容。
		*/
		//@{
		//! \pre 断言：参数的数据指针非空。
		LF_API bool
			CheckBOM(std::istream&, string_view);
		template<size_t _vN>
		inline bool
			CheckBOM(std::istream& is, const char(&str)[_vN])
		{
			return CheckBOM(is, { str, _vN - 1 });
		}
		//@}
		//@}
		//@}

		//! \brief 探测 BOM 和编码。
		//@{
		/*!
		\pre 参数指定的缓冲区至少具有 4 个字节可读。
		\pre 断言：参数非空。
		\return 检查的编码和 BOM 长度，若失败为 <tt>{CharSet::Null, 0}</tt> 。
		*/
		LF_API LB_NONNULL(1) pair<Encoding, size_t>
			DetectBOM(const char*);
		/*!
		\pre 断言：参数的数据指针非空。
		\return 检查的编码和 BOM 长度，若失败为 <tt>{CharSet::Null, 0}</tt> 。
		*/
		LF_API pair<Encoding, size_t>
			DetectBOM(string_view);
		/*!
		\exception LoggedEvent 流在检查 BOM 时候读取的字符数不是非负数。
		\sa VerifyEncoding

		设置读位置为起始位置。当流大小大于 1 时试验读取前 4 字节并检查 BOM 。
		若没有发现 BOM ，调用 VerifyEncoding 按前 64 字节探测编码。
		读取 BOM 时若遇到流无效，直接返回 <tt>{CharSet::Null, 0}</tt> 。
		*/
		//@{
		LF_API pair<Encoding, size_t>
			DetectBOM(std::streambuf&, size_t, Encoding = CS_Default);
		LF_API pair<Encoding, size_t>
			DetectBOM(std::istream&, size_t, Encoding = CS_Default);
		//@}
		//@}

		/*!
		\brief 写入指定编码的 BOM 。
		\return 写入的 BOM 的长度。
		\note 只写入 UTF-16 、 UTF-8 或 UTF-32 的 BOM 。
		*/
		LF_API size_t
			WriteBOM(std::ostream&, Text::Encoding);

	} // namespace Text;

} // namespace leo;

#endif

