#pragma once

#include <utility>
#include <cstdint>

namespace leo::coroutine::win32
{
	using handle_t = void*;
	using ulongptr_t = std::uintptr_t;
	using longptr_t = std::intptr_t;
	using dword_t = unsigned long;
	using socket_t = std::uintptr_t;
	using ulong_t = unsigned long;

	struct overlapped
	{
		ulongptr_t Internal;
		ulongptr_t InternalHigh;
		union
		{
			struct
			{
				dword_t Offset;
				dword_t OffsetHigh;
			};
			void* Pointer;
		};
		handle_t hEvent;
	};

	struct io_state : win32::overlapped
	{
		using callback_type = void(
			io_state* state,
			win32::dword_t errorCode,
			win32::dword_t numberOfBytesTransferred,
			win32::ulongptr_t completionKey);

		io_state(callback_type* callback = nullptr) noexcept
			: io_state(std::uint64_t(0), callback)
		{}

		io_state(void* pointer, callback_type* callback) noexcept
			: continuation_callback(callback)
		{
			this->Internal = 0;
			this->InternalHigh = 0;
			this->Pointer = pointer;
			this->hEvent = nullptr;
		}

		io_state(std::uint64_t offset, callback_type* callback) noexcept
			: continuation_callback(callback)
		{
			this->Internal = 0;
			this->InternalHigh = 0;
			this->Offset = static_cast<dword_t>(offset);
			this->OffsetHigh = static_cast<dword_t>(offset >> 32);
			this->hEvent = nullptr;
		}

		callback_type* continuation_callback;
	};
}
