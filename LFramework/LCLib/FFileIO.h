/*!	\file FFileIO.h
\ingroup Framework
\brief 容器、拟容器和适配器。
*/

#ifndef FrameWork_FFileIO_h
#define FrameWork_FFileIO_h 1

#include <LFramework/LCLib/Platform.h>
#include <LBase/lmacro.h>
#include <LBase/string.hpp>
#include <LFramework/LCLib/Debug.h>
#include <LFramework/LCLib/FReference.h>
#include<LBase/container.hpp>

#include <chrono>
#include <ios>
#include <fstream>
#include <system_error>

#if __GLIBCXX__
#include <ext/stdio_filebuf.h>
#include <LBase/utility.hpp>
#endif

namespace platform {
	/*!
	\brief 文件模式类型。
	*/
	//@{
#if LFL_Win32
	using mode_t = unsigned short;
#else
	using mode_t = ::mode_t;
#endif
	//@}

	/*!
	\brief 关闭管道流。
	\pre 参数非空，表示通过和 upopen 或使用相同实现打开的管道流。
	\note 基本语义同 POSIX.1 2004 的 \c ::pclose ，具体行为取决于实现。
	*/
	LB_API LB_NONNULL(1) int
		upclose(std::FILE*) lnothrowv;

	/*!
	\param filename 文件名，意义同 POSIX \c ::popen 。
	\param mode 打开模式，基本语义同 POSIX.1 2004 ，具体行为取决于实现。
	\pre 断言：\c filename 。
	\pre 间接断言： \c mode 。
	\warning 应使用 upclose 而不是 std::close 关闭管道流，否则可能引起未定义行为。
	*/
	//@{
	//! \brief 以 UTF-8 文件名无缓冲打开管道流。
	LB_API LB_NONNULL (1, 2) std::FILE*
		upopen(const char* filename, const char* mode) lnothrowv;
	//! \brief 以 UCS-2 文件名无缓冲打开管道流。
	LB_API LB_NONNULL (1, 2) std::FILE*
		upopen(const char16_t* filename, const char16_t* mode) lnothrowv;
	//@}

	using std::basic_filebuf;
	using std::filebuf;
	using std::wfilebuf;
	//@{
	using std::basic_ifstream;
	using std::basic_ofstream;
	using std::basic_fstream;
	using std::ifstream;
	using std::ofstream;
	using std::fstream;
	using std::wifstream;
	using std::wofstream;
	//@}
	using std::wfstream;

	/*!
	\brief 表示文件操作失败的异常。
	*/
	class LB_API FileOperationFailure : public std::system_error
	{
	public:
		using system_error::system_error;

		DefDeCopyCtor(FileOperationFailure)
			/*!
			\brief 虚析构：类定义外默认实现。
			*/
		~FileOperationFailure() override;
	};


	/*!
	\build 抛出由 errno 和参数指定的 FileOperationFailure 对象。
	\throw FileOperationFailure errno 和指定参数构造的异常。
	\relates FileOperationFaiure
	*/
	template<typename _tParam>
	LB_NORETURN void
		ThrowFileOperationFailure(_tParam&& arg, int err = errno)
	{
		leo::throw_error<FileOperationFailure>(err, lforward(arg));
	}
}


#endif