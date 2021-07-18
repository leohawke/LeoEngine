#include "Archive.h"
#include "Core/Coroutine/FileAsyncStream.h"
#include "System/SystemEnvironment.h"
namespace LeoEngine
{
	class FileReadArchive : public AsyncArchive
	{
	public:
		FileReadArchive(const std::filesystem::path& path)
			:stream(Environment->Scheduler->GetIOScheduler(),path,leo::coroutine::file_share_mode::read)
			
		{
			ArIsLoading = true;
		}

		Task<AsyncArchive&> Serialize(void* v, leo::uint64 length) override
		{
			co_await stream.Read(v, length);
			co_return *this;
		}
	private:
		leo::coroutine::FileAsyncStream stream;
	};

	AsyncArchive* CreateFileReader(const std::filesystem::path& filename)
	{
		return new FileReadArchive(filename);
	}

	class FileWriteArchive : public AsyncArchive
	{
	public:
		FileWriteArchive(const std::filesystem::path& path)
			:stream(Environment->Scheduler->GetIOScheduler(), path, leo::coroutine::file_share_mode::write)
		{
			ArIsSaving = true;
		}

		Task<AsyncArchive&> Serialize(void* v, leo::uint64 length) override
		{
			co_await stream.Write(v, length);
			co_return *this;
		}
	private:
		leo::coroutine::FileAsyncStream stream;
	};

	AsyncArchive* CreateFileWriter(const std::filesystem::path& filename)
	{
		return new FileWriteArchive(filename);
	}
}