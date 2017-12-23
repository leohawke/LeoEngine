/*!	\file Platform.h
\ingroup Framework
\brief ƽ̨��صĹ�������޹غ�����궨�弯�ϡ�
*/

#ifndef FrameWork_Common_h
#define FrameWork_Common_h 1

#include <LFramework/LCLib/Platform.h>
#include <LBase/functional.hpp>
#include <LBase/cassert.h>
#include <LBase/cwctype.h>
#include <LBase/cstring.h>
#include <LBase/lmacro.h>

namespace platform
{

	/*!
	\ingroup Platforms
	*/
	//@{
	using ID = std::uintmax_t;

	template<ID _vN>
	using MetaID = leo::integral_constant<ID, _vN>;

	//! \brief ƽ̨��ʶ�Ĺ���������ͣ�ָ������ƽ̨��
	struct IDTagBase
	{};

#if LB_IMPL_CLANGPP
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

	//! \brief ����ƽ̨��ʶ�Ĺ���������ͣ�ָ����������ƽ̨��
	struct HostedIDTagBase : virtual IDTagBase
	{};

	//! \brief ƽ̨��ʶ������������͡�
	template<ID _vN>
	struct IDTag : virtual IDTagBase, virtual MetaID<_vN>
	{};

	//! \brief ָ���ض��������͵� IDTag �ػ���
#define LFL_IDTag_Spec(_n, _base) \
	template<> \
	struct IDTag<LF_Platform_##_n> : virtual _base, \
		virtual MetaID<LF_Platform_##_n> \
	{};

	//! \brief ָ������ƽ̨�� IDTag �ػ���
#define LFL_IDTag_SpecHost(_n) \
	LFL_IDTag_Spec(_n, HostedIDTagBase)

