/*! \file Engine\Render\Effect\Effect.hpp
\ingroup Engine\Render\Effect
\brief Effect¹«¹²¼¯¡£
*/
#ifndef LE_RENDER_EFFECT_h
#define LE_RENDER_EFFECT_h 1

#include <LBase/linttype.hpp>
#include <vector>

namespace platform::Render {
	class PipleState {

	};

	class ShaderCompose {
	public:
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};
}

namespace platform::Render::Effect {
	class Effect {
	public:
		ShaderCompose& GetShader(leo::uint8 index);
	private:
		std::vector<ShaderCompose> shaders;
	};

	class Pass {
	public :
		void Bind(Effect &);
		ShaderCompose& GetShader(Effect&);
	private:
		PipleState state;
		leo::uint8 bind_index;
	};

	class Technique {
	public:
		Pass& GetPass(leo::uint8 index);
	private:
		std::vector<Pass> passes;
	};

}

#endif