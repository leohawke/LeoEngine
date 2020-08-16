#pragma once

namespace leo::threading {
	class auto_reset_event
	{
	public:
		auto_reset_event(bool initiallySet = false);

		~auto_reset_event();

		void set();

		void wait();

	private:
		void* event;
	};
}