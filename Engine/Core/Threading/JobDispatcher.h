/*! \file Core\Threading\JobDispatcher.h
\ingroup LeoEngine
\brief Job�����ȵ�Worker�н���ִ��
*/
#ifndef LECT_JOBDISPATCHER_HPP_
#define LECT_JOBDISPATCHER_HPP_ 1

#include <LBase/linttype.hpp>

namespace LeoEngine::Worker {
	using namespace leo::inttype;
	using workerid = uint32;

	/*
	JobDispatcher:
	1.֪���ж��Worker
	*/
	class JobDispatcher {
		virtual ~JobDispatcher();

		virtual uint32 GetNumWorkers() const = 0;

		virtual uint32 GetWorkerId() const = 0;
	};
}

#endif