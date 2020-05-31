#pragma once

#include <Engine/Render/BuiltInShader.h>
#include <Engine/Render/IGraphicsBuffer.hpp>
#include <Engine/Render/IDevice.h>
#include <Engine/Render/ICommandList.h>
#include <Engine/Render/ShaderParamterTraits.hpp>

namespace platform
{
	using leo::int32;

	/** Represents a subregion of a volume texture. */
	struct VolumeBounds
	{
		int32 MinX, MinY, MinZ;
		int32 MaxX, MaxY, MaxZ;

		VolumeBounds() :
			MinX(0),
			MinY(0),
			MinZ(0),
			MaxX(0),
			MaxY(0),
			MaxZ(0)
		{}

		VolumeBounds(int32 Max) :
			MinX(0),
			MinY(0),
			MinZ(0),
			MaxX(Max),
			MaxY(Max),
			MaxZ(Max)
		{}

		bool IsValid() const
		{
			return MaxX > MinX && MaxY > MinY && MaxZ > MinZ;
		}
	};

	class WriteToSliceVS : public Render::BuiltInShader
	{
	public:
		BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
			SHADER_PARAMETER(leo::math::float4, UVScaleBias)
			SHADER_PARAMETER(int, MinZ)
		END_SHADER_PARAMETER_STRUCT();

		EXPORTED_BUILTIN_SHADER(WriteToSliceVS);
	public:

		//CompilerFlags.Add( CFLAG_VertexToGeometryShader );

		template<class CommandList>
		void SetParameters(CommandList&, const VolumeBounds& bounds, leo::math::int3 Resolution)
		{

		}

	};

	class WriteToSliceGS : public Render::BuiltInShader
	{
		EXPORTED_BUILTIN_SHADER(WriteToSliceGS);

		//SetParameters
	private:
		//ShaderParameter MinZ;
	};

	std::shared_ptr<Render::GraphicsBuffer> GVolumeRasterizeVertexBuffer();


	//todo:ScreenRendering.h
	Render::VertexDeclarationElements GScreenVertexDeclaration();

	void RasterizeToVolumeTexture(Render::CommandList& CmdList, VolumeBounds VolumeBounds);
}