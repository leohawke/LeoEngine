#include "LShell.h"
#include <LFramework/Helper/GUIApplication.h>

namespace leo
{

	namespace Shells
	{

		using namespace Messaging;

		Shell::~Shell()
		{
			// TODO: Reconsider of using weak pointer in messages.
			//	FetchAppInstance().Queue.Remove(this, 0xFF);
		}

		bool
			Shell::IsActive() const
		{
			// NOTE: %get_raw used here for performance.
			return get_raw(FetchAppInstance().GetShellHandle()) == this;
		}

		void
			Shell::DefShlProc(const Message& msg)
		{
			switch (msg.GetMessageID())
			{
			case SM_Set:
			{
				auto h(FetchTarget<SM_Set>(msg));

				FetchAppInstance().Switch(h);
			}
			break;
			case SM_Quit:
				TraceDe(Notice, "Quit message handled.");
				std::exit(FetchTarget<SM_Quit>(msg));
			case SM_Bound:
			{
				const auto& pr(FetchTarget<SM_Bound>(msg));
				const auto h_shl(pr.first.lock());

				if (LB_LIKELY(h_shl))
					h_shl->OnGotMessage(pr.second);
			}
			break;
			case SM_Task:
				FetchTarget<SM_Task>(msg)();
			default:
				break;
			}
		}

	} // namespace Shells;

} // namespace leo;
