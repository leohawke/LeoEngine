#pragma once
#ifndef LE_RENDER_ShaderCore_h
#define LE_RENDER_ShaderCore_h 1

#include <LBase/linttype.hpp>
#include <LBase/lmemory.hpp>
#include <LBase/lmathtype.hpp>
#include <optional>

namespace platform::Render {

	class Texture;

	inline namespace Shader
	{
		struct ShaderCodeResourceCounts
		{
			leo::uint16 NumSamplers = 0;
			leo::uint16 NumSRVs = 0;
			leo::uint16 NumUAVs = 0;
			leo::uint16 NumCBs;
		};

		enum ShaderType : leo::uint8
		{
			VertexShader,
			PixelShader,
			HullShader,
			DomainShader,
			GeometryShader,
			VisibilityAll = 5,
			ComputeShader = 5,

			NumStandardType = 6,

			RayGen = 6,
			RayMiss,
			RayHitGroup,
			RayCallable
		};


		using ShaderBlob = std::pair<std::unique_ptr<stdex::byte[]>, std::size_t>;

		enum ShaderParamType
		{
			SPT_texture1D,
			SPT_texture2D,
			SPT_texture3D,
			SPT_textureCUBE,
			SPT_texture1DArray,
			SPT_texture2DArray,
			SPT_texture3DArray,
			SPT_textureCUBEArray,
			SPT_ConstatnBuffer,
			SPT_buffer,
			SPT_StructuredBuffer,
			SPT_rwbuffer,
			SPT_rwstructured_buffer,
			SPT_rwtexture1D,
			SPT_rwtexture2D,
			SPT_rwtexture3D,
			SPT_rwtexture1DArray,
			SPT_rwtexture2DArray,
			SPT_AppendStructuredBuffer,
			SPT_ConsumeStructuredBuffer,
			SPT_byteAddressBuffer,
			SPT_rwbyteAddressBuffer,
			SPT_RaytracingAccelerationStructure,
			SPT_sampler,
			SPT_shader,
			SPT_bool,
			SPT_string,
			SPT_uint,
			SPT_uint2,
			SPT_uint3,
			SPT_uint4,
			SPT_int,
			SPT_int2,
			SPT_int3,
			SPT_int4,
			SPT_float,
			SPT_float2,
			SPT_float2x2,
			SPT_float2x3,
			SPT_float2x4,
			SPT_float3,
			SPT_float3x2,
			SPT_float3x3,
			SPT_float3x4,
			SPT_float4,
			SPT_float4x2,
			SPT_float4x3,
			SPT_float4x4,

			SPT_ElemEmpty,
		};

		struct RayTracingShaderInfo
		{
			std::string EntryPoint;
			std::string AnyHitEntryPoint;
			std::string IntersectionEntryPoint;
		};

		struct ShaderInfo {
			ShaderType Type;

			ShaderInfo(ShaderType t);

			struct ConstantBufferInfo
			{
				struct VariableInfo
				{
					std::string name;
					uint32_t start_offset;
					uint8_t type;
					uint8_t rows;
					uint8_t columns;
					uint16_t elements;
				};
				std::vector<VariableInfo> var_desc;

				std::string name;
				size_t name_hash;
				uint32_t size = 0;
			};
			std::vector<ConstantBufferInfo> ConstantBufferInfos;

			struct BoundResourceInfo
			{
				std::string name;
				uint8_t type;
				uint8_t dimension;
				uint16_t bind_point;
			};
			std::vector<BoundResourceInfo> BoundResourceInfos;

			ShaderCodeResourceCounts ResourceCounts;

			std::optional<size_t> InputSignature = std::nullopt;
			std::optional<leo::math::data_storage<leo::uint16, 3>> CSBlockSize = std::nullopt;

			std::optional<RayTracingShaderInfo> RayTracingInfos = std::nullopt;
		};

		using ShaderMacro = std::pair<std::string, std::string>;
	}



}

#endif