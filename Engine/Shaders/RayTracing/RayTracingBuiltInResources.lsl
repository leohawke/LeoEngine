(RayTracing
	(shader 
		"
		struct HitGroupSystemRootConstants
		{
			// Config is a bitfield:
			// uint IndexStride  : 8; // Can be just 1 bit to indicate 16 or 32 bit indices
			// uint VertexStride : 8; // Can be just 2 bits to indicate float3, float2 or half2 format
			// uint Unused       : 16;
			uint Config;

			// Offset into HitGroupSystemIndexBuffer
			uint IndexBufferOffsetInBytes;

			// User-provided constant assigned to the hit group
			uint UserData;

			// Padding to ensure that root parameters are properly aligned to 8-byte boundary
			uint Unused;

			// Helper functions

			uint GetIndexStride()
			{
				return Config & 0xFF;
			}

			uint GetVertexStride()
			{
				return (Config >> 8) & 0xFF;
			}
		};
		"
	)
	(ByteAddressBuffer HitGroupSystemIndexBuffer (register 0 (space RAY_TRACING_REGISTER_SPACE_SYSTEM)))
	(ByteAddressBuffer HitGroupSystemVertexBuffer (register 1 (space RAY_TRACING_REGISTER_SPACE_SYSTEM))
	(ConstantBuffer  (elemtype HitGroupSystemRootConstants) HitGroupSystemRootConstants (register 0 (space RAY_TRACING_REGISTER_SPACE_SYSTEM))
)