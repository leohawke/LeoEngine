#pragma once

#include "file_read_operation.h"
#include "file_write_operation.hpp"
#include "file.h"
#include "Task.h"

namespace leo::coroutine
{
	class FileAsyncStream : public file
	{
	public:
		FileAsyncStream(IOScheduler& ioService,
			const std::filesystem::path& _path,
			file_share_mode opertionMode,
			file_buffering_mode bufferingMode = file_buffering_mode::default_);

		~FileAsyncStream();

		Task<std::size_t> Read(
			void* dstbuffer,
			std::size_t byteCount) noexcept;

		Task<std::size_t> Write(
			void* dstbuffer,
			std::size_t byteCount
		) noexcept;

		void Skip(std::uint64_t offset)
		{
			fileOffset += offset;
		}

		void SkipTo(std::uint64_t offset)
		{
			fileOffset = offset;
		}
	private:
		static  constexpr size_t bufferSize = 4096;
		std::byte buffer[bufferSize] = {};
		std::uint8_t bufferMode;
		std::uint64_t bufferCount :56 = 0;

		std::size_t bufferOffset = 0;

		std::uint64_t fileOffset = 0;

		//debug_info
#ifndef NDEBUG
		std::filesystem::path path;
#endif
	};
}