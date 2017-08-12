/*!	\file LAdaptor.h
\ingroup FrameWork
\brief 外部库关联。
*/

#ifndef FrameWork_LAdaptor_h
#define FrameWork_LAdaptor_h 1

#include<LBase/algorithm.hpp>
#include<LBase/functional.hpp>
#include<LFramework/LCLib/Debug.h>
#include<LFramework/LCLib/Mutex.h>
#include<LFramework/LCLib/FReference.h>
#include<LFramework/LCLib/FFileIO.h>

namespace leo {
	using platform::list;
	using platform::vector;
	using platform::unordered_map;
	using platform::multimap;
	using platform::set;

	using platform::forward_as_tuple;
	using platform::get;
	using platform::ignore;
	using platform::make_pair;
	using platform::make_tuple;
	using platform::pair;
	using platform::tie;
	using platform::tuple;
	using platform::tuple_cat;

	using platform::basic_string;
	using platform::string;
	using platform::wstring;
	using platform::sfmt;
	using platform::vsfmt;

	using platform::to_string;
	using platform::to_wstring;

	using platform::basic_string_view;
	using platform::string_view;
	using platform::u16string_view;
	using platform::wstring_view;

	using platform::bad_weak_ptr;
	using platform::const_pointer_cast;
	using platform::dynamic_pointer_cast;
	using platform::enable_shared_from_this;
	using platform::get_deleter;

	using platform::make_observer;
	using platform::make_shared;
	using platform::make_unique;
	using platform::make_unique_default_init;
	using platform::get_raw;
	using platform::observer_ptr;
	using platform::owner_less;
	using platform::reset;
	using platform::share_raw;
	using platform::shared_ptr;
	using platform::static_pointer_cast;
	using platform::unique_raw;
	using platform::unique_ptr;
	using platform::unique_ptr_from;
	using platform::weak_ptr;

	using platform::lref;

	//@{
	using platform::nptr;
	using platform::tidy_ptr;
	using platform::pointer_iterator;
	//@}

	/*!
	\brief 解锁删除器：使用线程模型对应的互斥量和锁。
	*/
	using platform::Threading::unlock_delete;


	/*!
	\brief 并发操作。
	*/
	using namespace platform::Concurrency;
	/*!
	\brief 平台公用描述。
	*/
	using namespace platform::Descriptions;

	/*!
	\brief 基本实用例程。
	*/
	//@{
	using platform::usystem;
	//@}

	/*!
	\brief 日志。
	*/
	//@{
	using platform::Echo;
	using platform::Logger;
	using platform::FetchCommonLogger;
	//@}

	/*!
	\brief 断言操作。
	\pre 允许对操作数使用 ADL 查找同名声明，但要求最终的表达式语义和调用这里的声明等价。
	*/
	using platform::Nonnull;

	using platform::FwdIter;
	using platform::Deref;

	/*!
	\brief 文件访问例程。
	*/
	//@{
	using platform::upclose;
	using platform::upopen;

	using platform::basic_filebuf;
	using platform::filebuf;
	using platform::wfilebuf;
	using platform::basic_ifstream;
	using platform::basic_ofstream;
	using platform::basic_fstream;
	using platform::ifstream;
	using platform::ofstream;
	using platform::fstream;
	using platform::wifstream;
	using platform::wofstream;
	using platform::wfstream;
	//@}
}
#endif