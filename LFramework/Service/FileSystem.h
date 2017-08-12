/*!	\file FileSystem.h
\ingroup Service
\brief 平台中立的文件系统抽象。
*/


#ifndef Framework_Service_FileSystem_h_
#define Framework_Service_FileSystem_h_ 1

#include <LBase/lmacro.h>
#include <LFramework/Core/LString.h>
#include <LBase/path.hpp> // for leo::path;

namespace leo
{
	namespace IO
	{
		/*!
		\brief 默认目录路径长度。
		\note 适合一般情形分配空间大小。
		*/
		lconstexpr const size_t MaxPathLength(limpl(1 << 10));


		/*!
		\brief 判断路径表示绝对路径。
		*/
		//@{
		inline PDefH(bool, IsAbsolute, const string& path)
			ImplRet(IsAbsolute(path.c_str()))
			inline PDefH(bool, IsAbsolute, const String& path)
			ImplRet(IsAbsolute(path.GetMBCS()))
			//@}

			/*!
			\brief 判断路径表示相对路径（包括空路径）。
			\note 使用 ADL IsAbsolute 。
			*/
			template<typename _type>
		inline bool
			IsRelative(const _type& arg)
		{
			return !IsAbsolute(arg);
		}


		/*!
		\brief 文件路径特征。
		*/
		struct PathTraits
		{
			using value_type = leo::String;

			template<typename _tChar>
			static lconstfn bool
				IsDelimiter(_tChar c) lnothrow
			{
				return IO::IsSeparator(c);
			}

			/*!
			\brief 调整绝对路径的第一组件。
			\note 使用 ADL cbegin 。
			\todo 支持更多根路径模式（如 Windows UNC ）。
			*/
			template<class _tPath, class _tString>
			static void
				AdjustForRoot(_tPath& pth, const _tString& sv)
			{
				using leo::cbegin;

				if (!pth.empty() && IsRelative(pth.front()) && IsAbsolute(sv.data()))
					pth.insert(cbegin(pth), { {} });
			}

			template<class _tString>
			static lconstfn bool
				is_parent(const _tString& str) lnothrow
			{
				using value_type = decltype(str[0]);

				return leo::string_length(str) == 2 && str[0] == value_type('.')
					&& str[1] == value_type('.');
			}

			template<class _tString>
			static lconstfn auto
				is_root(const _tString& str) lnothrow
				-> decltype(leo::string_length(str) == FetchRootNameLength(str))
			{
				return leo::string_length(str) == FetchRootNameLength(str);
			}
			template<typename _tChar, class _tTraits, class _tAlloc>
			static bool
				is_root(const basic_string<_tChar, _tTraits, _tAlloc>& str) lnothrow
			{
				return str.length() == FetchRootNameLength(str.c_str());
			}

			template<class _tString>
			static lconstfn bool
				is_self(const _tString& str) lnothrow
			{
				return
					leo::string_length(str) == 1 && str[0] == decltype(str[0])('.');
			}
		};


		/*!
		\brief 目录路径结束正规化：保证以一个分隔符结束。
		\note 兼容根目录。
		\since build 540
		*/
		template<class _tString>
		inline leo::decay_t<_tString>
			NormalizeDirectorlpathTail(_tString&& str, typename
				leo::string_traits<_tString>::const_pointer tail = &leo::to_array<
				typename leo::string_traits<_tString>::value_type>("/\\")[0])
		{
			using value_type = typename leo::string_traits<_tString>::value_type;

			TrimTrailingSeperator(lforward(str), tail);
			LAssert(str.empty() || !IsSeparator(str.back()), "Invalid path converted.");
			str += FetchSeparator<value_type>();
			return lforward(str);
		}


