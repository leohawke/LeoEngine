Texture2D<float4> Input:register(t0);
RWTexture2D<float4> Output:register(u0);
#ifndef APPROACH 
/*
#define RADIUS 5

static const float filter[RADIUS][RADIUS] = {};
*/


#define PATH_SIZE_X 32
#define PATH_SIZE_Y 32

[numthreads(PATH_SIZE_X, PATH_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int3 texturelocation = DTid - int3((RADIUS - 1) / 2, (RADIUS - 1) / 2, 0);

	float4 centerColor = Input.Load(DTid);

	const float rsigma = 0.051f;

	float4 outputColor = 0.f;
	float4 weight = 0.f;

	for (int x = 0; x < RADIUS; ++x)
	{
		for (int y = 0; y < RADIUS; ++y) {
			// Get the current sample 
			float4 SampleColor = Input.Load(texturelocation + int3(x, y, 0));
			//Find the delta, and use that to calculate the range weighting 
			float4 Delta = centerColor - SampleColor;
			float4 Range = exp((-1.0f * Delta * Delta) / (2.0f * rsigma * rsigma));
			// Sum both the color result and the total weighting used 
			outputColor += SampleColor * Range * filter[x][y];
			weight += Range * filter[x][y];
		}
	}
	// Store the renormalized result to the output resource 
	Output[DTid.xy] = outputColor / weight;

}
#else
/*
#define WIDTH 400
#define HEIGHT 300
static const float filter[RADIUS] = {};
*/

[numthreads(WIDTH, 1, 1)]
void approach_ver_main(uint3 DTid : SV_DispatchThreadID) {
	int3 texturelocation = DTid - int3((RADIUS - 1) / 2, 0, 0);

	float4 centerColor = Input.Load(DTid);

	const float rsigma = 0.051f;

	float4 outputColor = 0.f;
	float4 weight = 0.f;

	for (int x = 0; x < RADIUS; ++x)
	{
		// Get the current sample 
		float4 SampleColor = Input.Load(texturelocation + int3(x, 0, 0));
		//Find the delta, and use that to calculate the range weighting 
		float4 Delta = centerColor - SampleColor;
		float4 Range = exp((-1.0f * Delta * Delta) / (2.0f * rsigma * rsigma));
		// Sum both the color result and the total weighting used 
		outputColor += SampleColor * Range * filter[x];
		weight += Range * filter[x];
	}
	// Store the renormalized result to the output resource 
	Output[DTid.xy] = outputColor / weight;

}

[numthreads(1, HEIGHT, 1)]
void approach_hor_main(uint3 DTid : SV_DispatchThreadID) {
	int3 texturelocation = DTid - int3(0, (RADIUS - 1) / 2, 0);

	float4 centerColor = Input.Load(DTid);

	const float rsigma = 0.051f;

	float4 outputColor = 0.f;
	float4 weight = 0.f;

	for (int y = 0; y < RADIUS; ++y)
	{
		// Get the current sample 
		float4 SampleColor = Input.Load(texturelocation + int3(0, y, 0));
		//Find the delta, and use that to calculate the range weighting 
		float4 Delta = centerColor - SampleColor;
		float4 Range = exp((-1.0f * Delta * Delta) / (2.0f * rsigma * rsigma));
		// Sum both the color result and the total weighting used 
		outputColor += SampleColor * Range * filter[y];
		weight += Range * filter[y];
	}
	// Store the renormalized result to the output resource 
	Output[DTid.xy] = outputColor / weight;
}

#endif