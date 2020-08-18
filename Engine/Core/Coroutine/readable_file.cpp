#include "readable_file.h"

leo::coroutine::file_read_operation leo::coroutine::readable_file::read(
	std::uint64_t offset,
	void* buffer,
	std::size_t byteCount) const noexcept
{
	return file_read_operation(
		m_fileHandle,
		offset,
		buffer,
		byteCount);
}
