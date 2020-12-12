(effect
    (include SSD/SSDDefinitions.h)
    (texture2D SignalInput_Textures_0)
    (RWTexture2D (elemtype uint) SignalOutput_UAVs_0)
    (sampler point_sampler
        (filtering min_mag_mip_point)
		(address_u clamp)
		(address_v clamp)
    )
    (float4 ThreadIdToBufferUV)
    (float4 BufferBilinearUVMinMax)
    (float2 BufferUVToOutputPixelPosition)
    (float HitDistanceToWorldBluringRadius)
(shader
"
static const float DENOISER_INVALID_HIT_DISTANCE = -2.0;

static const float WORLD_RADIUS_INVALID = -1;

#define TILE_PIXEL_SIZE 8

uint2 BufferUVToBufferPixelCoord(float2 SceneBufferUV)
{
	return uint2(SceneBufferUV * BufferUVToOutputPixelPosition);
}

float SafeRcp(float x,float d= 0.0)
{
    return x > 0.0 ? rcp(x) : d;
}


[numthreads(TILE_PIXEL_SIZE,TILE_PIXEL_SIZE,1)]
void MainCS(
    uint2 DispatchThreadId : SV_DispatchThreadID,
	uint2 GroupId : SV_GroupID,
	uint2 GroupThreadId : SV_GroupThreadID,
	uint GroupThreadIndex : SV_GroupIndex
)
{
    // Find out scene buffer UV.
	float2 SceneBufferUV = DispatchThreadId * ThreadIdToBufferUV.xy + ThreadIdToBufferUV.zw;
	if (true)
	{
		SceneBufferUV = clamp(SceneBufferUV, BufferBilinearUVMinMax.xy, BufferBilinearUVMinMax.zw);
	}

    float4 Channels = SignalInput_Textures_0.SampleLevel(point_sampler,SceneBufferUV,0);

    float SampleCount = (Channels.g == -2.0 ? 0.0 : 1.0);
    float MissCount = SampleCount * Channels.r;
    float ClosestHitDistance = SampleCount * (Channels.g == DENOISER_MISS_HIT_DISTANCE ? 1000000 : Channels.g);

    float WorldBluringRadius = HitDistanceToWorldBluringRadius * ClosestHitDistance;

    if(MissCount != 0)
        WorldBluringRadius = WORLD_RADIUS_MISS;
    if(SampleCount == 0)
       WorldBluringRadius = WORLD_RADIUS_INVALID;

    if (SampleCount == 0)
	{
		WorldBluringRadius = -2.0;
	}
	else if (WorldBluringRadius == WORLD_RADIUS_MISS)
	{
		WorldBluringRadius = -1.0;
	}

    float MissCountRatio = MissCount * SafeRcp(SampleCount);

    uint EncodedData = MissCountRatio*255;
    EncodedData |= (f32tof16(WorldBluringRadius) << 16);

    uint2 OutputPixelPostion = BufferUVToBufferPixelCoord(SceneBufferUV);

    SignalOutput_UAVs_0[OutputPixelPostion] = EncodedData;
}
")
)