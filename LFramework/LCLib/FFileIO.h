/*!	\file FFileIO.h
\ingroup Framework
\brief ����������������������
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
	\brief �ļ�ģʽ���͡�
	*/
	//@{
#if LFL_Win32
	using mode_t = unsigned short;
#else
	using mode_t = ::mode_t;
#endif
	//@}

	/*!
	\brief �رչܵ�����
	\pre �����ǿգ���ʾͨ���� upopen ��ʹ����ͬʵ�ִ򿪵Ĺܵ�����
	\note ��������ͬ POSIX.1 2004 �� \c ::pclose ��������Ϊȡ����ʵ�֡�
	*/
	LB_API LB_NONNULL(1) int
		upclose(std::FILE*) lnothrowv;

	/*!
	\param filename �ļ���������ͬ POSIX \c ::popen ��
	\param mode ��ģʽ����������ͬ POSIX.1 2004 ��������Ϊȡ����ʵ�֡�
	\pre ���ԣ�\c filename ��
	\pre ��Ӷ��ԣ� \c mode ��
	\warning Ӧʹ�� upclose ������ std::close �رչܵ����������������δ������Ϊ��
	*/
	//@{
	//! \brief �� UTF-8 �ļ����޻���򿪹ܵ�����
	LB_API LB_NONNULL (1, 2) std::FILE*
		upopen(const char* filename, const char* mode) lnothrowv;
	//! \brief �� UCS-2 �ļ����޻���򿪹ܵ�����
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
	\brief ��ʾ�ļ�����ʧ�ܵ��쳣��
	*/
	class LB_API FileOperationFailure : public std::system_error
	{
	public:
		using system_error::system_error;

		DefDeCopyCtor(FileOperationFailure)
			/*!
			\brief ���������ඨ����Ĭ��ʵ�֡�
			*/
		~FileOperationFailure() override;
	};


	/*!
	\build �׳��� errno �Ͳ���ָ���� FileOperationFailure ����
	\throw FileOperationFailure errno ��ָ������������쳣��
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