#ifndef SSDPublicBufferEncoding_h
#define SSDPublicBufferEncoding_h 1

/** Compile time settings to encode buffer to the denoiser. */
#define SSD_ENCODE_CLAMP_COLOR 0x1
#define SSD_ENCODE_NORMALIZE_MISS 0x2
#define SSD_ENCODE_NORMALIZE_COLOR 0x4
#define SSD_ENCODE_NORMALIZE (SSD_ENCODE_NORMALIZE_MISS | SSD_ENCODE_NORMALIZE_COLOR)

/** Compile time settings to decode buffer from the denoiser. */
#define SSD_DECODE_NORMALIZE 0x1

uint Compress2Floats(float2 v)
{
	// f32tof16() is full rate on GCN.
	return f32tof16(v.x) | (f32tof16(v.y) << 16);
}

#endif
