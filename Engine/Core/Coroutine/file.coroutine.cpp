#include "file.h"
#include <LFramework/LCLib/NativeAPI.h>
#include "IOScheduler.h"

leo::coroutine::file::~file()
{
	CloseHandle(m_fileHandle);
}

std::uint64_t leo::coroutine::file::size() const
{
	LARGE_INTEGER size;
	BOOL ok = ::GetFileSizeEx(m_fileHandle, &size);
	if (!ok)
	{
		DWORD errorCode = ::GetLastError();
		throw std::system_error
		{
			static_cast<int>(errorCode),
			std::system_category(),
			"error getting file size: GetFileSizeEx"
		};
	}

	return size.QuadPart;
}

leo::coroutine::file::file(win32::handle_t&& fileHandle) noexcept
	: m_fileHandle(std::move(fileHandle))
{
}

leo::coroutine::win32::handle_t leo::coroutine::file::open(
	win32::dword_t fileAccess,
	IOScheduler& ioService,
	const std::filesystem::path& path,
	file_open_mode openMode,
	file_share_mode shareMode,
	file_buffering_mode bufferingMode)
{
	DWORD flags = FILE_FLAG_OVERLAPPED;
	if ((bufferingMode & file_buffering_mode::random_access) == file_buffering_mode::random_access)
	{
		flags |= FILE_FLAG_RANDOM_ACCESS;
	}
	if ((bufferingMode & file_buffering_mode::sequential) == file_buffering_mode::sequential)
	{
		flags |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	if ((bufferingMode & file_buffering_mode::write_through) == file_buffering_mode::write_through)
	{
		flags |= FILE_FLAG_WRITE_THROUGH;
	}
	if ((bufferingMode & file_buffering_mode::temporary) == file_buffering_mode::temporary)
	{
		flags |= FILE_ATTRIBUTE_TEMPORARY;
	}
	if ((bufferingMode & file_buffering_mode::unbuffered) == file_buffering_mode::unbuffered)
	{
		flags |= FILE_FLAG_NO_BUFFERING;
	}

	DWORD shareFlags = 0;
	if ((shareMode & file_share_mode::read) == file_share_mode::read)
	{
		shareFlags |= FILE_SHARE_READ;
	}
	if ((shareMode & file_share_mode::write) == file_share_mode::write)
	{
		shareFlags |= FILE_SHARE_WRITE;
	}
	if ((shareMode & file_share_mode::delete_) == file_share_mode::delete_)
	{
		shareFlags |= FILE_SHARE_DELETE;
	}

	DWORD creationDisposition = 0;
	switch (openMode)
	{
	case file_open_mode::create_or_open:
		creationDisposition = OPEN_ALWAYS;
		break;
	case file_open_mode::create_always:
		creationDisposition = CREATE_ALWAYS;
		break;
	case file_open_mode::create_new:
		creationDisposition = CREATE_NEW;
		break;
	case file_open_mode::open_existing:
		creationDisposition = OPEN_EXISTING;
		break;
	case file_open_mode::truncate_existing:
		creationDisposition = TRUNCATE_EXISTING;
		break;
	}

	// Open the file
	win32::handle_t fileHandle(
		::CreateFileW(
			path.wstring().c_str(),
			fileAccess,
			shareFlags,
			nullptr,
			creationDisposition,
			flags,
			nullptr));
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		const DWORD errorCode = ::GetLastError();
		throw std::system_error
		{
			static_cast<int>(errorCode),
			std::system_category(),
			"error opening file: CreateFileW"
		};
	}

	// Associate with the I/O service's completion port.
	const HANDLE result = ::CreateIoCompletionPort(
		fileHandle,
		ioService.native_iocp_handle(),
		0,
		0);
	if (result == nullptr)
	{
		const DWORD errorCode = ::GetLastError();
		throw std::system_error
		{
			static_cast<int>(errorCode),
			std::system_category(),
			"error opening file: CreateIoCompletionPort"
		};
	}

	// Configure I/O operations to avoid dispatching a completion event
	// to the I/O service if the operation completes synchronously.
	// This avoids unnecessary suspension/resuption of the awaiting coroutine.
	const BOOL ok = ::SetFileCompletionNotificationModes(
		fileHandle,
		FILE_SKIP_COMPLETION_PORT_ON_SUCCESS |
		FILE_SKIP_SET_EVENT_ON_HANDLE);
	if (!ok)
	{
		const DWORD errorCode = ::GetLastError();
		throw std::system_error
		{
			static_cast<int>(errorCode),
			std::system_category(),
			"error opening file: SetFileCompletionNotificationModes"
		};
	}

	return std::move(fileHandle);
}
