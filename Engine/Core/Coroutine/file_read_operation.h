#pragma once

#include <LBase/ldef.h>
#include <coroutine>

#include "io_state.h"

namespace leo::coroutine {

	class file_read_operation_impl
	{
	public:

		file_read_operation_impl(
			win32::handle_t fileHandle,
			void* buffer,
			std::size_t byteCount) noexcept
			: m_fileHandle(fileHandle)
			, m_buffer(buffer)
			, m_byteCount(byteCount)
		{}

		bool try_start(win32::win32_overlapped_operation_base& operation) noexcept;
		void cancel(win32::win32_overlapped_operation_base& operation) noexcept;

	private:

		win32::handle_t m_fileHandle;
		void* m_buffer;
		std::size_t m_byteCount;
	};

	class file_read_operation
		: public win32::win32_overlapped_operation<file_read_operation>
	{
	public:

		file_read_operation(
			win32::handle_t fileHandle,
			std::uint64_t fileOffset,
			void* buffer,
			std::size_t byteCount) noexcept
			: win32_overlapped_operation<file_read_operation>(fileOffset)
			, m_impl(fileHandle, buffer, byteCount)
		{}

	private:

		friend class win32::win32_overlapped_operation<file_read_operation>;

		bool try_start() noexcept { return m_impl.try_start(*this); }

		file_read_operation_impl m_impl;
	};
}
