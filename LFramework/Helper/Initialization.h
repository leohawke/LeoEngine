#pragma once

#include "LFramework/Core/ValueNode.h"

namespace Test {

	/*!
	\brief ȡֵ���͸��ڵ㡣
	\pre ���ԣ��ѳ�ʼ����
	*/
	leo::ValueNode&
		FetchRoot() lnothrow;

	/*!
	\brief ���� LSLV1 �����ļ���
	\param show_info �Ƿ��ڱ�׼�������ʾ��Ϣ��
	\pre ��Ӷ��ԣ�ָ������ǿա�
	\return ��ȡ�����á�
	\note Ԥ����Ϊ�������ļ���������ο� Documentation::YSLib ��
	*/
	LB_NONNULL(1, 2) leo::ValueNode
		LoadLSLV1File(const char* disp, const char* path, leo::ValueNode(*creator)(),
			bool show_info = {});

	/*!
	\brief ����Ĭ�����á�
	\return ��ȡ�����á�
	\sa LoadLSLA1File
	*/
	leo::ValueNode LoadConfiguration(bool = {});
}
