#include "file_write_operation.hpp"
#include <LFramework/LCLib/NativeAPI.h>

namespace leo::coroutine {
	bool file_write_operation_impl::try_start(
		win32::win32_overlapped_operation_base& operation) noexcept
	{
		const DWORD numberOfBytesToWrite =
			m_byteCount <= 0xFFFFFFFF ?
			static_cast<DWORD>(m_byteCount) : DWORD(0xFFFFFFFF);

		DWORD numberOfBytesWritten = 0;
		BOOL ok = ::WriteFile(
			m_fileHandle,
			m_buffer,
			numberOfBytesToWrite,
			&numberOfBytesWritten,
			operation.get_overlapped());
		const DWORD errorCode = ok ? ERROR_SUCCESS : ::GetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			// Completed synchronously.
			//
			// We are assuming that the file-handle has been set to the
			// mode where synchronous completions do not post a completion
			// event to the I/O completion port and thus can return without
			// suspending here.

			operation.error_code = errorCode;
			operation.bytes_transferred = numberOfBytesWritten;

			return false;
		}

		return true;
	}

	void file_write_operation_impl::cancel(
		win32::win32_overlapped_operation_base& operation) noexcept
	{
		(void)::CancelIoEx(m_fileHandle, operation.get_overlapped());
	}
}