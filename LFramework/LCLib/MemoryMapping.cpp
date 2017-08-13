#include "MemoryMapping.h"
#include "FFileIO.h"
#include "NativeAPI.h"
#include <LBase/cast.hpp>

#if LFL_Win32
#include "Host.h"
#include "../Win32/LCLib/Mingw32.h"
#endif

namespace platform
{

#if !LFL_DS
	void
		UnmapDelete::operator()(pointer p) const lnothrow
	{
		// XXX: Error ignored.
#if LFL_Win32
		::UnmapViewOfFile(p);
#else
		::munmap(p, size);
#endif
	}
#endif


	MappedFile::MappedFile(std::uint64_t off, size_t len, UniqueFile f,
		FileMappingOption opt, FileMappingKey key)
		: option(opt), file(std::move(f)), mapped(
#if LFL_DS
			new byte[len]
#else
			[=](){
		try
		{
			if (len != 0)
			{
#	if LFL_Win32
				const auto h(platform_ex::ToHandle(*file.get()));

				if (h != INVALID_HANDLE_VALUE)
				{
					platform_ex::UniqueHandle fm(LCL_CallF_Win32(
						CreateFileMapping, h, {},
						opt != FileMappingOption::ReadWrite ? PAGE_READONLY
						: PAGE_READWRITE, 0, 0, wcast(key)));

					return static_cast<byte*>(LCL_CallF_Win32(MapViewOfFile,
						fm.get(), opt != FileMappingOption::CopyOnWrite
						? (opt == FileMappingOption::ReadOnly ? FILE_MAP_READ
							: FILE_MAP_ALL_ACCESS) : FILE_MAP_COPY,
						static_cast<unsigned long>(off >> 32UL),
						static_cast<unsigned long>(off), len));
				}
				LCL_Raise_SysE(, "::_get_osfhandle", lfsig);
#	else
				// NOTE: Since no %MAP_FIXED or extensions can be specified
				//	here, there is no need to convert %ENOMEM to throwing
				//	%std::bad_alloc or derived exceptions.
				const auto
					p(::mmap({}, len, opt != FileMappingOption::ReadWrite
						? PROT_READ : PROT_READ | PROT_WRITE, opt
						== FileMappingOption::CopyOnWrite || key == FileMappingKey()
						? MAP_PRIVATE : MAP_SHARED, *file.get(), ::off_t(off)));

				if (p != MAP_FAILED)
					return static_cast<byte*>(p);
				YCL_Raise_SysE(, "::mmap", lfsig);
#	endif
			}
			throw std::invalid_argument("Empty file found.");
		}
		// TODO: Create specific exception type?
		CatchExpr(...,
			std::throw_with_nested(std::runtime_error("Mapping failed.")))
	}()
#endif
			// NOTE: For platform %Win32, relying on ::NtQuerySection is not allowed by
			//	current platform implmentation policy.
			, len)
	{
#if !LF_Hosted
		lunused(key);
#endif
	}
		MappedFile::MappedFile(size_t len, UniqueFile f,
			FileMappingOption opt, FileMappingKey key)
			: MappedFile(std::uint64_t(0), len, std::move(f), opt, key)
		{}
		MappedFile::MappedFile(UniqueFile f, FileMappingOption opt, FileMappingKey key)
			: MappedFile([&] {
			if (f)
			{
				const auto s(leo::not_widen<size_t>(f->GetSize()));

				return pair<size_t, UniqueFile>(s, std::move(f));
			}
			leo::throw_error(std::errc::bad_file_descriptor, lfsig);
		}(), opt, key)
		{}
		MappedFile::MappedFile(const char* path, FileMappingOption opt,
			FileMappingKey key)
			: MappedFile([&] {
			if (UniqueFile ufile{ uopen(path, int((opt == FileMappingOption::ReadOnly
				? OpenMode::Read : OpenMode::ReadWrite) | OpenMode::Binary)
			) })
				return ufile;
			LCL_Raise_SysE(, "Failed opening file", lfsig);
		}(), opt, key)
		{}
		MappedFile::~MappedFile()
		{
			if (*this)
			{
				// NOTE: At least POSIX specifiy nothing about mandontory of flush on
				//	unmapping.
				// NOTE: Windows will only flush when all shared mapping are closed, see
				//	https://msdn.microsoft.com/en-us/library/windows/desktop/aa366532(v=vs.85).aspx
				// TODO: Simplified without exceptions?
				TryExpr(Flush())
					CatchExpr(std::exception& e, TraceDe(Descriptions::Err,
						"Failed flushing mapped file: %s.", e.what()))
					CatchExpr(..., TraceDe(Descriptions::Err,
						"Failed flushing mapped file."));
			}
		}

		void
			MappedFile::FlushView()
		{
			LAssertNonnull(GetPtr());

#if LFL_DS
			// NOTE: Nothing to do to flush the view.
#elif LFL_Win32
			LCL_CallF_Win32(::FlushViewOfFile, GetPtr(), GetSize());
			// NOTE: It should be noted %::FlushFileBuffers is required to flush file
			//	buffer, but this will be called in file descriptor flush below.
#else
			// NOTE: It is unspecified that whether data in %MAP_PRIVATE mappings has
			//	any permanent storage locations, see http://pubs.opengroup.org/onlinepubs/9699919799/functions/msync.html.
			// NOTE: To notify the system scheduling at first. See https://jira.mongodb.org/browse/SERVER-12733.
			YCL_CallF_CAPI(, ::msync, GetPtr(), GetSize(), MS_ASYNC);
			YCL_CallF_CAPI(, ::msync, GetPtr(), GetSize(), MS_SYNC);
#endif
		}

		void
			MappedFile::FlushFile()
		{
			LAssertNonnull(file);

			if (option != FileMappingOption::ReadOnly)
				// XXX: In POSIX this is perhapers not required, but keeping uniformed
				//	behavior is better. See https://groups.google.com/forum/#!topic/comp.unix.programmer/pIiaQ6CUKjU
				// XXX: This would cause bad performance for some versions of Windows
				//	supported by this project, see https://jira.mongodb.org/browse/SERVER-12401.
				file->Flush();
		}

} // namespace platform;
