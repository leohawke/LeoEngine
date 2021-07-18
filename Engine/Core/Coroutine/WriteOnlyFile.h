#pragma once

#include "writable_file.h"
#include "filemode.h"
#include "Task.h"
#include <filesystem>

namespace leo::coroutine {
	class WriteOnlyFile : public writable_file
	{
	public:

		/// Open a file for write-only access.
		///
		/// \param ioContext
		/// The I/O context to use when dispatching I/O completion events.
		/// When asynchronous write operations on this file complete the
		/// completion events will be dispatched to an I/O thread associated
		/// with the I/O context.
		///
		/// \param pathMode
		/// Path of the file to open.
		///
		/// \param openMode
		/// Specifies how the file should be opened and how to handle cases
		/// when the file exists or doesn't exist.
		///
		/// \param shareMode
		/// Specifies the access to be allowed on the file concurrently with this file access.
		///
		/// \param bufferingMode
		/// Specifies the modes/hints to provide to the OS that affects the behaviour
		/// of its file buffering.
		///
		/// \return
		/// An object that can be used to write to the file.
		///
		/// \throw std::system_error
		/// If the file could not be opened for write.
		[[nodiscard]]
		static WriteOnlyFile open(
			IOScheduler& ioService,
			const std::filesystem::path& path,
			file_open_mode openMode = file_open_mode::create_or_open,
			file_share_mode shareMode = file_share_mode::none,
			file_buffering_mode bufferingMode = file_buffering_mode::default_);

	protected:
		WriteOnlyFile(win32::handle_t&& fileHandle) noexcept;
	};
}