#include "ReadOnlyFile.h"
#include <LFramework/LCLib/NativeAPI.h>

leo::coroutine::ReadOnlyFile leo::coroutine::ReadOnlyFile::open(
	IOScheduler& ioService,
	const std::filesystem::path& path,
	file_share_mode shareMode,
	file_buffering_mode bufferingMode)
{
	return ReadOnlyFile(file::open(
		GENERIC_READ,
		ioService,
		path,
		file_open_mode::open_existing,
		shareMode,
		bufferingMode));
}

leo::coroutine::ReadOnlyFile::ReadOnlyFile(
	win32::handle_t&& fileHandle) noexcept
	: file(std::move(fileHandle))
	, readable_file(win32::handle_t{})
{
}
