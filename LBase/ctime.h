/*!	\file ctime.h
\ingroup LBase
\brief ISO C 日期和时间接口扩展。
*/


#ifndef LBase_ctime_h_
#define LBase_ctime_h_ 1

#include <LBase/ldef.h>
#include <ctime> // for std::tm;

namespace leo
{
	//@{
	//! \brief 判断 std::tm 的日期数据成员范围是否有效。
	lconstfn bool
		is_date_range_valid(const std::tm& t)
	{
		return !(t.tm_mon < 0 || 12 < t.tm_mon || t.tm_mday < 1 || 31 < t.tm_mday);
	}

	//! \brief 判断 std::tm 的时间数据成员范围是否有效。
	lconstfn bool
		is_time_no_leap_valid(const std::tm& t)
	{
		return !(t.tm_hour < 0 || 23 < t.tm_hour || t.tm_hour < 0 || 59 < t.tm_min
			|| t.tm_sec < 0 || 59 < t.tm_min);
	}
	//@}

} // namespace leo;

#endif

