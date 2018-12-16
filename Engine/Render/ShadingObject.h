#pragma once

#ifndef LE_RENDER_ShadingObject_h
#define LE_RENDER_ShadingObject_h 1

#include <LBase/lmacro.h>
#include <LBase/lmath.hpp>

namespace LeoEngine::Render {
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