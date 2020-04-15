#pragma once

#include <Engine/Render/BuiltInShader.h>
#include <Engine/Render/IGraphicsBuffer.hpp>
#include <Engine/Render/IDevice.h>

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

	class WrtieToSliceVS : public Render::BuiltInShader
	{
		EXPORTED_SHADER_TYPE(WrtieToSliceVS);

		//CompilerFlags.Add( CFLAG_VertexToGeometryShader );

		//SetParameters

	private:
		//ShaderParameter UVScaleBias;
		//ShaderParameter MinZ;
	};

	class WriteToSliceGS : public Render::BuiltInShader
	{
		EXPORTED_SHADER_TYPE(WriteToSliceGS);

		//SetParameters
	private:
		//ShaderParameter MinZ;
	};

	std::shared_ptr<Render::GraphicsBuffer> GVolumeRasterizeVertexBuffer();


	//todo:ScreenRendering.h
	Render::VertexDeclarationElements GScreenVertexDeclaration();
}