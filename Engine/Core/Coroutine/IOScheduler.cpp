#include "IOScheduler.h"
#include "io_state.h"
#include <LFramework/LCLib/NativeAPI.h>
#include <system_error>

namespace
{
	HANDLE create_io_completion_port(std::uint32_t concurrencyHint)
	{
		HANDLE handle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, concurrencyHint);
		if (handle == NULL)
		{
			DWORD errorCode = ::GetLastError();
			throw std::system_error
			{
				static_cast<int>(errorCode),
				std::system_category(),
				"Error creating IOScheduler: CreateIoCompletionPort"
			};
		}
		return handle;
	}
}

namespace leo::coroutine {

	IOScheduler::schedule_operation IOScheduler::schedule() noexcept
	{
		return schedule_operation{*this};
	}

	void* IOScheduler::native_iocp_handle() noexcept
	{
		return iocp_handle;
	}

	IOScheduler::IOScheduler()
		:IOScheduler(0)
	{
	}

	IOScheduler::IOScheduler(std::uint32_t concurrencyHint)
		:iocp_handle(create_io_completion_port(concurrencyHint))
		,operations_head(nullptr)
	{
	}

	std::uint64_t IOScheduler::process_one_pending_event()
	{
		std::uint64_t eventCount = 0;
		if (try_enter_event_loop())
		{
			constexpr bool waitForEvent = false;
			if (try_process_one_event(waitForEvent))
			{
				++eventCount;
			}
			exit_event_loop();
		}

		return eventCount;
	}

	bool IOScheduler::try_enter_event_loop() noexcept
	{
		auto currentState = thread_state.load(std::memory_order_relaxed);
		do
		{
			if ((currentState & stop_requested_flag) != 0)
			{
				return false;
			}
		} while (!thread_state.compare_exchange_weak(
			currentState,
			currentState + active_thread_count_increment,
			std::memory_order_relaxed));

		return true;
	}

	void IOScheduler::exit_event_loop() noexcept
	{
		thread_state.fetch_sub(active_thread_count_increment, std::memory_order_relaxed);
	}

	bool IOScheduler::try_process_one_event(bool waitForEvent)
	{
		const DWORD timeout = waitForEvent ? INFINITE : 0;

		while (true)
		{
			// Check for any schedule_operation objects that were unable to be
			// queued to the I/O completion port and try to requeue them now.
			try_reschedule_overflow_operations();

			DWORD numberOfBytesTransferred = 0;
			ULONG_PTR completionKey = 0;
			LPOVERLAPPED overlapped = nullptr;
			BOOL ok = ::GetQueuedCompletionStatus(
				iocp_handle,
				&numberOfBytesTransferred,
				&completionKey,
				&overlapped,
				timeout);
			if (overlapped != nullptr)
			{
				DWORD errorCode = ok ? ERROR_SUCCESS : ::GetLastError();

				auto* state = static_cast<win32::io_state*>(
					reinterpret_cast<win32::overlapped*>(overlapped));

				state->continuation_callback(
					state,
					errorCode,
					numberOfBytesTransferred,
					completionKey);

				return true;
			}
			else if (ok)
			{
				if (completionKey != 0)
				{
					// This was a coroutine scheduled via a call to
					// io_service::schedule().
					std::experimental::coroutine_handle<>::from_address(
						reinterpret_cast<void*>(completionKey)).resume();
					return true;
				}

				// Empty event is a wake-up request, typically associated with a
				// request to exit the event loop.
				// However, there may be spurious such events remaining in the queue
				// from a previous call to stop() that has since been reset() so we
				// need to check whether stop is still required.
			}
			else
			{
				const DWORD errorCode = ::GetLastError();
				if (errorCode == WAIT_TIMEOUT)
				{
					return false;
				}

				throw std::system_error
				{
					static_cast<int>(errorCode),
					std::system_category(),
					"Error retrieving item from IOScheduler queue: GetQueuedCompletionStatus"
				};
			}
		}
	}

	void IOScheduler::try_reschedule_overflow_operations() noexcept
	{
		auto* operation = operations_head.exchange(nullptr, std::memory_order_acquire);
		while (operation != nullptr)
		{
			auto* next = operation->pNext;
			BOOL ok = ::PostQueuedCompletionStatus(
				iocp_handle,
				0,
				reinterpret_cast<ULONG_PTR>(operation->continuation.address()),
				nullptr);
			if (!ok)
			{
				// Still unable to queue these operations.
				// Put them back on the list of overflow operations.
				auto* tail = operation;
				while (tail->pNext != nullptr)
				{
					tail = tail->pNext;
				}

				schedule_operation* head = nullptr;
				while (!operations_head.compare_exchange_weak(
					head,
					operation,
					std::memory_order_release,
					std::memory_order_relaxed))
				{
					tail->pNext = head;
				}

				return;
			}

			operation = next;
		}
	}

	void IOScheduler::schedule_impl(schedule_operation* operation) noexcept
	{
		const BOOL ok = ::PostQueuedCompletionStatus(
			iocp_handle,
			0,
			reinterpret_cast<ULONG_PTR>(operation->continuation.address()),
			nullptr);
		if (!ok)
		{
			// Failed to post to the I/O completion port.
			//
			// This is most-likely because the queue is currently full.
			//
			// We'll queue up the operation to a linked-list using a lock-free
			// push and defer the dispatch to the completion port until some I/O
			// thread next enters its event loop.
			auto* head = operations_head.load(std::memory_order_acquire);
			do
			{
				operation->pNext = head;
			} while (!operations_head.compare_exchange_weak(
				head,
				operation,
				std::memory_order_release,
				std::memory_order_acquire));
		}
	}
}