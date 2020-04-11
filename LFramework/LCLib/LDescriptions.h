#pragma once

#include <LBase/ldef.h>
#include <cstdint>

namespace platform
{
	/*!
	\brief 平台描述空间。
	*/
	namespace Descriptions
	{

		/*!
		\brief 记录等级。
		*/
		enum RecordLevel : std::uint8_t
		{
			Emergent = 0x00,
			Alert = 0x20,
			Critical = 0x40,
			Err = 0x60,
			Warning = 0x80,
			Notice = 0xA0,
			Informative = 0xC0,
			Debug = 0xE0
		};

	} // namespace Descriptions;
}