// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

#pragma warning(disable:3557)
#pragma warning(disable:3574)

//-----------------------------------------------------------------------------
// Macros and constant values
//-----------------------------------------------------------------------------

// The compute shaders for column and row filtering are created from the same
// function, distinguished by macro "SCAN_COL_PASS" parsed to the compiler.
#if SCAN_COL_PASS == 1
#define input_mono(a, b)	input_col_mono(a, b)
#define output_mono(a, b)	output_col_mono(a, b)
#define DATA_SIZE			NUM_IMAGE_ROWS
#else
#define input_mono(a, b)	input_row_mono(a, b)
#define output_mono(a, b)	output_row_mono(a, b)
#define DATA_SIZE			NUM_IMAGE_COLS
#endif


static const uint SCAN_WARP_SIZE = 32;
static const uint SCAN_LOG2_WARP_SIZE = 5;

// The following constants are defined via compiler command line
//static const uint THREADS_PER_GROUP = 256;
//static const uint SCAN_SMEM_SIZE = 1200;
//static const uint TEXELS_PER_THREAD = 5;
//static const uint NUM_IMAGE_ROWS = 1200;
//static const uint NUM_IMAGE_COLS = 1600;



//-----------------------------------------------------------------------------
// Shared memory variables
//-----------------------------------------------------------------------------

// Shared memory for prefix sum operation (scan).
groupshared float gs_ScanData[SCAN_SMEM_SIZE];


//-----------------------------------------------------------------------------
// Shader constant buffers
//-----------------------------------------------------------------------------

cbuffer cbParams
{
	float g_HalfBoxFilterWidth = 10.f;			// w/2
	float g_FracHalfBoxFilterWidth = 0.3f;		// frac(w/2+0.5)
	float g_InvFracHalfBoxFilterWidth = 0.7f;	// 1-frac(w/2+0.5)
	float g_RcpBoxFilterWidth = 0.05f;			// 1/w
	uint  g_NumApproxPasses = 3;
};


//-----------------------------------------------------------------------------
// Buffers, textures & samplers
//-----------------------------------------------------------------------------

// FP16x4 texture of rendered scene
Texture2D				g_texInput;
// FP32 texture format for output
RWTexture2D<uint>		g_rwtOutput;


//-----------------------------------------------------------------------------
// Subroutines for scan operation
//-----------------------------------------------------------------------------

inline float scan_warp_inclusive(uint thread_id, float value, uint size)
{
	// Naive inclusive scan: O(N * log2(N)) operations within a warp (32 threads)
	// All threads within a warp always execute the same instruction, thus we don't
	// need to do sync-up.
	uint location = 2 * thread_id - (thread_id & (size - 1));

	// Initialize the first half of shared memory with zeros to avoid "if (pos >= offset)"
	// condition evaluation
	gs_ScanData[location] = 0;
	location += size;
	gs_ScanData[location] = value;

	for (uint offset = 1; offset < size; offset <<= 1)
		gs_ScanData[location] += gs_ScanData[location - offset];

	return gs_ScanData[location];
}

inline float scan_warp_exclusive(uint thread_id, float value, uint size)
{
	return scan_warp_inclusive(thread_id, value, size) - value;
}

inline float scan_top_inclusive(uint thread_id, float value, uint size)
{
	// Warp-level inclusive warp scan. Preserve scan result in each thread's
	// register space (variable 'warp_result')
	float warp_result = scan_warp_inclusive(thread_id, value, SCAN_WARP_SIZE);

	// Sync to wait for warp scans to complete because gs_ScanData is going to
	// be overwritten
	GroupMemoryBarrierWithGroupSync();

	// Save top elements of each warp for exclusive warp scan
	if ((thread_id & (SCAN_WARP_SIZE - 1)) == SCAN_WARP_SIZE - 1)
		gs_ScanData[thread_id >> SCAN_LOG2_WARP_SIZE] = warp_result;

	// Wait for warp scans to complete
	GroupMemoryBarrierWithGroupSync();

	// Grab top warp elements
	float top_value = gs_ScanData[thread_id];

	// Calculate exclsive scan and write back to shared memory
	gs_ScanData[thread_id] = scan_warp_exclusive(thread_id, top_value, THREADS_PER_GROUP >> SCAN_LOG2_WARP_SIZE);

	// Wait for the result of top element scan
	GroupMemoryBarrierWithGroupSync();

	// Return updated warp scans with exclusive scan results
	return warp_result + gs_ScanData[thread_id >> SCAN_LOG2_WARP_SIZE];
}