		/*!
		\brief 解析字符串为路径。
		\note 忽略空路径组件。
		\note 使用 ADL cbegin 和 cend 。
		*/
		//@{
		template<class _tPath, typename _func,
			class _tString = typename _tPath::value_type,
			class _tTraits = typename _tPath::traits_type>
			_tPath
			ParsePath(const _tString& sv, _func f, _tPath res = {})
		{
			using leo::cbegin;
			using leo::cend;

			leo::split(cbegin(sv), cend(sv), [](decltype(*cbegin(sv)) c) {
				return _tTraits::IsDelimiter(c);
			}, [&](decltype(cbegin(sv)) b, decltype(cend(sv)) e) {
				if (b != e)
				{
					f(res, b, e);
					if (res.size() == 1)
						_tTraits::AdjustForRoot(res,
							typename _tPath::value_type(cbegin(sv), e));
				}
			});
			return res;
		}
		template<class _tPath, class _tString = typename _tPath::value_type,
			class _tTraits = typename _tPath::traits_type>
			_tPath
			ParsePath(const _tString& sv, _tPath res = {})
		{
			using leo::cbegin;
			using leo::cend;

			return IO::ParsePath<_tPath>(sv,
				[](_tPath& pth, decltype(cbegin(sv)) b, decltype(cend(sv)) e) {
				pth.push_back(typename _tPath::value_type(b, e));
			}, std::move(res));
		}
		//@}


		using lpath = leo::lpath<vector<String>, PathTraits>;


		/*!
		\brief 路径。
		\warning 非虚析构。
		*/
		class LF_API Path : private lpath, private leo::totally_ordered<Path>,
			private leo::dividable<Path, String>, private leo::dividable<Path>
		{
		public:
			using lpath::iterator;
			using lpath::const_iterator;
			using lpath::value_type;
			using lpath::traits_type;

		public:
			/*!
			\brief 无参数构造：默认实现。
			*/
			DefDeCtor(Path)
				explicit
				Path(lpath pth) lnothrow
				: lpath(std::move(pth))
			{}
			explicit
				Path(const u16string& str) lnothrow
				: Path(ParsePath<lpath>(str))
			{}
			template<typename _type, limpl(typename = leo::enable_if_t<
				!std::is_constructible<const u16string&, _type&&>::value>,
				typename = leo::exclude_self_t<Path, _type>)>
				explicit
				Path(_type&& arg)
				: lpath(ParsePath<lpath>(String(lforward(arg))))
			{}
			template<typename _type>
			explicit
				Path(_type&& arg, Text::Encoding enc)
				: lpath(ParsePath<lpath>(String(lforward(arg), enc)))
			{}
			template<typename _tIn>
			Path(_tIn first, _tIn last)
				: lpath(first, last)
			{}
			template<typename _tElem>
			Path(std::initializer_list<_tElem> il)
				: lpath(il)
			{}
			/*!
			\brief 复制构造：默认实现。
			*/
			DefDeCopyCtor(Path)
				/*!
				\brief 转移构造：默认实现。
				*/
				DefDeMoveCtor(Path)
				DefDeDtor(Path)

				/*!
				\brief 复制赋值：默认实现。
				*/
				DefDeCopyAssignment(Path)
				/*!
				\brief 转移赋值：默认实现。
				*/
				DefDeMoveAssignment(Path)

				//! \brief 追加路径。
				//@{
				PDefHOp(Path&, /=, const String& fname)
				ImplRet(GetBaseRef() /= fname, *this)
				PDefHOp(Path&, /=, const Path& pth)
				ImplRet(GetBaseRef() /= pth, *this)
				//@}

				friend PDefHOp(bool, == , const Path& x, const Path& y)
				ImplRet(x.GetBase() == y.GetBase())

				friend PDefHOp(bool, <, const Path& x, const Path& y)
				ImplRet(x.GetBase() < y.GetBase())

				/*!
				\brief 转换为字符串。
				\sa Verify
				*/
				explicit
				DefCvt(const, String, GetLeafString())
				/*!
				\brief 转换为窄字符串。
				*/
				explicit
				DefCvt(const, string, String(*this).GetMBCS())

				DefGetter(const lnothrow, const lpath&, Base, *this)
				DefGetter(lnothrow, lpath&, BaseRef, *this)
				//@{
				//! \brief 取不带分隔符结束的字符串。
				PDefH(String, GetLeafString,
					char16_t delimiter = FetchSeparator<char16_t>()) const
				ImplRet(leo::to_string(GetBase(), { delimiter }))
				/*!
				\brief 取指定分隔符的字符串表示。
				\post 断言：结果为空或以分隔符结束。
				*/
				String
				GetString(char16_t = FetchSeparator<char16_t>()) const;
			//@}
			/*!
			\brief 正规化：去除自指和父节点的路径成员。
			*/
			PDefH(void, Normalize, )
				ImplExpr(leo::normalize(*this))

				/*!
				\brief 验证：转换为指定分隔符表示的字符串并检查分隔符。
				\note 使用 VerifyDirectory 验证，当且仅当确认为可打开的目录时结果以分隔符结束。
				\sa GetString
				\sa VerifyDirectory
				*/
				String
				Verify(char16_t = FetchSeparator<char16_t>()) const;
			/*!
			\brief 验证指定分隔符和编码的多字节字符串。
			*/
			PDefH(string, VerifyAsMBCS,
				char16_t delimiter = char16_t(FetchSeparator<char16_t>()),
				Text::Encoding enc = Text::CS_Default) const
				ImplRet(Verify(delimiter).GetMBCS(enc))

				//@{
				using lpath::back;

			using lpath::begin;

			using lpath::cbegin;

			using lpath::clear;

			using lpath::cend;

			using lpath::empty;

			using lpath::end;

			using lpath::erase;

			using lpath::filter_self;

			using lpath::front;

			using lpath::insert;

			using lpath::is_absolute;

			using lpath::is_relative;

			using lpath::max_size;

			using lpath::merge_parents;

			using lpath::pop_back;

			using lpath::push_back;

			using lpath::size;

			//@{
			friend PDefH(void, swap, Path& x, lpath& y)
				ImplExpr(y.swap(x));
			friend PDefH(void, swap, lpath& x, Path& y)
				ImplExpr(x.swap(y));
			friend PDefH(void, swap, Path& x, Path& y)
				ImplExpr(y.GetBaseRef().swap(x))
				//@}

				friend PDefH(String, to_string, const Path& pth)
				ImplRet(to_string(pth.GetBase(), { FetchSeparator<char16_t>() }))
				//@}
		};


