/*! \file Core\Threading\JobDispatcher.h
\ingroup LeoEngine
\brief Job被调度到Worker中进行执行
*/
#ifndef LECT_JOBDISPATCHER_HPP_
#define LECT_JOBDISPATCHER_HPP_ 1

#include <LBase/linttype.hpp>

namespace LeoEngine::Worker {
	using namespace leo::inttype;
	using workerid = uint32;

	/*
	JobDispatcher:
	1.知道有多个Worker
	*/
	class JobDispatcher {
		virtual ~JobDispatcher();

		virtual uint32 GetNumWorkers() const = 0;

		virtual uint32 GetWorkerId() const = 0;
	};
}

#endif