inline float scan_top_exclusive(uint thread_id, float value, uint size)
{
	return scan_top_inclusive(thread_id, value, size) - value;
}

inline void scan_inclusive(uint thread_id, uint size)
{
	uint i;

	// Each thread deals the number of "TEXELS_PER_THREAD" texels
	uint location = thread_id * TEXELS_PER_THREAD;
	// The lowest level (level-0) are stored in register space
	float local_data[TEXELS_PER_THREAD];

	// Read back data from shared memory to register space
	for (i = 0; i < TEXELS_PER_THREAD; i++)
		local_data[i] = location + i < size ? gs_ScanData[location + i] : 0;

	// Perform level-0 sum
	for (i = 1; i < TEXELS_PER_THREAD; i++)
		local_data[i] += local_data[i - 1];

	// Wait until all intra-thread operations finished
	GroupMemoryBarrierWithGroupSync();

	// Level-1 exclusive scan
	float top_value = local_data[TEXELS_PER_THREAD - 1];
	float top_result = scan_top_exclusive(thread_id, top_value, THREADS_PER_GROUP);

	// Wait until top level scan finished
	GroupMemoryBarrierWithGroupSync();

	// Propagate level-1 scan result to level-0, and then write to shared memory
	for (i = 0; i < TEXELS_PER_THREAD; i++)
	{
		if (location + i < size)
			gs_ScanData[location + i] = local_data[i] + top_result;
	}

	// Wait until all write operations finished
	GroupMemoryBarrierWithGroupSync();
}

inline void scan_inclusive_filtering(uint thread_id, uint size)
{
	int i;

	// Each thread deals the number of "TEXELS_PER_THREAD" texels
	uint location = thread_id * TEXELS_PER_THREAD;
	// The lowest level (level-0) are stored in register space
	float local_data[TEXELS_PER_THREAD];

	// Calculating average values in the box window while performing level-0
	int L_pos = (int)ceil(location - 0.5f - g_HalfBoxFilterWidth) - 1;
	int R_pos = (int)floor(location - 0.5f + g_HalfBoxFilterWidth);

	float L_sum = gs_ScanData[clamp(L_pos, 0, (int)size - 1)] * g_FracHalfBoxFilterWidth;
	float R_sum = gs_ScanData[clamp(R_pos, 0, (int)size - 1)] * g_InvFracHalfBoxFilterWidth;

	for (i = 0; (uint)i < TEXELS_PER_THREAD; i++)
	{
		float L_next = gs_ScanData[clamp(L_pos + 1 + i, 0, (int)size - 1)];
		float R_next = gs_ScanData[clamp(R_pos + 1 + i, 0, (int)size - 1)];

		local_data[i] = (R_sum + R_next * g_FracHalfBoxFilterWidth) - (L_sum + L_next * g_InvFracHalfBoxFilterWidth);
		local_data[i] *= g_RcpBoxFilterWidth;

		L_sum += L_next;
		R_sum += R_next;
	}

	// Wait until all intra-thread operations finished
	GroupMemoryBarrierWithGroupSync();

	// Level-1 exclusive scan
	float top_value = local_data[TEXELS_PER_THREAD - 1];
	float top_result = scan_top_exclusive(thread_id, top_value, THREADS_PER_GROUP);

	// Wait until top level scan finished
	GroupMemoryBarrierWithGroupSync();

	// Propagate level-1 scan result to level-0, and then write to shared memory
	for (i = 0; (uint)i < TEXELS_PER_THREAD; i++)
	{
		if (location + i < size)
			gs_ScanData[location + i] = local_data[i] + top_result;
	}

	// Wait until all write operations finished
	GroupMemoryBarrierWithGroupSync();
}

