#ifndef SSDMetadata_h
#define SSDMetadata_h 1

#define MAX_COMPRESSED_METADATA_VGPRS 5

struct FSSDCompressedSceneInfos
{
	/** Raw compressed buffer, kept fully compressed to have minimal VGPR footprint. */
	//uint VGPR[MAX_COMPRESSED_METADATA_VGPRS];
};

FSSDCompressedSceneInfos CreateCompressedSceneInfos()
{
	FSSDCompressedSceneInfos CompressedInfos;
	return CompressedInfos;
}

/** olds commonly used information of the scene of a given sample. */
struct FSSDSampleSceneInfos
{
	/** Raw screen position of the sample. */
	//float2 ScreenPosition;

	/** The raw device Z. */
	//float DeviceZ;

	/** Raw pixel depth in world space. */
	//float WorldDepth;

	/** Roughness of the pixel. */
	//float Roughness;

	/** Normal of the pixel in world space. */
	//float3 WorldNormal;

	/** Normal of the pixel in view space. */
	//float3 ViewNormal;

	/** Position of the pixel in the translated world frame to save VALU. */
	//float3 TranslatedWorldPosition;
};

FSSDSampleSceneInfos CreateSampleSceneInfos()
{
	FSSDSampleSceneInfos Infos;
	//Infos.WorldDepth = 0;
	//Infos.ScreenPosition = 0;
	//Infos.Roughness = 0;
	//Infos.WorldNormal = 0;
	//Infos.ViewNormal = 0;
	//Infos.TranslatedWorldPosition = 0;
	return Infos;
}

FSSDSampleSceneInfos UncompressSampleSceneInfo(
	const uint CompressedLayout, const bool bIsPrevFrame,
	float2 ScreenPosition,
	FSSDCompressedSceneInfos CompressedInfos)
{
	FSSDSampleSceneInfos Infos = CreateSampleSceneInfos();

	return Infos;
}

FSSDCompressedSceneInfos CompressSampleSceneInfo(
	const uint CompressedLayout,
	FSSDSampleSceneInfos Infos)
{
	FSSDCompressedSceneInfos CompressedInfos = CreateCompressedSceneInfos();
	return CompressedInfos;
}

#endif