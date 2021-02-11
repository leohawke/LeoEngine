#include "RenderObject.h"

using namespace platform::Render;

std::stack<RObject*> RObject::PendingDeletes;
leo::uint32 RObject::CurrentFrame = 0;
std::vector<RObject::RObjectToDelete> RObject::DeferredDeletionQueue;

void RObject::FlushPendingDeletes()
{
	auto Delete = [](std::vector<RObject*>& ToDelete)
	{
		for (auto& Object : ToDelete)
		{
			delete Object;
		}
	};

	while (!PendingDeletes.empty())
	{
		DeferredDeletionQueue.emplace_back(CurrentFrame);

		while (!PendingDeletes.empty())
		{
			DeferredDeletionQueue.back().Objects.push_back(PendingDeletes.top());
			PendingDeletes.pop();
		}
	}

	const auto NumFramesToExpire = 3;

	if (!DeferredDeletionQueue.empty())
	{
		auto DeletedBatchCount = 0;
		while (DeletedBatchCount < DeferredDeletionQueue.size())
		{
			auto& ObjectBatch = DeferredDeletionQueue[DeletedBatchCount];

			if (((ObjectBatch.FrameDeleted + NumFramesToExpire) < CurrentFrame))
			{
				Delete(ObjectBatch.Objects);
				++DeletedBatchCount;
			}
			else
			{
				break;
			}
		}

		DeferredDeletionQueue.erase(DeferredDeletionQueue.begin(), DeferredDeletionQueue.begin() + DeletedBatchCount);

		++CurrentFrame;
	}

}

platform::Render::RObject::~RObject()
{
}
