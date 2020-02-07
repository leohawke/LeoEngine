/*! \file exception.h
\ingroup LBase
\brief ��׼���쳣��չ�ӿڡ�
*/

#ifndef LBase_exception_h
#define LBase_exception_h 1

#include "LBase/deref_op.hpp" // for call_value_or;
#include "LBase/type_traits.hpp" // for remove_cv_t;
#include <memory> // for std::addressof;
#include <functional> // for std::mem_fn;
#include <stdexcept> // for std::logic_error;
#include <system_error> // for std::error_category, std::generic_category,
//	std::system_error, std::errc;

namespace leo
{
	/*!	\defgroup exceptions Exceptions
	\brief �쳣���͡�
	\since build 1.4
	*/


	/*!
	\brief ���������쳣��
	\since build 1.4
	*/
	template<typename _func>
	void
		iterate_exceptions(_func&& f, std::exception_ptr p = std::current_exception())
	{
		while (p)
			p = lforward(f)(p);
	}

	/*!
	\brief ȡǶ���쳣ָ�롣
	\return ������ std::rethrow_if_nested �׳��쳣�������򷵻��쳣ָ�룬����Ϊ�ա�
	\since build 1.4
	*/
	template<class _tEx>
	inline std::exception_ptr
		get_nested_exception_ptr(const _tEx& e)
	{
		return leo::call_value_or(std::mem_fn(&std::nested_exception::nested_ptr),
			dynamic_cast<const std::nested_exception*>(std::addressof(e)));
	}

	/*!
	\brief �����쳣������ǰ�������ڴ�����쳣���׳�Ƕ���쳣�������׳��쳣��
	\since build 1.4
	*/
	template<typename _type>
	LB_NORETURN inline void
		raise_exception(_type&& e)
	{
		if (std::current_exception())
			std::throw_with_nested(lforward(e));
		throw lforward(e);
	}


	/*!
	\brief ����Ƕ���쳣��
	\since build 1.4
	*/
	template<typename _tEx, typename _func,
		typename _tExCaught = remove_cv_t<_tEx>&>
		void
		handle_nested(_tEx& e, _func&& f)
	{
		try
		{
			std::rethrow_if_nested(e);
		}
		catch (_tExCaught ex)
		{
			lforward(f)(ex);
		}
	}


	/*!
	\since build 1.3
	\ingroup exceptions
	*/
	//@{
	//! \brief �쳣����֧�ֵĲ�����
	class LB_API unsupported : public std::logic_error
	{
	public:
		unsupported()
			: logic_error("Unsupported operation found.")
		{}
		//! \since build 1.4
		using logic_error::logic_error;
		//! \since build 1.4
		unsupported(const unsupported&) = default;
		/*!
		\brief ���������ඨ����Ĭ��ʵ�֡�
		\since build 1.4
		*/
		~unsupported() override;
	};


	//! \brief �쳣��δʵ�ֵĲ�����
	class LB_API unimplemented : public unsupported
	{
	public:
		unimplemented()
			: unsupported("Unimplemented operation found.")
		{}
		//! \since build 1.3
		using unsupported::unsupported;
		//! \since build 1.4
		unimplemented(const unimplemented&) = default;
		/*!
		\brief ���������ඨ����Ĭ��ʵ�֡�
		\since build 1.4
		*/
		~unimplemented() override;
	};
	//@}

	/*!
	\brief �쳣���޷���ʾ��ֵ/ö�ٷ�Χ�ڵ�ת����
	*/
	class LB_API narrowing_error : public std::logic_error
	{
	public:
		narrowing_error()
			: logic_error("Norrowing found.")
		{}
		using logic_error::logic_error;
		narrowing_error(const narrowing_error&) = default;
		//! \brief ���������ඨ����Ĭ��ʵ�֡�
		~narrowing_error() override;
	};
	//@}


	class LB_API invalid_construction : public std::invalid_argument
	{
	public:
		invalid_construction();
		invalid_construction(const invalid_construction&) = default;

		/*!
		\brief ���������ඨ����Ĭ��ʵ�֡�
		*/
		~invalid_construction() override;
	};

	/*!
	\brief �׳� invalid_construction �쳣��
	\throw invalid_construction
	\relates invalid_construction
	*/
	LB_NORETURN LB_API void
		throw_invalid_construction();

	LB_NORETURN LB_API void
		throw_out_of_range(const char* msg);

	/*!
	\brief �׳����� std::system_error ��������ͬ���캯���������쳣��
	\throw _type ʹ�ò���������쳣��
	\since build 1.4
	*/
	//@{
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(std::error_code ec, _tParams&&... args)
	{
		throw _type(ec, lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(std::error_condition cond, _tParams&&... args)
	{
		throw _type(cond.value(), cond.category(), lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(int ev, _tParams&&... args)
	{
		throw _type(ev, std::generic_category(), lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(int ev, const std::error_category& ecat, _tParams&&... args)
	{
		throw _type(ev, ecat, lforward(args)...);
	}
	//@}

} // namespace leo;
#endif