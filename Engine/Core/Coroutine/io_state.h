#pragma once

#include <LBase/ldef.h>
#include <utility>
#include <cstdint>
#include <system_error>
#include <experimental/coroutine>

struct _OVERLAPPED;

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

	class win32_overlapped_operation_base
		: protected io_state
	{
	public:

		win32_overlapped_operation_base(
			io_state::callback_type* callback) noexcept
			: io_state(callback)
			, error_code(0)
			, bytes_transferred(0)
		{}

		win32_overlapped_operation_base(
			void* pointer,
			io_state::callback_type* callback) noexcept
			: io_state(pointer, callback)
			, error_code(0)
			, bytes_transferred(0)
		{}

		win32_overlapped_operation_base(
			std::uint64_t offset,
			io_state::callback_type* callback) noexcept
			: io_state(offset, callback)
			, error_code(0)
			, bytes_transferred(0)
		{}

		_OVERLAPPED* get_overlapped() noexcept
		{
			return reinterpret_cast<_OVERLAPPED*>(
				static_cast<overlapped*>(this));
		}

		std::size_t get_result()
		{
			if (error_code != 0)
			{
				throw std::system_error{
					static_cast<int>(error_code),
					std::system_category()
				};
			}

			return bytes_transferred;
		}

		dword_t error_code;
		dword_t bytes_transferred;

	};

	template<typename OPERATION>
	class win32_overlapped_operation
		: protected win32_overlapped_operation_base
	{
	protected:

		win32_overlapped_operation() noexcept
			: win32_overlapped_operation_base(
				&win32_overlapped_operation::on_operation_completed)
		{}

		win32_overlapped_operation(void* pointer) noexcept
			: win32_overlapped_operation_base(
				pointer,
				&win32_overlapped_operation::on_operation_completed)
		{}

		win32_overlapped_operation(std::uint64_t offset) noexcept
			: win32_overlapped_operation_base(
				offset,
				&win32_overlapped_operation::on_operation_completed)
		{}

	public:

		bool await_ready() const noexcept { return false; }

		bool await_suspend(std::experimental::coroutine_handle<> awaitingCoroutine)
		{
			static_assert(std::is_base_of_v<win32_overlapped_operation, OPERATION>);

			continuation = awaitingCoroutine;
			return static_cast<OPERATION*>(this)->try_start();
		}

		decltype(auto) await_resume()
		{
			return static_cast<OPERATION*>(this)->get_result();
		}

	private:

		static void on_operation_completed(
			io_state* ioState,
			dword_t errorCode,
			dword_t numberOfBytesTransferred,
			[[maybe_unused]] ulongptr_t completionKey) noexcept
		{
			auto* operation = static_cast<win32_overlapped_operation*>(ioState);
			operation->error_code = errorCode;
			operation->bytes_transferred = numberOfBytesTransferred;
			operation->continuation.resume();
		}

		std::experimental::coroutine_handle<> continuation;
	};
}
