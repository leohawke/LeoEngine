#ifndef LBase_cctype_h
#define LBase_cctype_h 1

#include "ldef.h"
#include <cctype>
#include <cwctype>

namespace leo
{
	/*!
	\brief ʹ�� US-ASCII �ַ����� std::isprint ʵ�֡�
	\see ISO C11 7.4/3 ��ע��
	*/
	lconstfn bool
		isprint_ASCII(char c)
	{
		return c >= 0x20 && c < 0x7F;
	}


	//! \brief ʹ�� ISO/IEC 8859-1 �ַ����� std::isprint ʵ�֡�
	lconstfn bool
		isprint_ISO8859_1(char c)
	{
		return isprint_ASCII(c) || unsigned(c) > 0xA0;
	}

	/*!
	\brief �����޹ص� std::isprint ʵ�֡�
	\note ��ǰʹ�û��� ISO/IEC 8859-1 �� Unicode ����ʵ�֡�
	\note ����Ϊ��� MSVCRT ��ʵ�ֵı�ͨ��
	\sa https://connect.microsoft.com/VisualStudio/feedback/details/799287/isprint-incorrectly-classifies-t-as-printable-in-c-locale
	*/
	lconstfn bool
		isprint(char c)
	{
		return limpl(isprint_ISO8859_1(c));
	}

	/*!
	\brief ת����Сд�ַ���
	\note �� ISO C ��׼���Ӧ�ӿڲ�ͬ���� ISO C++ \<locale\> �еĽӿ����ƣ�
	�����ͷ���ֵ���ַ����Ͷ����Ƕ�Ӧ���������͡�
	*/
	//@{
	template<typename _tChar>
	inline _tChar
		tolower(_tChar c) lnothrow
	{
		return _tChar(std::towupper(wint_t(c)));
	}
	inline char
		tolower(char c) lnothrow
	{
		return char(std::tolower(c));
	}


	template<typename _tChar>
	inline _tChar
		toupper(_tChar c) lnothrow
	{
		return _tChar(std::towlower(wint_t(c)));
	}
	inline char
		toupper(char c) lnothrow
	{
		return char(std::toupper(c));
	}
	//@}
}

#endif
