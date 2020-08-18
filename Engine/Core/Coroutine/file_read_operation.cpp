#include "file_read_operation.h"
#include <LFramework/LCLib/NativeAPI.h>

bool leo::coroutine::file_read_operation_impl::try_start(
	leo::coroutine::win32::win32_overlapped_operation_base& operation) noexcept
{
	const DWORD numberOfBytesToRead =
		m_byteCount <= 0xFFFFFFFF ?
		static_cast<DWORD>(m_byteCount) : DWORD(0xFFFFFFFF);

	DWORD numberOfBytesRead = 0;
	BOOL ok = ::ReadFile(
		m_fileHandle,
		m_buffer,
		numberOfBytesToRead,
		&numberOfBytesRead,
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
		operation.bytes_transferred = numberOfBytesRead;

		return false;
	}

	return true;
}

void leo::coroutine::file_read_operation_impl::cancel(
	leo::coroutine::win32::win32_overlapped_operation_base& operation) noexcept
{
	(void)::CancelIoEx(m_fileHandle, operation.get_overlapped());
}