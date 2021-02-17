#pragma once
#ifndef LE_RENDER_ShaderCore_h
#define LE_RENDER_ShaderCore_h 1

#include <LBase/linttype.hpp>
#include <LBase/lmathtype.hpp>
#include <LBase/cformat.h>
#include <optional>

namespace platform::Render {

	class Texture;

	inline namespace Shader
	{
		using leo::uint16;
		using leo::uint32;
		using leo::int32;
		using leo::uint8;

		struct ShaderCodeResourceCounts
		{
			uint16 NumSamplers = 0;
			uint16 NumSRVs = 0;
			uint16 NumUAVs = 0;
			uint16 NumCBs = 0;
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

		enum ShaderBaseType
		{
			SBT_INVALID,

			SBT_SRV,
			SBT_UAV,
			SBT_SAMPLER,
			SBT_CBUFFER,

			SBT_BOOL,

			SBT_INT32,
			SBT_UINT32,
			SBT_FLOAT32,

			SBT_ELEMTYPE = 1<<15,
		};

		template<uint8 row, uint8 col>
		constexpr uint8 MakeFormatRowCol = (row << 4) | col;

		template<uint16 ch0,uint8 row,uint8 col,uint8 index>
		constexpr uint32 MakeShaderFormat = (ch0 << 16) | (MakeFormatRowCol<row, col> << 8) | index;

		enum ShaderParamType :uint32
		{
			SPT_texture1D = MakeShaderFormat<SBT_SRV| SBT_ELEMTYPE,1,1,0>,
			SPT_texture2D,
			SPT_texture3D,
			SPT_textureCUBE,
			SPT_texture1DArray,
			SPT_texture2DArray,
			SPT_texture3DArray,
			SPT_textureCUBEArray,
			SPT_ConstantBuffer = MakeShaderFormat<SBT_CBUFFER| SBT_ELEMTYPE, 1, 1, 0>,
			SPT_buffer = MakeShaderFormat<SBT_SRV| SBT_ELEMTYPE, 1, 1, 8>,
			SPT_StructuredBuffer,
			SPT_rwbuffer = MakeShaderFormat<SBT_UAV| SBT_ELEMTYPE, 1, 1, 0>,
			SPT_rwstructured_buffer,
			SPT_rwtexture1D,
			SPT_rwtexture2D,
			SPT_rwtexture3D,
			SPT_rwtexture1DArray,
			SPT_rwtexture2DArray,
			SPT_AppendStructuredBuffer,
			SPT_ConsumeStructuredBuffer = MakeShaderFormat<SBT_SRV| SBT_ELEMTYPE, 1, 1, 10>,
			SPT_byteAddressBuffer = MakeShaderFormat<SBT_SRV , 1, 1, 10>,
			SPT_rwbyteAddressBuffer = MakeShaderFormat<SBT_UAV, 1, 1, 8>,
			SPT_RaytracingAccelerationStructure = MakeShaderFormat<SBT_SRV, 1, 1, 12>,
			SPT_sampler = MakeShaderFormat<SBT_SAMPLER, 1, 1, 0>,
			SPT_shader = MakeShaderFormat<SBT_INVALID, 1, 1, 0>,
			SPT_bool = MakeShaderFormat<SBT_BOOL, 1, 1, 0>,
			SPT_string = MakeShaderFormat<SBT_INVALID, 1, 1, 1>,
			SPT_uint  = MakeShaderFormat<SBT_UINT32, 1, 1, 0>,
			SPT_uint2 = MakeShaderFormat<SBT_UINT32, 1, 2, 0>,
			SPT_uint3 = MakeShaderFormat<SBT_UINT32, 1, 3, 0>,
			SPT_uint4 = MakeShaderFormat<SBT_UINT32, 1, 4, 0>,
			SPT_int   = MakeShaderFormat<SBT_INT32, 1, 1, 0>,
			SPT_int2  = MakeShaderFormat<SBT_INT32, 1, 2, 0>,
			SPT_int3  = MakeShaderFormat<SBT_INT32, 1, 3, 0>,
			SPT_int4  = MakeShaderFormat<SBT_INT32, 1, 4, 0>,
			SPT_float = MakeShaderFormat<SBT_FLOAT32, 1, 1, 0>,
			SPT_float2 = MakeShaderFormat<SBT_FLOAT32, 1, 2, 0>,
			SPT_float2x2 = MakeShaderFormat<SBT_FLOAT32, 2, 2, 0>,
			SPT_float2x3 = MakeShaderFormat<SBT_FLOAT32, 2, 3, 0>,
			SPT_float2x4 = MakeShaderFormat<SBT_FLOAT32, 2, 4, 0>,
			SPT_float3 = MakeShaderFormat<SBT_FLOAT32, 1, 3, 0>,
			SPT_float3x2 = MakeShaderFormat<SBT_FLOAT32, 3, 2, 0>,
			SPT_float3x3 = MakeShaderFormat<SBT_FLOAT32, 3, 3, 0>,
			SPT_float3x4 = MakeShaderFormat<SBT_FLOAT32, 3, 4, 0>,
			SPT_float4 = MakeShaderFormat<SBT_FLOAT32, 1, 4, 0>,
			SPT_float4x2 = MakeShaderFormat<SBT_FLOAT32, 4, 2, 0>,
			SPT_float4x3 = MakeShaderFormat<SBT_FLOAT32, 4, 3, 0>,
			SPT_float4x4 = MakeShaderFormat<SBT_FLOAT32, 4, 4, 0>,

			SPT_ElemEmpty = MakeShaderFormat<SBT_INVALID, 1, 1, 2>,
		};

		constexpr ShaderBaseType GetBaseType(ShaderParamType type)
		{
			return static_cast<ShaderBaseType>((type >> 16) & ~SBT_ELEMTYPE);
		}

		constexpr bool IsElemType(ShaderParamType type)
		{
			return static_cast<ShaderBaseType>(type >> 16) & SBT_ELEMTYPE;
		}

		constexpr bool IsNumberType(ShaderParamType type)
		{
			auto base_type = GetBaseType(type);

			return base_type == SBT_INT32 || base_type == SBT_UINT32 || base_type == SBT_FLOAT32;
		}

		constexpr uint8 GetNumRows(ShaderParamType type)
		{
			return ((type >> 8) & 0XF0) >> 4;
		}

		constexpr uint8 GetNumColumns(ShaderParamType type)
		{
			return ((type >> 8) & 0X0F);
		}

		struct RayTracingShaderInfo
		{
			std::string EntryPoint;
			std::string AnyHitEntryPoint;
			std::string IntersectionEntryPoint;
		};

		enum class ShaderParamClass :uint8_t
		{
			LooseData,
			UniformBuffer,
			Sampler,
			SRV,
			UAV,

			Num
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
					uint16_t size;
				};
				std::vector<VariableInfo> var_desc;

				std::string name;
				size_t name_hash;
				uint16_t size;
				uint32_t bind_point;
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
		struct ParameterAllocation
		{
			uint16 BufferIndex;
			uint16 BaseIndex;
			uint16 Size;
			ShaderParamClass Class = ShaderParamClass::Num;
		};

		class ShaderParameterMap
		{
		public:
			void AddParameterAllocation(const std::string& ParameterName, uint16 BufferIndex, uint16 BaseIndex, uint16 Size, ShaderParamClass ParameterType);

			bool FindParameterAllocation(const std::string& ParameterName, uint16& OutBufferIndex, uint16& OutBaseIndex, uint16& OutSize) const;

		private:
			std::unordered_map<std::string, ParameterAllocation> ParameterMap;
		};

		struct FShaderCompilerEnvironment
		{
			void SetDefine(const char* Name, const char* Value)
			{
				Definitions.emplace(Name, Value);
			}
			void SetDefine(const char* Name, bool Value)
			{
				SetDefine(Name, Value ? 1 : 0);
			}
			void SetDefine(const char* Name, int32 Value)
			{
				SetDefine(Name, leo::sfmt("%d", Value).c_str());
			}

			const std::unordered_map<std::string, std::string>& GetDefinitions() const
			{
				return Definitions;
			}
		private:
			std::unordered_map<std::string, std::string> Definitions;
		};
	}
}

#endif