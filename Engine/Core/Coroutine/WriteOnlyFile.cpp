#include "WriteOnlyFile.h"
#include <LFramework/LCLib/NativeAPI.h>

using namespace leo::coroutine;

leo::coroutine::WriteOnlyFile leo::coroutine::WriteOnlyFile::open(
	leo::coroutine::IOScheduler& ioService,
	const std::filesystem::path& path,
	file_open_mode openMode,
	file_share_mode shareMode,
	file_buffering_mode bufferingMode)
{
	return WriteOnlyFile(file::open(
		GENERIC_WRITE,
		ioService,
		path,
		openMode,
		shareMode,
		bufferingMode));
}

leo::coroutine::WriteOnlyFile::WriteOnlyFile(
	win32::handle_t&& fileHandle) noexcept
	: file(std::move(fileHandle))
	, writable_file(win32::handle_t{})
{
}