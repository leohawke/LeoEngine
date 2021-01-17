#ifndef SSDDefinitions_h
#define SSDDefinitions_h 1

static const float DENOISER_MISS_HIT_DISTANCE = -1.0;
static const float WORLD_RADIUS_MISS = asfloat(0x7F7FFFFF);
static const float WORLD_RADIUS_INVALID = -1;

// Maximum number of buffer multiple signals can be encode to/decoded from.
#define MAX_MULTIPLEXED_TEXTURES 4

//------------------------------------------------------- ENUMS

/** Layouts of the metadata buffer */
	/** Uses standard depth buffer and gbuffer. */
#define METADATA_BUFFER_LAYOUT_DISABLED 0

/** Layouts of the signal buffer. */
	/** Buffer layout for the shadow penumbra given as input. */
#define SIGNAL_BUFFER_LAYOUT_UNINITIALIZED 0xDEAD


/** Buffer layout for the shadow penumbra given as input. */
#define SIGNAL_BUFFER_LAYOUT_PENUMBRA_INPUT_NSPP 15

/** Buffer layout for the shadow penumbra given as input. */
#define SIGNAL_BUFFER_LAYOUT_PENUMBRA_INJESTION_NSPP 16

/** Internal buffer layout for the shadow penumbra multiplexed into buffers.
 * This buffer layout is able to pack two signals per render target.
 */
#define SIGNAL_BUFFER_LAYOUT_PENUMBRA_RECONSTRUCTION 12

 /** Internal buffer to encode history rejection preconvolution. */
#define SIGNAL_BUFFER_LAYOUT_PENUMBRA_REJECTION 17

/** Internal buffer layout for the shadow penumbra to be stored in indivual per light histories. */
#define SIGNAL_BUFFER_LAYOUT_PENUMBRA_HISTORY 11

/** Defines how the signal is being processed. Matches C++'s ESignalProcessing. */
	/** Shadow penumbra denoising. */
#define SIGNAL_PROCESSING_SHADOW_VISIBILITY_MASK 0

/** Shadow penumbra denoising. */
#define SIGNAL_PROCESSING_POLYCHROMATIC_PENUMBRA_HARMONIC 1

/** Defines the color space bitfield. */
#define COLOR_SPACE_RGB 0x0

/** Sets of sample available for the spatial kernel. */
	// For debug purpose, only sample the center of the kernel.
#define SAMPLE_SET_1X1 0

// Filtering
#define SAMPLE_SET_2X2_BILINEAR 1

// Stocastic 2x2 that only take one sample.
#define SAMPLE_SET_2X2_STOCASTIC 13

// Square kernel
#define SAMPLE_SET_3X3 2
#define SAMPLE_SET_3X3_SOBEK2018 3
#define SAMPLE_SET_5X5_WAVELET 4

#define SAMPLE_SET_3X3_PLUS 5
#define SAMPLE_SET_3X3_CROSS 6

#define SAMPLE_SET_NXN 7

// [ Stackowiak 2015, "Stochastic Screen-Space Reflections" ]
#define SAMPLE_SET_STACKOWIAK_4_SETS 8

#define SAMPLE_SET_HEXAWEB 11

#define SAMPLE_SET_STOCASTIC_HIERARCHY 12

#define SAMPLE_SET_DIRECTIONAL_RECT 14
#define SAMPLE_SET_DIRECTIONAL_ELLIPSE 15

/** By default, the color space stored into intermediary buffer is linear premultiplied RGBA. */
#define STANDARD_BUFFER_COLOR_SPACE COLOR_SPACE_RGB

/** Compression of signal for VGPR occupency. */
	/** The signal is not being compressed. */
#define SIGNAL_COMPRESSION_DISABLED 0

/** VGPR compression for spherical harmonic. */
#define SIGNAL_COMPRESSION_DIFFUSE_INDIRECT_HARMONIC 0xD200


/** Compression of signal for VGPR occupency. */
	/** The signal is not being compressed. */
#define ACCUMULATOR_COMPRESSION_DISABLED 0

/** VGPR compression for spherical harmonic. */
#define ACCUMULATOR_COMPRESSION_PENUMBRA_DRB 0x0001

/** Technic used to compute the world vector betweem neighbor and reference. */
// Directly use FSSDKernelConfig::RefSceneMetadata.TranslatedWorldPosition. Cost 3 VGPR over entire kernel inner loop.
#define NEIGHBOR_TO_REF_CACHE_WORLD_POSITION 0

// Compute by deriving the information of the reference pixel last minute from kernel configuration, trading VALU to save VGPR.
	// Caution: does not work with previous frame reprojection.
#define NEIGHBOR_TO_REF_LOWEST_VGPR_PRESSURE 1

/** Type to use for texture sampling. */
#define SIGNAL_TEXTURE_TYPE_FLOAT4 0
#define SIGNAL_TEXTURE_TYPE_UINT1 1
#define SIGNAL_TEXTURE_TYPE_UINT2 2
#define SIGNAL_TEXTURE_TYPE_UINT3 3
#define SIGNAL_TEXTURE_TYPE_UINT4 4

/** Bilateral settings bitfield. */
#define BILATERAL_POSITION_BASED(n) (0x0000 + (n) & 0xF)
#define BILATERAL_NORMAL 0x0010
#define BILATERAL_TOKOYASHI_LOBE 0x0020
#define BILATERAL_TOKOYASHI_AXES 0x0040
#define BILATERAL_TOKOYASHI (BILATERAL_TOKOYASHI_LOBE | BILATERAL_TOKOYASHI_AXES)

/** Bilateral settings presets for spatial kernels. */
	// No bilateral settings.
#define BILATERAL_PRESET_DISABLED 0x0000

#define BILATERAL_PRESET_MONOCHROMATIC_PENUMBRA 0x0011

#define BILATERAL_PRESET_POLYCHROMATIC_PENUMBRA 0x0022

#define BILATERAL_PRESET_REFLECTIONS 0x1001
#define BILATERAL_PRESET_REFLECTIONS_1SPP 0x1002
#define BILATERAL_PRESET_REFLECTIONS_TAA 0x1003

#define BILATERAL_PRESET_DIFFUSE 0x2001

#define BILATERAL_PRESET_SPHERICAL_HARMONIC 0x3001

#endif