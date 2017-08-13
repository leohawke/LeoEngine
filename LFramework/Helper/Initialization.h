#pragma once

#include "LFramework/Core/ValueNode.h"

namespace Test {

	/*!
	\brief 取值类型根节点。
	\pre 断言：已初始化。
	*/
	leo::ValueNode&
		FetchRoot() lnothrow;

	/*!
	\brief 载入 LSLV1 配置文件。
	\param show_info 是否在标准输出中显示信息。
	\pre 间接断言：指针参数非空。
	\return 读取的配置。
	\note 预设行为、配置文件和配置项参考 Documentation::YSLib 。
	*/
	LB_NONNULL(1, 2) leo::ValueNode
		LoadLSLV1File(const char* disp, const char* path, leo::ValueNode(*creator)(),
			bool show_info = {});

	/*!
	\brief 载入默认配置。
	\return 读取的配置。
	\sa LoadLSLA1File
	*/
	leo::ValueNode LoadConfiguration(bool = {});
}
