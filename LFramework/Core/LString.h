/*!	\file LString.h
\ingroup LFrameWork/Core
\brief 基础字符串管理。
*/

#ifndef LFramework_Core_LString_h_
#define LFramework_Core_LString_h_ 1

#include <LFramework/Core/LObject.h>
#include <LFramework/Adaptor/LTextBase.h>


namespace leo
{
	namespace Text
	{

		/*!
		\brief Framewrok 标准字符串（使用 UCS-2 作为内部编码）。
		\warning 非虚析构。
		*/
		class String : public u16string
		{
		public:
			/*!
			\brief 无参数构造：默认实现。
			*/
			DefDeCtor(String)
				//@{
				using u16string::u16string;
			//! \brief 构造：使用 YSLib 基本字符串。
			String(const u16string& s)
				: u16string(s)
			{}
			//! \brief 构造：使用 LFramework 基本字符串右值引用。
			String(u16string&& s)
				: u16string(std::move(s))
			{}
			explicit
				String(u16string_view sv)
				: u16string(sv)
			{}
			//@}
			/*!
			\brief 构造：使用指针表示的 NTCTS 和指定编码。
			\pre 间接断言：指针参数非空。
			*/
			template<typename _tChar>
			LB_NONNULL(2)
				String(const _tChar* s, Encoding enc = CS_Default)
				: u16string(MakeUCS2LE<u16string>(s, enc))
			{}
			/*!
			\brief 构造：使用指针表示的字符范围和指定编码。
			\pre 间接断言：指针参数非空。
			*/
			template<typename _tChar>
			LB_NONNULL(2)
				String(const _tChar* s, size_t n, Encoding enc = CS_Default)
				: String(basic_string_view<_tChar>(s, n), enc)
			{}
			template<typename _tChar>
			explicit
				String(basic_string_view<_tChar> sv, Encoding enc = CS_Default)
				: u16string(MakeUCS2LE<u16string>(sv.data(), enc))
			{}
			/*
			\brief 构造：使用指定字符类型的 basic_string 和指定编码。
			*/
			template<typename _tChar, class _tTraits = std::char_traits<_tChar>,
				class _tAlloc = std::allocator<_tChar>>
				String(const basic_string<_tChar, _tTraits, _tAlloc>& s,
					Encoding enc = CS_Default)
				: String(basic_string_view<_tChar>(s), enc)
			{}
			/*!
			\brief 构造：使用字符的初值符列表。
			*/
			template<typename _tChar>
			String(std::initializer_list<_tChar> il)
				: u16string(il.begin(), il.end())
			{}
			/*!
			\brief 复制构造：默认实现。
			*/
			DefDeCopyCtor(String)
				/*!
				\brief 转移构造：默认实现。
				*/
				DefDeMoveCtor(String)
				DefDeDtor(String)

				/*!
				\brief 复制赋值：默认实现。
				*/
				DefDeCopyAssignment(String)
				/*!
				\brief 转移赋值：默认实现。
				*/
				DefDeMoveAssignment(String)

				/*!
				\brief 重复串接。
				*/
				String&
				operator*=(size_t);

			/*!
			\brief 取指定编码的多字节字符串。
			*/
			PDefH(string, GetMBCS, Encoding enc = CS_Default) const
				ImplRet(MakeMBCS<string>(*this, enc))
		};

		/*!
		\relates String
		*/
		inline PDefHOp(String, *, const String& str, size_t n)
			ImplRet(String(str) *= n)

	} // namespace Text;

} // namespace leo;

#endif

