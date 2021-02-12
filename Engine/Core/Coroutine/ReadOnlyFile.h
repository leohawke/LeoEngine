#pragma once

#include "readable_file.h"
#include "filemode.h"
#include "Task.h"
#include <filesystem>

namespace leo::coroutine {

	class ReadOnlyFile : public readable_file
	{
	public:

		/// Open a file for read-only access.
		///
		/// \param ioContext
		/// The I/O context to use when dispatching I/O completion events.
		/// When asynchronous read operations on this file complete the
		/// completion events will be dispatched to an I/O thread associated
		/// with the I/O context.
		///
		/// \param path
		/// Path of the file to open.
		///
		/// \param shareMode
		/// Specifies the access to be allowed on the file concurrently with this file access.
		///
		/// \param bufferingMode
		/// Specifies the modes/hints to provide to the OS that affects the behaviour
		/// of its file buffering.
		///
		/// \return
		/// An object that can be used to read from the file.
		///
		/// \throw std::system_error
		/// If the file could not be opened for read.
		[[nodiscard]]
		static ReadOnlyFile open(
			IOScheduler& ioService,
			const std::filesystem::path& path,
			file_share_mode shareMode = file_share_mode::read,
			file_buffering_mode bufferingMode = file_buffering_mode::default_);

	protected:
		ReadOnlyFile(win32::handle_t&& fileHandle) noexcept;
	};

	class FileAsyncStream
	{
	public:
		FileAsyncStream(IOScheduler& ioService,
			const std::filesystem::path& _path,
			file_share_mode shareMode = file_share_mode::read,
			file_buffering_mode bufferingMode = file_buffering_mode::default_)
			:file(ReadOnlyFile::open(ioService,_path,shareMode,bufferingMode))
#ifndef NDEBUG
			,path(_path)
#endif
		{
		}

		Task<std::size_t> Read(
			void* dstbuffer,
			std::size_t byteCount) noexcept
		{
			if (bufferOffset + byteCount < bufferReadCount)
			{
				std::memcpy(dstbuffer, &buffer[bufferOffset], byteCount);
				bufferOffset += byteCount;

				co_return byteCount;
			}
			else {
				std::uint64_t dstoffset = 0;
				while (byteCount > 0)
				{
					auto readCount =std::min<std::size_t>(bufferReadCount - bufferOffset,byteCount);
					std::memcpy((std::byte*)dstbuffer + dstoffset, buffer+bufferOffset, readCount);

					byteCount -= readCount;
					dstoffset += readCount;
					bufferOffset += readCount;

					if (bufferOffset != bufferReadCount && bufferReadCount != 0)
						break;

					bufferReadCount = co_await file.read(fileOffset, buffer, bufferSize);
					fileOffset += bufferReadCount;
					bufferOffset = 0;

					if (bufferReadCount < bufferSize)
					{
						readCount = std::min<std::size_t>(bufferReadCount, byteCount);
						std::memcpy((std::byte*)dstbuffer + dstoffset, buffer, readCount);

						dstoffset += readCount;
						bufferOffset += readCount;

						break;
					}
				}
				co_return dstoffset;
			}
		}

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
		std::byte buffer[bufferSize];
		std::uint64_t bufferReadCount = 0;

		std::size_t bufferOffset = 0;

		std::uint64_t fileOffset = 0;
		ReadOnlyFile file;

		//debug_info
#ifndef NDEBUG
		std::filesystem::path path;
#endif
	};
}