		/*!
		\brief 取扩展名。
		\note 非贪婪匹配。
		*/
		//@{
		//! \since build 599
		template<class _tString>
		_tString
			GetExtensionOf(const _tString& fname)
		{
			return leo::find_suffix(fname, typename _tString::value_type('.'));
		}
		//! \relates Path
		inline PDefH(String, GetExtensionOf, const Path& pth)
			ImplRet(pth.empty() ? String() : GetExtensionOf(pth.back()))
			//@}


			//! \relates Path
			//@{
			/*!
			\brief 判断路径表示绝对路径。
			*/
			inline PDefH(bool, IsAbsolute, const Path& pth)
			ImplRet(!pth.empty() && IsAbsolute(pth.GetString()))

			/*!
			\brief 根据当前工作目录和指定路径取绝对路径。
			\post 断言：结果是绝对路径。
			\return 若指定路径为相对路径则为正规化当前工作目录追加此路径，否则为正规化指定路径。
			\note 第二参数指定取当前工作目录指定的起始缓冲区大小。
			\sa IsRelative
			\sa Path::Normalize
			\sa FetchCurrentWorkingDirectory
			*/
			LF_API Path
			MakeNormalizedAbsolute(const Path&, size_t = MaxPathLength);
		//@}


		/*!
		\brief 验证路径表示的目录是否存在且可打开。
		\note 受权限限制。
		*/
		//@{
		LF_API LB_NONNULL(1) bool
			VerifyDirectory(const char*);
		LF_API LB_NONNULL(1) bool
			VerifyDirectory(const char16_t*);
		inline PDefH(bool, VerifyDirectory, const string& path)
			ImplRet(VerifyDirectory(path.c_str()))
			inline PDefH(bool, VerifyDirectory, const u16string& path)
			ImplRet(VerifyDirectory(path.c_str()))
			inline PDefH(bool, VerifyDirectory, const String& path)
			ImplRet(VerifyDirectory(path.c_str()))
			inline PDefH(bool, VerifyDirectory, const Path& pth)
			ImplRet(!pth.empty() && VerifyDirectory(pth.GetString()))
			//@}

