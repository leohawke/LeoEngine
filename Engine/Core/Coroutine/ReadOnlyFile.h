#pragma once

#include "readable_file.h"
#include "filemode.h"
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
}
