#include "LMessage.h"

namespace leo
{
	namespace Messaging
	{

		bool
			operator==(const Message& x, const Message& y)
		{
			return x.id == y.id && x.content == y.content;
		}

		void
			swap(Message& x, Message& y) lnothrow
		{
			std::swap(x.id, y.id),
				swap(x.content, y.content);
		}


		void
			MessageQueue::Merge(MessageQueue& mq)
		{
			std::for_each(mq.begin(), mq.end(), [this](decltype(*mq.begin()) pr) {
				if (pr.second)
					insert(std::move(pr));
			});
			mq.clear();
		}

		void
			MessageQueue::Peek(Message& msg) const
		{
			if (!empty())
				msg = cbegin()->second;
		}

		void
			MessageQueue::Pop()
		{
			if (!empty())
				erase(cbegin());
		}

		void
			MessageQueue::Push(const Message& msg, Priority prior)
		{
			if (msg)
				emplace(prior, msg);
		}
		void
			MessageQueue::Push(Message&& msg, Priority prior)
		{
			if (msg)
				emplace(prior, std::move(msg));
		}

		void
			MessageQueue::Remove(Priority p)
		{
			erase(upper_bound(p), end());
		}


		ImplDeDtor(MessageException)


			ImplDeDtor(MessageSignal)

	} // namespace Messaging;
} // namespace leo;