			/*!
			\brief 验证目录路径结尾：确保当且仅当验证成功时保留结尾字符。
			\note 使用 ADL VerfiyDirectory 。
			*/
			template<class _tString>
		inline leo::decay_t<_tString>
			VerifyDirectorlpathTail(_tString&& str)
		{
			if (!(str.empty() || VerifyDirectory(str)))
				str.pop_back();
			return lforward(str);
		}


		/*!
		\brief 验证路径表示的目录是否存在，若不存在则逐级创建。
		\throw std::system_error 失败时根据 errno 抛出的异常。
		\note 使用 umkdir 实现。
		*/
		//@{
		LF_API void
			EnsureDirectory(const Path&);
		inline PDefH(void, EnsureDirectory, const char* path)
			ImplExpr(EnsureDirectory(Path(path)))
			inline PDefH(void, EnsureDirectory, const string& path)
			ImplExpr(EnsureDirectory(path.c_str()))
			inline PDefH(void, EnsureDirectory, const String& path)
			ImplExpr(EnsureDirectory(path.GetMBCS()))
			//@}


			/*!
			\brief 解析路径：取跟踪链接的绝对路径。
			\return 解析得到的绝对路径。
			\exception std::system_error std::errc::too_many_symbolic_link_levels
			超过链接限制。
			\note 忽略空路径组件。
			\note 使用 ADL cbegin 和 cend 。
			*/
			//@{
			/*!
			\note 第二参数指定起始路径。
			\note 第三参数指定解析链接的次数上限。
			*/
			template<class _tPath = Path, class _tString = typename _tPath::value_type,
			class _tTraits = typename _tPath::traits_type>
			_tPath
			ResolvePathWithBase(const _tString& sv, _tPath base,
				size_t n = FetchLimit(SystemOption::MaxSymlinkLoop))
		{
			using leo::cbegin;
			using leo::cend;

			return IO::ParsePath<_tPath>(sv,
				[&](_tPath& pth, decltype(cbegin(sv)) b, decltype(cend(sv)) e) {
				pth /= typename _tPath::value_type(b, e);
				if (pth.size() == 1)
					_tTraits::AdjustForRoot(pth,
						typename _tPath::value_type(cbegin(sv), e));

				auto str(to_string(pth));

				// XXX: This may be not efficient because underlying calls will resolve
				//	the path prefix repeatedly.
				// FIXME: Blocked. TOCTTOU access.
				if (IO::IterateLink(str, n))
				{
					// XXX: Handle absolute device path, etc?
					if (IsAbsolute(str))
						pth = IO::ParsePath<_tPath>(std::move(str));
					else
					{
						// TODO: Check condition?
						if (!pth.empty())
							pth.pop_back();
						pth /= str;
					}
				}
			}, std::move(base));
		}

		/*!
		\note 第二参数指定解析链接的次数上限。
		\note 第三参数指定起始路径使用的初始缓冲区大小。
		*/
		template<class _tPath = Path, class _tString = typename _tPath::value_type,
			class _tTraits = typename _tPath::traits_type>
			_tPath
			ResolvePath(const _tString& sv,
				size_t n = FetchLimit(SystemOption::MaxSymlinkLoop),
				size_t init_len = MaxPathLength)
		{
			return IO::ResolvePathWithBase(sv, IsAbsolute(sv.data()) ? _tPath()
				: IO::ParsePath<_tPath>(FetchCurrentWorkingDirectory<typename
					_tString::value_type>(init_len)), n);
		}
		//@}