	//! \brief ָ�������ض�ƽ̨�� IDTag �ػ���
#define LFL_IDTag_SpecOnBase(_n, _b) \
	LFL_IDTag_Spec(_n, IDTag<LF_Platform_##_b>)

	LFL_IDTag_SpecHost(Win32)
		LFL_IDTag_SpecOnBase(Win64, Win32)
		LFL_IDTag_SpecOnBase(MinGW32, Win32)
		LFL_IDTag_SpecOnBase(MinGW64, Win64)
		//LFL_IDTag_SpecHost(Linux)
		//LFL_IDTag_SpecOnBase(Linux_x86, Linux)
		//LFL_IDTag_SpecOnBase(Linux_x64, Linux)
		//LFL_IDTag_SpecOnBase(Android, Linux)
		//LFL_IDTag_SpecOnBase(Android_ARM, Android)

		//! \brief ȡƽ̨��ʶ��������Ǽ������͡�
		template<ID... _vN>
	struct IDTagSet : virtual IDTag<_vN>...
	{};
	//@}

#if LB_IMPL_CLANGPP
#	pragma GCC diagnostic pop
#endif

#define LCL_Tag_constfn inline


	/*!
	\ingroup PlatformEmulation
	\brief ��������ƽ̨ģ��Ĵ���ģ�塣
	*/
#define LCL_DefPlatformFwdTmpl(_n, _fn) \
	DefFwdTmplAuto(_n, _fn(platform::IDTag<LF_Platform>(), lforward(args)...))


	/*!
	\brief ���������ں�ʽ���õĸ����ꡣ
	\sa leo::is_detected
	*/
	//@{
#define LCL_CheckDecl_t(_fn) CheckDecl##_fn##_t
#define LCL_DeclCheck_t(_fn, _call) \
	template<typename... _tParams> \
	using LCL_CheckDecl_t(_fn) \
		= decltype(_call(std::declval<_tParams&&>()...));
	//@}


	/*!
	\brief �쳣��ֹ������
	*/
	LB_NORETURN LB_API void
		terminate() lnothrow;


	/*!
	\brief ƽ̨�����ռ䡣
	*/
	namespace Descriptions
	{

		/*!
		\brief ��¼�ȼ���
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


	  /*!
	  \brief ���Ĭ��������ָ���ַ��Ƿ�Ϊ�ɴ�ӡ�ַ���
	  \note MSVCRT �� isprint/iswprint ʵ��ȱ�ݵı�ͨ��
	  \sa https://connect.microsoft.com/VisualStudio/feedback/details/799287/isprint-incorrectly-classifies-t-as-printable-in-c-locale
	  */
	  //@{
	inline PDefH(bool, IsPrint, char c)
		ImplRet(leo::isprint(c))
		inline PDefH(bool, IsPrint, wchar_t c)
		ImplRet(stdex::iswprint(c))
		template<typename _tChar>
	bool
		IsPrint(_tChar c)
	{
		return platform::IsPrint(wchar_t(c));
	}
	//@}


	//@{
	inline PDefH(leo::uchar_t*, ucast, wchar_t* p) lnothrow
		ImplRet(leo::replace_cast<leo::uchar_t*>(p))
		inline PDefH(const leo::uchar_t*, ucast, const wchar_t* p) lnothrow
		ImplRet(leo::replace_cast<const leo::uchar_t*>(p))
		template<typename _tChar>
	_tChar*
		ucast(_tChar* p) lnothrow
	{
		return p;
	}

	inline PDefH(wchar_t*, wcast, leo::uchar_t* p) lnothrow
		ImplRet(leo::replace_cast<wchar_t*>(p))
		inline PDefH(const wchar_t*, wcast, const leo::uchar_t* p) lnothrow
		ImplRet(leo::replace_cast<const wchar_t*>(p))
		template<typename _tChar>
	_tChar*
		wcast(_tChar* p) lnothrow
	{
		return p;
	}
	//@}

	/*!
	\brief ���ò������쳣��
	*/
	template<typename _func, typename... _tParams>
	leo::invoke_result_t<_func(_tParams&&...)>
		CallNothrow(const leo::invoke_result_t<_func(_tParams&&...)>& v, _func f,
			_tParams&&... args) lnothrowv
	{
		TryRet(f(lforward(args)...))
			CatchExpr(std::bad_alloc&, errno = ENOMEM)
			CatchIgnore(...)
			return v;
	}

	/*!
	\brief ѭ���ظ�������
	*/
	template<typename _func, typename _tErrorRef,
		typename _tError = leo::decay_t<_tErrorRef>,
		typename _type = leo::result_of_t<_func&()>>
		_type
		RetryOnError(_func f, _tErrorRef&& err, _tError e = _tError())
	{
		return leo::retry_on_cond([&](_type res) {
			return res < _type() && _tError(err) == e;
		}, f);
	}

	template<typename _func, typename _type = leo::result_of_t<_func&()>>
	inline _type
		RetryOnInterrupted(_func f)
	{
		return RetryOnError(f, errno, EINTR);
	}

	/*!
	\brief ִ�� UTF-8 �ַ����Ļ������
	\note ʹ�� std::system ʵ�֣�������Ϊ����� std::system ��Ϊһ�¡�
	*/
	LB_API int
		usystem(const char*);



	//@{
	/*!
	\brief ϵͳ����ѡ�
	\note �� Max ��ʼ�����Ʊ�ʾ�ɾ��е����ֵ��
	*/
	enum class SystemOption
	{
		/*!
		\brief �ڴ�ҳ���ֽ�����
		\note 0 ��ʾ��֧�ַ�ҳ��
		*/
		PageSize,
		//@{
		//! \brief һ�������пɾ��е��ź��������������
		MaxSemaphoreNumber,
		//! \brief �ɾ��е��ź��������ֵ��
		MaxSemaphoreValue,
		//@}
		//! \brief ��·�������з������ӿɱ��ɿ���������������
		MaxSymlinkLoop
	};


	/*!
	\brief ȡ�������á�
	\return ѡ�������ֵ�ܱ��������ͱ�ʾʱΪת����Ķ�Ӧѡ��ֵ������ΪĬ�ϳ���ֵ��
	\note ���� LB_STATELESS ���Σ����Ҳ�����ʱ������ʹ�ø���������档
	\note ���� LB_STATELESS ���Σ����Ҳ�����ʱ���������� errno Ϊ EINVAL ��
	\sa SystemOption
	\sa TraceDe

	������ѡ��ֵ��Ϊ������ָ����ѯ�ض�������ֵ��
	ĳЩƽ̨���ý�����Ƿ���ʱȷ���ĳ��������ô˺����޸����ã�ʹ�� LB_STATELESS ���Σ�
	����ƽ̨����������ʱȷ����ͨ����ѯ������������������ʱ�ṩ�Ľӿڣ�
	ȷ��ѡ���Ƿ���ڣ��Լ���������õ�ֵ��
	��ѡ����ڣ��Ҷ�Ӧ��ֵ���� size_t ��ʾ���򷵻�ѡ���Ӧ��ֵ��
	���򣬷���һ��Ĭ�ϵĳ���ֵ��Ϊ���ˣ���ʾ���ض��������ܿɿ�������Ĭ�����ơ�
	ע�����ֵ����ƽ̨��أ�
	���� POSIX �ṩ��Сֵ�ҷ���ѡ�������Σ�ʹ�� POSIX ��Сֵ����������ֲ�ԡ�
	����ֵ���ܲ�����ʵ��ʵ��֧�ֵ�ֵ��
	�Ա�ʾ�ɾ��е����ֵ�����ƣ�����ֵΪ \c size_t(-1) ����ʱʵ��ʵ��֧�ֵ�ֵ���ܳ���
	size_t �ı�ʾ��Χ����û��ָ����ʽ���ƣ����ܴ洢���ַ�ռ����ƣ���
	��������ֵȡ����ѡ��ľ��庬�塣
	*/
	LF_API
		size_t
		FetchLimit(SystemOption) lnothrow;
	//@}

} // namespace platform;

#endif