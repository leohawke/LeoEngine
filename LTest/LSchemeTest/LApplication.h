/*!	\file LApplication.h
\ingroup LTest
\brief 系统资源和应用程序实例抽象。
*/

#include <LBase/sutility.h>
#include <LBase/lmacro.h>
#include <memory>

#ifndef LTEST_LScheme_LApplication_h_
#define LTEST_LScheme_LApplication_h_ 1

namespace leo {

	namespace Shells{
	//! \brief 外壳程序：实现运行期控制流映像语义。
	class Shell : private noncopyable, public std::enable_shared_from_this<Shell>
	{
	public:
		/*!
		\brief 无参数构造。
		*/
		DefDeCtor(Shell)
			/*!
			\brief 析构。
			*/
			virtual
			~Shell();
	};
	}

	class Application :public Shells::Shell
	{
	public:
		//! \brief 无参数构造：默认构造。
		Application();

		//! \brief 析构：释放 Shell 所有权和其它资源。
		~Application() override;
	};
}


#endif