		/*!
		\brief 目录遍历操作。
		*/
		//@{
		template<typename _func>
		void
			Traverse(HDirectory& dir, _func f)
		{
			for (NativePathView npv : dir)
			{
				LAssert(!npv.empty(), "Empty name found.");
				// TODO: Allow user code to specify filtering on self node?
				if (!PathTraits::is_self(npv))
					f(dir.GetNodeCategory(), npv);
			}
		}
		//! \exception std::system_error 目录打开失败。
		//@{
		//! \note 允许目录路径以分隔符结束。
		//@{
		//! \pre 间接断言：指针参数非空。
		template<typename _func>
		inline LB_NONNULL(1) void
			Traverse(const char* path, _func f)
		{
			// NOTE: Separators at end of path is allowed by %HDirectory::HDirectory.
			HDirectory dir(path);

			IO::Traverse(dir, f);
		}
		//! \since build 593
		template<typename _func>
		inline void
			Traverse(const string& path, _func f)
		{
			IO::Traverse(path.c_str(), f);
		}
		//@}
		template<typename _func>
		inline void
			Traverse(const Path& pth, _func f)
		{
			IO::Traverse(string(pth), f);
		}
		//@}

		//! \note 允许目录路径以分隔符结束。
		//@{
		/*!
		\pre 间接断言：指针参数非空。
		*/
		template<typename _func>
		LB_NONNULL(1) void
			TraverseChildren(const char* path, _func f)
		{
			IO::Traverse(path, [f](NodeCategory c, NativePathView npv)
				ynoexcept_spec(f(c, npv)) {
				if (!PathTraits::is_parent(npv))
					f(c, npv);
			});
		}
		template<typename _func>
		void
			TraverseChildren(const string& path, _func f)
		{
			IO::TraverseChildren(path.c_str(), f);
		}
		//@}
		template<typename _func>
		inline void
			TraverseChildren(const Path& pth, _func f)
		{
			IO::TraverseChildren(string(pth), f);
		}
		//@}

		/*!
		\brief 递归遍历目录树。
		\warning 不检查无限递归调用。
		\note 使用 ADL \c TraverseChildren 遍历子目录。
		\note 函数传入的路径参数可以被修改。
		*/
		template<typename _func, typename... _tParams>
		auto
			TraverseTree(_func f, const Path& dst, const Path& src, _tParams&&... args)
			-> decltype(f(std::declval<Path&>(), std::declval<Path&>(),
				std::forward<_tParams>(args)...))
		{
			EnsureDirectory(dst);
			TraverseChildren(src, [&](NodeCategory c, NativePathView npv) {
				const String name(npv);
				// NOTE: Allowed to be modified.
				auto dname(dst / name), sname(src / name);

				// XXX: Blocked. 'lforward' cause G++ 5.2 crash: internal compiler
				//	error: Aborted (program cc1plus) or crash silently.
				return bool(c & NodeCategory::Directory)
					? IO::TraverseTree(f, dname, sname, std::forward<_tParams>(args)...)
					: f(dname, sname, std::forward<_tParams>(args)...);
			});
		}


		//@{
		/*!
		\brief 复制文件处理器：通知文件复制事件。
		\note 函数参数分别对应目标和源。
		*/
		using CopyFileHandler = std::function<void(FileDescriptor, FileDescriptor)>;

		//! \sa CopyFileHandler
		//@{
		//! \brief 保持修改时间。
		const auto PreserveModificationTime(leo::bind_forward(
			&FileDescriptor::SetModificationTime, &FileDescriptor::GetModificationTime));
		//! \brief 保持修改和访问时间。
		const auto PreserveModificationAndAccessTime(leo::bind_forward(
			&FileDescriptor::SetModificationAndAccessTime,
			&FileDescriptor::GetModificationAndAccessTime));
		//@}
		//@}

