#pragma once

#include <LBase/ldef.h>
#include <coroutine>

#include "io_state.h"

namespace leo::coroutine {
	class file_write_operation_impl
	{
	public:

		file_write_operation_impl(
			win32::handle_t fileHandle,
			const void* buffer,
			std::size_t byteCount) noexcept
			: m_fileHandle(fileHandle)
			, m_buffer(buffer)
			, m_byteCount(byteCount)
		{}

		bool try_start(win32::win32_overlapped_operation_base& operation) noexcept;
		void cancel(win32::win32_overlapped_operation_base& operation) noexcept;

	private:

		win32::handle_t m_fileHandle;
		const void* m_buffer;
		std::size_t m_byteCount;

	};

	class file_write_operation
		: public win32::win32_overlapped_operation<file_write_operation>
	{
	public:

		file_write_operation(
			win32::handle_t fileHandle,
			std::uint64_t fileOffset,
			const void* buffer,
			std::size_t byteCount) noexcept
			: win32::win32_overlapped_operation<file_write_operation>(fileOffset)
			, m_impl(fileHandle, buffer, byteCount)
		{}

	private:

		friend class win32::win32_overlapped_operation<file_write_operation>;

		bool try_start() noexcept { return m_impl.try_start(*this); }

		file_write_operation_impl m_impl;

	};
}