inline float box_filtering(uint location, uint size)
{
	// Calculate the sampling locations (left and right side of the filter window).
	// We treat the original data as a piecewise box function, thus the sum is a
	// piecewise linear function. For arbitrary FP sampling location, we interpolate
	// the value from adjacent data pairs.

	float center = location - 0.5f;
	int L_a = clamp((int)ceil(center - g_HalfBoxFilterWidth), 0, (int)size - 1);
	int L_b = clamp(L_a - 1, 0, (int)size - 1);
	int R_a = clamp((int)floor(center + g_HalfBoxFilterWidth), 0, (int)size - 1);
	int R_b = clamp(R_a + 1, 0, (int)size - 1);

	float L_value = lerp(gs_ScanData[L_a], gs_ScanData[L_b], g_FracHalfBoxFilterWidth);
	float R_value = lerp(gs_ScanData[R_a], gs_ScanData[R_b], g_FracHalfBoxFilterWidth);

	return (R_value - L_value) * g_RcpBoxFilterWidth;
}

static const float3 RGB709_to_CIE_Y = float3(0.212671f, 0.715160f, 0.072169f);

inline void input_col_mono(uint group_id, uint thread_id)
{
	uint col = group_id;
	uint row = thread_id;

	// Fetch in FP32 value.
	while (row < NUM_IMAGE_ROWS)
	{
		gs_ScanData[row] =dot(g_texInput[uint2(col, row)].xyz,RGB709_to_CIE_Y);

		row += THREADS_PER_GROUP;
	}

	// Wait until all write operations finished
	GroupMemoryBarrierWithGroupSync();
}

inline uint color_to_uint(float color)
{
	// Convert to R11G11B10
	color = clamp(color, 0, 1);
	uint int_r = (uint)(color * 2047.0f + 0.5f);
	//uint int_g = (uint)(color * 2047.0f + 0.5f);
	uint int_b = (uint)(color * 1023.0f + 0.5f);

	// Pack into UINT32
	return (int_r << 21) | (int_r << 10) | int_b;
}

inline void output_col_mono(uint group_id, uint thread_id)
{
	uint col = group_id;
	uint row = thread_id;

	// Output FP32 values.
	while (row < NUM_IMAGE_ROWS)
	{
		g_rwtOutput[uint2(col, row)] = color_to_uint(box_filtering(row, NUM_IMAGE_ROWS));

		row += THREADS_PER_GROUP;
	}
}

inline float uint_to_color(uint int_color)
{
	// Unpack from UINT32
	float r = (float)(int_color >> 21);
	//float g = (float)((int_color >> 10) & 0x7ff);
	//float b = (float)(int_color & 0x0003ff);

	// Convert R11G11B10 to float3
	return r / 2047.0f;
}

inline void input_row_mono(uint group_id, uint thread_id)
{
	uint col = thread_id;
	uint row = group_id;

	// Fetch back the data output by vertical filtering pass.
	while (col < NUM_IMAGE_COLS)
	{
		gs_ScanData[col] = uint_to_color(g_rwtOutput[uint2(col, row)]);

		col += THREADS_PER_GROUP;
	}

	// Wait until all write operations finished
	GroupMemoryBarrierWithGroupSync();
}





inline void output_row_mono(uint group_id, uint thread_id)
{
	uint col = thread_id;
	uint row = group_id;

	// Output FP32 values.
	while (col < NUM_IMAGE_COLS)
	{
		g_rwtOutput[uint2(col, row)] = color_to_uint(box_filtering(col, NUM_IMAGE_COLS));

		col += THREADS_PER_GROUP;
	}
}


//-----------------------------------------------------------------------------
// Name: GaussianMono_CS
// Type: Compute Shader
// Desc: Perform column or row Gaussian filtering
//-----------------------------------------------------------------------------
[numthreads(THREADS_PER_GROUP, 1, 1)]
void GaussianMono_CS(uint3 GroupID			: SV_GroupID,
	uint3 GroupThreadID : SV_GroupThreadID)
{
	// Step 1: Fetch the entire column (or row) of texels into shared memory
	input_mono(GroupID.x, GroupThreadID.x);

	// Step 2. Scan the columns or rows in-place for the first time
	scan_inclusive(GroupThreadID.x, DATA_SIZE);

	// Step 3: Perform box filtering repeatly for approximating Gaussian blur
	for (uint i = 0; i < g_NumApproxPasses; i++)
		scan_inclusive_filtering(GroupThreadID.x, DATA_SIZE);

	// Step 4: Write back to global memory
	output_mono(GroupID.x, GroupThreadID.x);
}