		/*!
		\brief 复制文件。
		\pre 间接断言：表示目标和源的参数非空。
		\note 第一参数表示目标，第二参数表示源。
		\note mode_t 参数依次表示打开目标和源的权限模式。
		\note 不复制元数据。
		\see $2015-09 @ %Documentation::Workflow::Annual2015.
		*/
		//@{
		/*!
		\note 不清空目标。
		*/
		LF_API void
			CopyFile(UniqueFile, FileDescriptor);
		//! \exception std::system_error 打开文件失败。
		//@{
		//! \note 不清空目标。
		LF_API LB_NONNULL(2) void
			CopyFile(UniqueFile, const char*);
		//! \sa EnsureUniqueFile
		//@{
		//! \note 除第二参数外含义和 EnsureUniqueFile 的参数依次相同。
		//@{
		LF_API LB_NONNULL(1) void
			CopyFile(const char*, FileDescriptor, mode_t = DefaultPMode(),
				size_t = 1, bool = {});
		LF_API LB_NONNULL(1, 2) void
			CopyFile(const char*, const char*, mode_t = DefaultPMode(), size_t = 1,
				bool = {});
		//@}
		/*!
		\exception std::bad_function_call 第三参数为空。
		\note 第三参数指定成功复制内容后关闭目标文件前的操作。
		\note 除第二和第三参数外含义和 EnsureUniqueFile 的参数依次相同。
		*/
		//@{
		LF_API LB_NONNULL(1) void
			CopyFile(const char*, FileDescriptor, CopyFileHandler, mode_t = DefaultPMode(),
				size_t = 1, bool = {});
		LF_API LB_NONNULL(1, 2) void
			CopyFile(const char*, const char*, CopyFileHandler, mode_t = DefaultPMode(),
				size_t = 1, bool = {});
		//@}
		//@}
		//@}
		//@}

		//! \note 失败时立刻终止操作。
		//@{
		/*!
		\brief 复制目录树。
		\warning 不检查无限递归调用。
		\note 使用 ADL \c TraverseChildren 递归遍历。
		*/
		template<typename... _tParams>
		void
			CopyTree(const Path& dst, const Path& src, _tParams&&... args)
		{
			// XXX: Blocked. 'lforward' may cause G++ 5.2 silent crash.
			IO::TraverseTree([](const Path& dname, const Path& sname,
				_tParams&&... fargs) {
				IO::CopyFile(string(dname).c_str(), string(sname).c_str(),
					std::forward<_tParams>(fargs)...);
			}, dst, src, std::forward<_tParams>(args)...);
		}

		//! \exception std::system_error 路径指向的不是一个目录或删除失败。
		//@{
		/*!
		\brief 清空参数指定路径的目录树内容。
		\since build 639
		*/
		//@{
		LF_API void
			ClearTree(const Path&);
		inline PDefH(void, ClearTree, const string& pth)
			ImplRet(ClearTree(Path(pth)))
			//@}

			//! \brief 删除参数指定路径的目录树。
			//@{
			inline PDefH(void, DeleteTree, const Path& pth)
			ImplExpr(ClearTree(pth), Remove(string(pth).c_str()))
			//! \since build 593
			inline PDefH(void, DeleteTree, const string& pth)
			ImplExpr(DeleteTree(Path(pth)))
			//@}
			//@}

			/*!
			\brief 遍历目录中的项目，更新至列表。
			\exception std::system_error 操作失败。
			*/
			LF_API void
			ListFiles(const Path&, vector<String>&);
		//@}


		/*!
		\brief 按文件系统节点类别对路径分类。
		\todo 支持目录和常规文件外的其它类别。
		*/
		LF_API NodeCategory
			ClassifyNode(const Path&);

	} // namespace IO;

} // namespace leo;

#endif

