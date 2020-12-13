#ifndef SSDDefinitions_h
#define SSDDefinitions_h 1

static const float DENOISER_MISS_HIT_DISTANCE = -1.0;
static const float WORLD_RADIUS_MISS = asfloat(0x7F7FFFFF);

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

#endif