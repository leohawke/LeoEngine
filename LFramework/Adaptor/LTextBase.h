/*!	\file LTextBase.h
\ingroup Adaptor
\brief 外部库关联：文本接口。
*/


#ifndef Framework_Adaptor_YTextBase_H_
#define Framework_Adaptor_YTextBase_H_ 1

#include <LFramework/Adaptor/LAdaptor.h>

//包含 CHRLib 。
#include <LFramework/CHRLib/CharacterProcessing.h>

namespace leo
{
	using platform::u16string;
	//! \since build 641x
	using platform::u32string;

	namespace Text
	{
		using namespace CHRLib;
		//! \since build 512
		using platform::IsPrint;

	} // namespace Text;

	using Text::ucsint_t;

} // namespace leo;

#endif

