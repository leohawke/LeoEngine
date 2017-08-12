#include "FileSystem.h"
#include <LBase/cstring.h>
#include <LBase/exception.h>

namespace leo
{
	using namespace Text;

	namespace IO
	{

		String
			Path::GetString(char16_t delimiter) const
		{
			const auto res(leo::to_string_d(GetBase(), delimiter));

			LAssert(res.empty() || res.back() == delimiter,
				"Invalid conversion result found.");
			return res;
		}

		String
			Path::Verify(char16_t delimiter) const
		{
			return VerifyDirectoryPathTail(GetString(delimiter));
		}


		Path
			MakeNormalizedAbsolute(const Path& pth, size_t init_size)
		{
			Path res(pth);

			if (IsRelative(res))
				res = Path(FetchCurrentWorkingDirectory<char16_t>(init_size)) / res;
			res.Normalize();
			LTraceDe(Debug, "Converted path is '%s'.", res.VerifyAsMBCS().c_str());
			LAssert(IsAbsolute(res), "Invalid path converted.");
			return res;
		}


		namespace
		{

			template<typename _tChar>
			LB_NONNULL(1) bool
				VerifyDirectoryImpl(const _tChar* path)
			{
				try
				{
					DirectorySession ss(path);

					return true;
				}
				CatchExpr(std::system_error& e, YTraceDe(Debug,
					"Directory verfication failed."), ExtractAndTrace(e, Debug))
					return {};
			}

		} // unnamed namespace;

		bool
			VerifyDirectory(const char* path)
		{
			return VerifyDirectoryImpl(path);
		}
		bool
			VerifyDirectory(const char16_t* path)
		{
			return VerifyDirectoryImpl(path);
		}

		void
			EnsureDirectory(const Path& pth)
		{
			string upath;

			for (const auto& name : pth)
			{
				upath += MakeMBCS(name.c_str()) + FetchSeparator<char>();
				if (!VerifyDirectory(upath) && !umkdir(upath.c_str()) && errno != EEXIST)
				{
					LTraceDe(Err, "Failed making directory path '%s'", upath.c_str());
					leo::throw_error(errno, yfsig);
				}
			}
		}


		namespace
		{

			LB_NONNULL(1) UniqueFile
				OpenFileForCopy(const char* src)
			{
				return
					OpenFile(src, omode_convb(std::ios_base::in | std::ios_base::binary));
			}

		} // unnamed namespace;

		void
			CopyFile(UniqueFile p_dst, FileDescriptor src_fd)
		{
			FileDescriptor::WriteContent(Nonnull(p_dst.get()), Nonnull(src_fd));
		}
		void
			CopyFile(UniqueFile p_dst, const char* src)
		{
			CopyFile(std::move(p_dst), OpenFileForCopy(src).get());
		}
		void
			CopyFile(const char* dst, FileDescriptor src_fd, mode_t dst_mode,
				size_t allowed_links, bool share)
		{
			FileDescriptor::WriteContent(EnsureUniqueFile(dst, dst_mode, allowed_links,
				share).get(), Nonnull(src_fd));
		}
		void
			CopyFile(const char* dst, const char* src, mode_t dst_mode,
				size_t allowed_links, bool share)
		{
			CopyFile(dst, OpenFileForCopy(src).get(), dst_mode, allowed_links, share);
		}
		void
			CopyFile(const char* dst, FileDescriptor src_fd, CopyFileHandler f,
				mode_t dst_mode, size_t allowed_links, bool share)
		{
			const auto p_dst(EnsureUniqueFile(dst, dst_mode, allowed_links,
				share));

			FileDescriptor::WriteContent(p_dst.get(), Nonnull(src_fd));
			// NOTE: Guards are not used since the post operations should only be
			//	performed after success copying.
			f(p_dst.get(), src_fd);
		}
		void
			CopyFile(const char* dst, const char* src, CopyFileHandler f, mode_t dst_mode,
				size_t allowed_links, bool share)
		{
			CopyFile(dst, OpenFileForCopy(src).get(), f, dst_mode, allowed_links,
				share);
		}


		void
			ClearTree(const Path& pth)
		{
			TraverseChildren(pth, [&](NodeCategory c, NativePathView npv) {
				const auto child(pth / String(npv));

				if (c == NodeCategory::Directory)
					DeleteTree(child);
				Remove(string(child).c_str());
			});
		}

		void
			ListFiles(const Path& pth, vector<String>& lst)
		{
			Traverse(pth, [&](NodeCategory c, NativePathView npv) {
				lst.push_back(!PathTraits::is_parent(npv)
					&& bool(c & NodeCategory::Directory) ? String(npv)
					+ FetchSeparator<char16_t>() : String(npv));
			});
		}


		NodeCategory
			ClassifyNode(const Path& pth)
		{
			if (pth.empty())
				return NodeCategory::Empty;

			const auto& fname(pth.back());

			switch (leo::classify_path<String, PathTraits>(fname))
			{
			case leo::path_category::empty:
				return NodeCategory::Empty;
			case leo::path_category::self:
			case leo::path_category::parent:
				break;
			default:
				return VerifyDirectory(pth) ? NodeCategory::Directory
					: (ufexists(String(pth).c_str()) ? NodeCategory::Regular
						: NodeCategory::Missing);
				// TODO: Implementation for other categories.
			}
			return NodeCategory::Unknown;
		}

	} // namespace IO;

} // namespace leo;
