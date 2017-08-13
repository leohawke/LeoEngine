#include "File.h"

namespace leo
{
	namespace IO
	{

		UniqueFile
			OpenFile(const char* filename, int omode, mode_t pmode)
		{
			if (UniqueFile p_ifile{ uopen(filename, omode, pmode) })
				return p_ifile;
			else
				leo::throw_error(errno, "Failed opening file '" + string(filename)
					+ '\'');
		}


		SharedInputMappedFileStream::SharedInputMappedFileStream(const char* path)
			: MappedFile(path), SharedIndirectLockGuard<const UniqueFile>(
				GetUniqueFile()), leo::membuf(leo::replace_cast<const char*>(
					GetPtr()), GetSize()), std::istream(this)
		{}

		ImplDeDtor(SharedInputMappedFileStream)


			UniqueLockedOutputFileStream::UniqueLockedOutputFileStream(int fd)
			: ofstream(UniqueFile(fd)), desc(fd), lock()
		{
			if (*this)
				lock = unique_lock<FileDescriptor>(desc);
		}
		UniqueLockedOutputFileStream::UniqueLockedOutputFileStream(UniqueFile ufile)
			: UniqueLockedOutputFileStream(*ufile.get())
		{
			ufile.release();
		}

		ImplDeDtor(UniqueLockedOutputFileStream)

	} // namespace IO;

} // namespace leo;
