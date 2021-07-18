#pragma once

#include "io_state.h"
#include "filemode.h"
#include <cstdint>
#include <filesystem>

namespace leo::coroutine {

	class IOScheduler;

	class file
	{
	public:
		file(file&& other) noexcept = default;

		virtual ~file();

		/// Get the size of the file in bytes.
		std::uint64_t size() const;

		static win32::handle_t open(
			win32::dword_t fileAccess,
			IOScheduler& ioService,
			const std::filesystem::path& path,
			file_open_mode openMode,
			file_share_mode shareMode,
			file_buffering_mode bufferingMode);
	protected:

		file(win32::handle_t&& fileHandle) noexcept;

		win32::handle_t m_fileHandle;
	};
}