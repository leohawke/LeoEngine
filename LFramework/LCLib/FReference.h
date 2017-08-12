/*!	\file FReference.h
\ingroup Framework
\brief ָ������÷��ʲ���ģ�顣
*/

#ifndef FrameWork_FReference_h
#define FrameWork_FReference_h 1

#include<LBase/memory.hpp>
#include<LBase/ref.hpp>
#include<LBase/pointer.hpp>
#include<utility>


namespace platform
{
	inline namespace references
	{
		using limpl(std)::bad_weak_ptr;
		using limpl(std)::const_pointer_cast;
		using limpl(std)::default_delete;
		using limpl(std)::dynamic_pointer_cast;
		using limpl(std)::enable_shared_from_this;
		using limpl(std)::get_deleter;
		using limpl(std)::make_shared;
		using std::make_shared;
		using limpl(leo)::make_observer;
		using limpl(leo)::make_unique;
		using limpl(leo)::make_unique_default_init;
		using leo::get_raw;
		using limpl(std)::owner_less;
		using limpl(leo)::observer_ptr;
		using leo::reset;
		using leo::share_raw;
		using limpl(std)::shared_ptr;
		using limpl(std)::static_pointer_cast;
		using leo::unique_raw;
		using limpl(std)::unique_ptr;
		/*!
		\ingroup metafunctions
		\brief ȡɾ������Ӧ�� unique_ptr ʵ����
		\sa std::defer_element
		\sa std::unique_ptr_pointer

		Ԫ������ͨ�� unique_ptr_pointer ��ָ����ɾ�����ƶϣ���ʧ����Ϊ�ڶ�������
		*/
		template<class _tDeleter, typename _tDefault = void>
		using unique_ptr_from = unique_ptr<leo::defer_element<
			leo::unique_ptr_pointer<void, _tDeleter>, _tDefault>, _tDeleter>;
		using limpl(std)::weak_ptr;

		using leo::lref;

		//@{
		using leo::nptr;
		using leo::tidy_ptr;
		using leo::pointer_iterator;
		//@}

	} // inline namespace references;

} // namespace platform;

namespace platform_ex
{

	using namespace platform::references;

} // namespace platform_ex;

#endif