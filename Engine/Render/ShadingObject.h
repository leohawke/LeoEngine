#pragma once

#ifndef LE_RENDER_ShadingObject_h
#define LE_RENDER_ShadingObject_h 1

#include <LBase/lmacro.h>
#include <LBase/lmath.hpp>

namespace LeoEngine::Render {
	enum ShadingListID {
		ESLIST_INTERNAL, //! internally used.
		EFLIST_PREPROCESS, //!Pre-process items.
		EFLIST_GENERAL, //!Opaque/shadow passes.
		EFLIST_ZPREPASS, //!Items that are rendered into the z-prepass.

		EFLIST_CUSTOM, //!Custom scene pass.
	};

	const int EFLIST_COUNT = EFLIST_CUSTOM+1;


	namespace lm = leo::math;
	class lalignas(16) ShadingObject {
	public:
		struct InstanceInfo {
			lm::float4x4 matrix;
		};
	public:
		void SetMatrix(const lm::float4x4& matrix) {
			instance_info.matrix = matrix;
		}

		const lm::float4x4& GetMatrix(){
			return instance_info.matrix;
		}
	protected:
		InstanceInfo instance_info;
	};
}

#endif