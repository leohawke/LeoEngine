#include "FileAsyncStream.h"
#include <LFramework/LCLib/NativeAPI.h>
#include <LFramework/Win32/LCLib/Mingw32.h>
#include <spdlog/spdlog.h>
using namespace leo::coroutine;

win32::handle_t StreamOpen(IOScheduler& ioService, const std::filesystem::path& path, file_share_mode opertionMode, file_buffering_mode bufferingMode)
{
	switch (opertionMode)
	{
	case leo::coroutine::file_share_mode::read:
		return file::open(
			GENERIC_READ,
			ioService,
			path,
			file_open_mode::open_existing,
			opertionMode,
			bufferingMode);
	case leo::coroutine::file_share_mode::write:
		return file::open(
			GENERIC_WRITE,
			ioService,
			path,
			file_open_mode::create_or_open,
			opertionMode,
			bufferingMode);
	}

	assert(false);

	return {};
}

leo::coroutine::FileAsyncStream::FileAsyncStream(IOScheduler& ioService, const std::filesystem::path& Inpath, file_share_mode opertionMode, file_buffering_mode bufferingMode)
	:file(StreamOpen(ioService, Inpath, opertionMode,bufferingMode))
	,bufferMode(static_cast<std::uint8_t>(opertionMode))
#ifndef NDEBUG
	,path(Inpath)
#endif
{
}

leo::coroutine::FileAsyncStream::~FileAsyncStream()
{
	if ((bufferMode & static_cast<std::uint8_t>(leo::coroutine::file_share_mode::write)) != 0)
	{
		file_write_operation(
			m_fileHandle,
			fileOffset,
			buffer,
			bufferOffset);

		DWORD numberOfBytesWritten = 0;
		SetFileCompletionNotificationModes(m_fileHandle, FILE_SKIP_COMPLETION_PORT_ON_SUCCESS);
		OVERLAPPED overlapped;
		overlapped.Offset = static_cast<DWORD>(fileOffset);
		overlapped.OffsetHigh =static_cast<DWORD>(fileOffset>>32);
		overlapped.hEvent = CreateEventW(nullptr, true, false, nullptr);
		overlapped.hEvent = (HANDLE)((uintptr_t)overlapped.hEvent | 1);
		auto ok = WriteFile(m_fileHandle, buffer, bufferOffset, &numberOfBytesWritten, &overlapped);

		overlapped.hEvent = (HANDLE)((uintptr_t)overlapped.hEvent & ~1);
		auto wait_result = WaitForSingleObject(overlapped.hEvent, INFINITE);
		if (wait_result == WAIT_FAILED)
		{
			spdlog::critical(platform_ex::Windows::Win32Exception::FormatMessage(::GetLastError()));
		}
		CloseHandle(overlapped.hEvent);

		/*
		* SyncWait(file_write_operation(
			m_fileHandle,
			fileOffset,
			buffer,
			bufferOffset));
		*/
	}
}

Task<std::size_t> FileAsyncStream::Read(
	void* dstbuffer,
	std::size_t byteCount) noexcept
{
	if (bufferOffset + byteCount < bufferCount)
	{
		std::memcpy(dstbuffer, &buffer[bufferOffset], byteCount);
		bufferOffset += byteCount;

		co_return byteCount;
	}
	else {
		auto readCount = bufferCount - bufferOffset;
		std::memcpy(dstbuffer, buffer + bufferOffset, readCount);
		bufferOffset = 0;
		byteCount -= readCount;

		while (byteCount > 0)
		{
			auto operCount = co_await file_read_operation(
				m_fileHandle,
				fileOffset,
				(std::byte*)dstbuffer + readCount,
				bufferSize);

			fileOffset += operCount;
			byteCount -= operCount;
			readCount += operCount;

			if (operCount != bufferSize)
				break;
		}

		if (byteCount > 0)
		{
			bufferCount = co_await file_read_operation(
				m_fileHandle,
				fileOffset,
				buffer,
				bufferSize);

			fileOffset += bufferCount;

			auto copycount = std::min<std::size_t>(bufferCount - bufferOffset, byteCount);
			std::memcpy((std::byte*)dstbuffer + readCount, buffer, copycount);
			readCount += copycount;
		}
		co_return readCount;
	}
}

Task<std::size_t> FileAsyncStream::Write(
	void* dstbuffer,
	std::size_t byteCount) noexcept
{
	if (bufferOffset + byteCount < bufferSize)
	{
		std::memcpy(&buffer[bufferOffset], dstbuffer, byteCount);
		bufferOffset += byteCount;

		co_return byteCount;
	}
	else {
		auto buffer_reamining = bufferSize - bufferOffset;
		std::memcpy(&buffer[bufferOffset], dstbuffer, buffer_reamining);
		bufferOffset = 0;
		std::uint64_t dstoffset =  co_await file_write_operation(
			m_fileHandle,
			fileOffset,
			buffer,
			bufferSize);
		byteCount -= buffer_reamining;
		fileOffset += dstoffset;

		while (byteCount >= bufferSize)
		{
			dstoffset += co_await file_write_operation(
				m_fileHandle,
				fileOffset,
				(std::byte*)dstbuffer + dstoffset,
				bufferSize);
			byteCount -= bufferSize;
			fileOffset += bufferSize;
		}

		if (byteCount > 0)
		{
			std::memcpy(buffer, (std::byte*)dstbuffer + dstoffset, byteCount);
			bufferOffset += byteCount;
			dstoffset += byteCount;
		}

		co_return dstoffset;
	}
}