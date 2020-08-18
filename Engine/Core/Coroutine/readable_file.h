#pragma once

#include "file.h"
#include "file_read_operation.h"

namespace leo::coroutine {
	class readable_file : virtual public file
	{
	public:
		/// Read some data from the file.
		///
		/// Reads \a byteCount bytes from the file starting at \a offset
		/// into the specified \a buffer.
		///
		/// \param offset
		/// The offset within the file to start reading from.
		/// If the file has been opened using file_buffering_mode::unbuffered
		/// then the offset must be a multiple of the file-system's sector size.
		///
		/// \param buffer
		/// The buffer to read the file contents into.
		/// If the file has been opened using file_buffering_mode::unbuffered
		/// then the address of the start of the buffer must be a multiple of
		/// the file-system's sector size.
		///
		/// \param byteCount
		/// The number of bytes to read from the file.
		/// If the file has been opeend using file_buffering_mode::unbuffered
		/// then the byteCount must be a multiple of the file-system's sector size.
		///
		/// \param ct
		/// An optional cancellation_token that can be used to cancel the
		/// read operation before it completes.
		///
		/// \return
		/// An object that represents the read-operation.
		/// This object must be co_await'ed to start the read operation.
		[[nodiscard]]
		file_read_operation read(
			std::uint64_t offset,
			void* buffer,
			std::size_t byteCount) const noexcept;
	protected:
		using file::file;
	};

}