(effect
(include SSDDefinitions.h)
(shader
"
//require macro DIM_STAGE
#define SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK 0

#define STAGE_RECONSTRUCTION 0

//Policy to use to change the size of kernel.
#define SAMPLE_COUNT_POLICY_DISABLED 0

// Only output the sum of the signal 0.
#define OUTPUT_MODE_SUM 0
// Output the result of descending ring bucketing.
#define OUTPUT_MODE_DRB 2

//configs
#define TILE_PIXEL_SIZE 8
#define CONFIG_SIGNAL_PROCESSING SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
#define CONFIG_UPSCALE  0
#define CONFIG_SIGNAL_BATCH_SIZE 1


#if CONFIG_SIGNAL_PROCESSING == SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK
    #define MAX_SIGNAL_BATCH_SIZE CONFIG_SIGNAL_BATCH_SIZE
    #define SIGNAL_ARRAY_SIZE CONFIG_SIGNAL_BATCH_SIZE

    #define CONFIG_BILATERAL_PRESET BILATERAL_PRESET_MONOCHROMATIC_PENUMBRA
    #define CONFIG_MULTIPLEXED_SIGNALS_PER_SIGNAL_DOMAIN 1

#if DIM_STAGE == STAGE_RECONSTRUCTION
		// Input and output layout.
		#define CONFIG_SIGNAL_INPUT_LAYOUT  SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP
		#define CONFIG_SIGNAL_OUTPUT_LAYOUT SIGNAL_BUFFER_LAYOUT_PENUMBRA_RECONSTRUCTION

		#define CONFIG_SIGNAL_INPUT_TEXTURE_TYPE SIGNAL_TEXTURE_TYPE_UINT2
		#define CONFIG_SIGNAL_OUTPUT_TEXTURE_TYPE SIGNAL_TEXTURE_TYPE_FLOAT4
		#define CONFIG_INPUT_TEXTURE_COUNT ((CONFIG_SIGNAL_BATCH_SIZE + 1) / 2)
		#define CONFIG_OUTPUT_TEXTURE_COUNT CONFIG_SIGNAL_BATCH_SIZE

		#define CONFIG_SAMPLE_SET           SAMPLE_SET_STACKOWIAK_4_SETS
		#define CONFIG_BILATERAL_DISTANCE_COMPUTATION SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS
		#define CONFIG_MAX_WITH_REF_DISTANCE 1
		#define CONFIG_OUTPUT_MODE          OUTPUT_MODE_DRB

		#if DIM_SIGNAL_BATCH_SIZE > 1
			#define CONFIG_CLAMP_UV_PER_SIGNAL 1
		#endif
#endif
#endif

/** Changes the logic controling the number of sample to do. */
#ifndef CONFIG_SAMPLE_COUNT_POLICY
	#define CONFIG_SAMPLE_COUNT_POLICY SAMPLE_COUNT_POLICY_DISABLED
#endif

/** Selects a subset of sample of a given CONFIG_SAMPLE_SET */
#ifndef CONFIG_SAMPLE_SUBSET
	#define CONFIG_SAMPLE_SUBSET 0
#endif

/** Whether the ray tracing input may needs to be upscale to the view's resolution. */
#ifndef CONFIG_UPSCALE
	#define CONFIG_UPSCALE 0
#endif

/** Color space of the input signal. */
#ifndef CONFIG_INPUT_COLOR_SPACE
	#define CONFIG_INPUT_COLOR_SPACE STANDARD_BUFFER_COLOR_SPACE
#endif

/** Color space to use for the accumulation. */
#ifndef CONFIG_ACCUMULATION_COLOR_SPACE
	#define CONFIG_ACCUMULATION_COLOR_SPACE STANDARD_BUFFER_COLOR_SPACE
#endif

/** Whether the input signal should be normalized. */
#ifndef CONFIG_NORMALIZE_INPUT
	#define CONFIG_NORMALIZE_INPUT 0
#endif

/** The oupput mode that should be use. */
#ifndef CONFIG_OUTPUT_MODE
	#define CONFIG_OUTPUT_MODE OUTPUT_MODE_SUM
#endif

/** The number of signal that should be processed per signal domain. */
#ifndef CONFIG_MULTIPLEXED_SIGNALS_PER_SIGNAL_DOMAIN
	#define CONFIG_MULTIPLEXED_SIGNALS_PER_SIGNAL_DOMAIN SIGNAL_ARRAY_SIZE
#endif

/** Selects how the world distance should be computed for bilateral rejection. */
#ifndef CONFIG_BILATERAL_DISTANCE_COMPUTATION
	#define CONFIG_BILATERAL_DISTANCE_COMPUTATION SIGNAL_WORLD_FREQUENCY_MIN_METADATA
#endif

/** Adds a multiplier on how the distance should be computed. */
#ifndef CONFIG_BILATERAL_DISTANCE_MULTIPLIER
	#define CONFIG_BILATERAL_DISTANCE_MULTIPLIER 1.0
#endif

/** Whether neighbor bilateral distance should be maxed with reference one. */
#ifndef CONFIG_MAX_WITH_REF_DISTANCE
	#define CONFIG_MAX_WITH_REF_DISTANCE 0
#endif


// Whitelist kernel to compile.
#if CONFIG_SAMPLE_SET == SAMPLE_SET_STACKOWIAK_4_SETS
	#define COMPILE_STACKOWIAK_KERNEL 1
#endif

// White list accumulators to compile.
#if CONFIG_OUTPUT_MODE == OUTPUT_MODE_DRB
	#define COMPILE_DRB_ACCUMULATOR 1
	#define COMPILE_MININVFREQ_ACCUMULATOR 1
#endif
"
)
(refer SSD/SSDSpatialKernel.lsl)
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
#define SignalInput_Textures_1 SignalInput_Textures_0
#define SignalInput_Textures_2 SignalInput_Textures_1
#define SignalInput_Textures_3 SignalInput_Textures_2

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

	FSSDSignalArray RefSamples = SampleMultiplexedSignals(
			Signal_Textures_0,
			Signal_Textures_1,
			Signal_Textures_2,
			Signal_Textures_3,
			GlobalPointClampedSampler,
			CONFIG_SIGNAL_INPUT_LAYOUT,
			/* MultiplexedSampleId = */ 0,
			/* bNormalizeSample = */ CONFIG_NORMALIZE_INPUT != 0,
			SceneBufferUV);
		
		#if CONFIG_NORMALIZE_INPUT
			FSSDSignalArray NormalizedRefSamples = RefSamples;
		#else
			// TODO(Denoiser): Decode twice instead.
			FSSDSignalArray NormalizedRefSamples = NormalizeToOneSampleArray(RefSamples);
		#endif

	/** factor by witch should be spread out. */
	#if CONFIG_UPSCALE
		float KernelSpreadFactor = UpscaleFactor;
	#elif !CONFIG_CUSTOM_SPREAD_FACTOR
		const float KernelSpreadFactor = 1;
	#endif

	/** Find out the number of samples that should be done. */
	float RequestedSampleCount = 1024;

	float2 KernelBufferUV;
	uint SampleTrackId;

	FSSDSignalAccumulatorArray SignalAccumulators;
	{
		FSSDKernelConfig KernelConfig = CreateKernelConfig();

		KernelConfig.SampleSet = CONFIG_SAMPLE_SET;
		KernelConfig.SampleSubSetId = CONFIG_SAMPLE_SUBSET;
		KernelConfig.BufferLayout = CONFIG_SIGNAL_INPUT_LAYOUT;
		KernelConfig.MultiplexedSignalsPerSignalDomain = CONFIG_MULTIPLEXED_SIGNALS_PER_SIGNAL_DOMAIN;
		KernelConfig.NeighborToRefComputation = NEIGHBOR_TO_REF_LOWEST_VGPR_PRESSURE;
		KernelConfig.bUnroll = CONFIG_SAMPLE_SET != SAMPLE_SET_STACKOWIAK_4_SETS;
		KernelConfig.bDescOrder = CONFIG_OUTPUT_MODE == OUTPUT_MODE_DRB;
		KernelConfig.BilateralDistanceComputation = CONFIG_BILATERAL_DISTANCE_COMPUTATION;
		KernelConfig.WorldBluringDistanceMultiplier = CONFIG_BILATERAL_DISTANCE_MULTIPLIER;
		KernelConfig.bNormalizeSample = CONFIG_NORMALIZE_INPUT != 0;;
		KernelConfig.bSampleKernelCenter = CONFIG_UPSCALE;
		KernelConfig.bForceKernelCenterAccumulation = true;
		KernelConfig.bClampUVPerMultiplexedSignal = CONFIG_CLAMP_UV_PER_SIGNAL != 0;

		
		KernelConfig.bComputeSampleColorSH = DIM_STAGE == STAGE_RECONSTRUCTION && DIM_MULTI_SPP == 0;
		{
			UNROLL_N(SIGNAL_ARRAY_SIZE)
			for (uint MultiplexId = 0; MultiplexId < 1; MultiplexId++)
			{
				KernelConfig.BufferColorSpace[MultiplexId] = CONFIG_INPUT_COLOR_SPACE;
				KernelConfig.AccumulatorColorSpace[MultiplexId] = CONFIG_ACCUMULATION_COLOR_SPACE;
			}
		}

		SetBilateralPreset(0x0011,  KernelConfig);

		KernelConfig.BufferSizeAndInvSize = BufferSizeAndInvSize;
		KernelConfig.BufferBilinearUVMinMax = BufferBilinearUVMinMax;
		KernelConfig.KernelSpreadFactor = KernelSpreadFactor;
		KernelConfig.HarmonicPeriode = HarmonicPeriode;

		#if CONFIG_CLAMP_UV_PER_SIGNAL
			UNROLL_N(CONFIG_SIGNAL_BATCH_SIZE)
			for (uint BatchedSignalId = 0; BatchedSignalId < CONFIG_SIGNAL_BATCH_SIZE; BatchedSignalId++)
			{
				uint MultiplexId = BatchedSignalId / CONFIG_MULTIPLEXED_SIGNALS_PER_SIGNAL_DOMAIN;
				KernelConfig.PerSignalUVMinMax[MultiplexId] = InputBufferUVMinMax[MultiplexId];
			}
		#endif
		
		KernelConfig.BufferUV = SceneBufferUV; 
		{
			// Straight up plumb down the compress layout to save any VALU.
			KernelConfig.CompressedRefSceneMetadata = CompressedRefSceneMetadata;
			
			KernelConfig.RefBufferUV = SceneBufferUV;
			KernelConfig.RefSceneMetadataLayout = CONFIG_METADATA_BUFFER_LAYOUT;
		}
		KernelConfig.HammersleySeed = Rand3DPCG16(int3(SceneBufferUV * BufferUVToOutputPixelPosition, View.StateFrameIndexMod8)).xy;

		// Set up reference distance for all signals.
		#if CONFIG_MAX_WITH_REF_DISTANCE
		{
			KernelConfig.bMaxWithRefBilateralDistance = true;

			UNROLL_N(SIGNAL_ARRAY_SIZE)
			for (uint MultiplexId = 0; MultiplexId < 1; MultiplexId++)
			{
				if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS)
				{
					KernelConfig.RefBilateralDistance[MultiplexId] = RefSamples.Array[MultiplexId].WorldBluringRadius;
				}
				else
				{
					const uint BatchedSignalId = ComputeSignalBatchIdFromSignalMultiplexId(KernelConfig, MultiplexId);

					KernelConfig.RefBilateralDistance[MultiplexId] = GetSignalWorldBluringRadius(RefSamples.Array[MultiplexId], RefSceneMetadata, BatchedSignalId);
				}
			}
		}
		#endif

		FSSDSignalAccumulatorArray UncompressedAccumulators = CreateSignalAccumulatorArray();

		// When not upscaling, manually force accumulate the sample of the kernel.
		if (!KernelConfig.bSampleKernelCenter && !KernelConfig.bDescOrder)
		{
			UNROLL_N(SIGNAL_ARRAY_SIZE)
			for (uint SignalMultiplexId = 0; SignalMultiplexId < SIGNAL_ARRAY_SIZE; SignalMultiplexId++)
			{
				const uint BatchedSignalId = ComputeSignalBatchIdFromSignalMultiplexId(KernelConfig, SignalMultiplexId);
				
				uint2 RefPixelCoord = floor(KernelConfig.BufferUV * KernelConfig.BufferSizeAndInvSize.xy);
				FSSDSignalSample CenterSample = TransformSignalSampleForAccumulation(
					KernelConfig,
					SignalMultiplexId,
					RefSceneMetadata,
					RefSamples.Array[SignalMultiplexId],
					RefPixelCoord);
				
				FSSDSampleAccumulationInfos SampleInfos;
				SampleInfos.Sample = CenterSample;
				SampleInfos.FinalWeight = 1.0;
				SampleInfos.InvFrequency = GetSignalWorldBluringRadius(CenterSample, RefSceneMetadata, BatchedSignalId);
				
				if (KernelConfig.BilateralDistanceComputation == SIGNAL_WORLD_FREQUENCY_PRECOMPUTED_BLURING_RADIUS)
				{
					// TODO(Denoiser): this is ineficient, could fetch the normalised WorldBluringRadius instead of SafeRcp().
					SampleInfos.InvFrequency = CenterSample.WorldBluringRadius * SafeRcp(CenterSample.SampleCount);
				}

				AccumulateSample(
					/* inout */ UncompressedAccumulators.Array[SignalMultiplexId],
					SampleInfos);
			}
		}

		#if CONFIG_SAMPLE_SET == SAMPLE_SET_STACKOWIAK_4_SETS
		{
			KernelConfig.SampleCount = clamp(uint(RequestedSampleCount) / kStackowiakSampleSetCount, 1, MaxSampleCount);

			#if CONFIG_UPSCALE
			{
				// TODO(Denoiser): could be optimised, but currently reusing same peace of code as reflection for maintainability.
				uint2 RayDispatchThreadId = (DispatchThreadId - UpscaleFactor / 2) / UpscaleFactor;
				uint2 ClosestRayPixelCoord = GetPixelCoord(RayDispatchThreadId);
			
				uint RaySubPixelId = View.StateFrameIndex & (UpscaleFactor * UpscaleFactor - 1);

				KernelConfig.BufferUV = ((ViewportMin + ClosestRayPixelCoord + (0.5 * KernelSpreadFactor + 0.5))) * KernelConfig.BufferSizeAndInvSize.zw;
			
				// Sample the center of the kernel by comparing it against the RefSceneMetadata, since it may no match.
				KernelConfig.bSampleKernelCenter = true;

				// Id of the pixel in the quad.
				KernelConfig.SampleTrackId = ((DispatchThreadId.x & 1) | ((DispatchThreadId.y & 1) << 1)) ^ 0x3;

				// To avoid precision problem when comparing potentially identicall 
				KernelConfig.bForceKernelCenterAccumulation = RaySubPixelId == ((DispatchThreadId.x & 1) | ((DispatchThreadId.y & 1) << 1));
			}
			#else
			{
				// Put the kernel center at the center of the quad. Half pixel shift is done in the sample offsets.
				KernelConfig.BufferUV = float2(DispatchThreadId | 1) * ThreadIdToBufferUV.xy + ThreadIdToBufferUV.zw;

				// Id of the pixel in the quad. This is to match hard coded first samples of the sample set.
				KernelConfig.SampleTrackId = ((DispatchThreadId.x & 1) | ((DispatchThreadId.y & 1) << 1));
			}
			#endif

			#if CONFIG_VGPR_OPTIMIZATION
				// Keek sample SampleTrackId & SceneBufferUV arround for computation of pixel output coordinate.
				// Should be VGPR free given it's curernt is being used in accumulation has well that is highest VGPR pressure of the shader.
				// TODO(Denoiser): could save 1 VGPR by using 2 SGPR instead of SampleTrackId.
				SampleTrackId = KernelConfig.SampleTrackId;
				KernelBufferUV = KernelConfig.BufferUV;
			#endif
		}
		#endif

		FSSDCompressedSignalAccumulatorArray CompressedAccumulators = CompressAccumulatorArray(UncompressedAccumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);

		AccumulateKernel(
			KernelConfig,
			Signal_Textures_0,
			Signal_Textures_1,
			Signal_Textures_2,
			Signal_Textures_3,
			/* inout */ UncompressedAccumulators,
			/* inout */ CompressedAccumulators);

			// Manually sample the center of the kernel after any accumulation when accumulating in descending order.
		if (!KernelConfig.bSampleKernelCenter && KernelConfig.bDescOrder)
		{
			// Remove any jitter the kernel may have. Won't have ant VGPR cost when no jittering, because KernelConfig.BufferUV == SceneBufferUV.
			// TODO(Denoiser): This is costly for VGPR pressure if using KernelConfig.BufferUV was != SceneBufferUV.
			KernelConfig.BufferUV = SceneBufferUV;

			SampleAndAccumulateCenterSampleAsItsOwnCluster(
				KernelConfig,
				Signal_Textures_0,
				Signal_Textures_1,
				Signal_Textures_2,
				Signal_Textures_3,
				/* inout */ UncompressedAccumulators,
				/* inout */ CompressedAccumulators);
		}
		
		#if CONFIG_ACCUMULATOR_VGPR_COMPRESSION == ACCUMULATOR_COMPRESSION_DISABLED
			SignalAccumulators = UncompressedAccumulators;
		#else
			SignalAccumulators = UncompressAccumulatorArray(CompressedAccumulators, CONFIG_ACCUMULATOR_VGPR_COMPRESSION);
		#endif
	}

	// Transcode the spatial accumulation into multiplexed signal according to different modes.
	uint MultiplexCount = 1;
	FSSDSignalArray OutputSamples = CreateSignalArrayFromScalarValue(0.0);
	{
	#if CONFIG_OUTPUT_MODE == OUTPUT_MODE_DRB
		{
			MultiplexCount = CONFIG_SIGNAL_BATCH_SIZE;
			
			UNROLL_N(CONFIG_SIGNAL_BATCH_SIZE)
			for (uint MultiplexId = 0; MultiplexId < CONFIG_SIGNAL_BATCH_SIZE; MultiplexId++)
			{
				UncompressSignalAccumulator(/* inout */ SignalAccumulators.Array[MultiplexId]);

				OutputSamples.Array[MultiplexId] = SignalAccumulators.Array[MultiplexId].Previous;

				// Output the minimal inverse frequency as new world bluring radius for subsequent passes.
				// TODO(Denoiser): store this in its own signal to avoid *= N; *= rcp(N);
				OutputSamples.Array[MultiplexId].WorldBluringRadius = 
					OutputSamples.Array[MultiplexId].SampleCount * SignalAccumulators.Array[MultiplexId].MinInvFrequency;
			
				// No need to keep the VGPR pressure at this point for WorldBluringRadius, because no passes use it after.
				if (DIM_STAGE == STAGE_POST_FILTERING && 0)
				{
					OutputSamples.Array[MultiplexId].WorldBluringRadius = 0;
				}
			}
		}
		#endif
	}

	uint2 OutputPixelPostion;
	#if CONFIG_VGPR_OPTIMIZATION && !CONFIG_UPSCALE // TODO(Denoiser)
	{
		// No need to keep DispatchThreadId, can recompute the output pixel position based on information stored in VGPRs for spatial kernel.
		#if CONFIG_SAMPLE_SET == SAMPLE_SET_STACKOWIAK_4_SETS || CONFIG_SAMPLE_SET == SAMPLE_SET_STOCASTIC_HIERARCHY
			#if CONFIG_VGPR_FREE_SAMPLE_TRACK_ID
				SampleTrackId = GetSampleTrackIdFromLaneIndex();
			#endif

			#if CONFIG_UPSCALE
				SampleTrackId ^= 0x3;
			#endif
			OutputPixelPostion = (uint2(KernelBufferUV * BufferUVToOutputPixelPosition) & ~0x1) | (uint2(SampleTrackId, SampleTrackId >> 1) & 0x1);
		#else
			OutputPixelPostion = BufferUVToBufferPixelCoord(SceneBufferUV);
		#endif
	}
	#else
		OutputPixelPostion = ViewportMin + DispatchThreadId;
	#endif 

	RANCH
	if (all(OutputPixelPostion < ViewportMax))
	{
		OutputMultiplexedSignal(
				SignalOutput_UAVs_0,
				SignalOutput_UAVs_0,
				SignalOutput_UAVs_0,
				SignalOutput_UAVs_0,
				CONFIG_SIGNAL_OUTPUT_LAYOUT, MultiplexCount,
				OutputPixelPostion, OutputSamples.Array);
	}
}
")
)