#define NOISE_LATTICE_SIZE 16
#define INV_LATTICE_SIZE (1.0/(float)(NOISE_LATTICE_SIZE))


SamplerState LinearRepeat :register(s0);

SamplerState NearestClamp :register(s1);

SamplerState NearestRepeat :register(s2);

SamplerState LinearClamp :register(s3);

float4 NLQu(float3 uvw, Texture3D noiseTex) {
	return noiseTex.SampleLevel(LinearRepeat, uvw, 0);
}

float4 NLQs(float3 uvw, Texture3D noiseTex) {
	return NLQu(uvw, noiseTex) * 2 - 1;
}

float4 NMQu(float3 uvw, Texture3D noiseTex) {
	// smooth the input coord
	float3 t = frac(uvw * NOISE_LATTICE_SIZE + 0.5);
		float3 t2 = (3 - 2 * t)*t*t;
		float3 uvw2 = uvw + (t2 - t) / (float)(NOISE_LATTICE_SIZE);
		// fetch
		return NLQu(uvw2, noiseTex);
}

float4 NMQs(float3 uvw, Texture3D noiseTex) {
	// smooth the input coord
	float3 t = frac(uvw * NOISE_LATTICE_SIZE + 0.5);
		float3 t2 = (3 - 2 * t)*t*t;
		float3 uvw2 = uvw + (t2 - t) / (float)(NOISE_LATTICE_SIZE);
		// fetch  
		return NLQs(uvw2, noiseTex);
}

// SUPER MEGA HIGH QUALITY noise sampling (signed)
float NHQu(float3 uvw, Texture3D tex, float smooth = 1)
{
	float3 uvw2 = floor(uvw * NOISE_LATTICE_SIZE) * INV_LATTICE_SIZE;
		float3 t = (uvw - uvw2) * NOISE_LATTICE_SIZE;
		t = lerp(t, t*t*(3 - 2 * t), smooth);

	float2 d = float2(INV_LATTICE_SIZE, 0);

		float4 f1 = tex.SampleLevel(NearestRepeat, uvw2, 0).zxyw; // <+0, +y, +z, +yz>
		float4 f2 = tex.SampleLevel(NearestRepeat, uvw2 + d.xyy, 0).zxyw; // <+x, +xy, +xz, +xyz>
		float4 f3 = lerp(f1, f2, t.xxxx);  // f3 = <+0, +y, +z, +yz>
		float2 f4 = lerp(f3.xy, f3.zw, t.yy); // f4 = <+0, +z>
		float  f5 = lerp(f4.x, f4.y, t.z);

	return f5;
}

float NHQs(float3 uvw, Texture3D tex, float smooth = 1) {
	return NHQu(uvw, tex, smooth) * 2 - 1;
}

cbuffer CBChangeOnLod : register(b0)
{
	//旋转矩阵用来减少重复性
	float4x4 octaveMat6;
	float4x4 octaveMat7;
	//有LOD层次决定 1.0 2.0 4.0
	float  wsChunkSize = 4.0;
}

Texture3D noiseVol0 : register(t0);
Texture3D noiseVol1 : register(t1);
Texture3D noiseVol2 : register(t2);
Texture3D noiseVol3 : register(t3);
Texture3D packedNoiseVol0 : register(t4);
Texture3D packedNoiseVol1 : register(t5);
Texture3D packedNoiseVol2 : register(t6);
Texture3D packedNoiseVol3 : register(t7);

float3 rot(float3 coord, float4x4 mat)
{
	return float3(dot(coord,mat._11_21_31),   // 3x3 transform,
		dot(coord, mat._12_22_32),   // no translation
		dot(coord, mat._13_23_33));
}

float smooth_snap(float t, float m)
{
	// input: t in [0..1]
	// maps input to an output that goes from 0..1,
	// but spends most of its time at 0 or 1, except for
	// a quick, smooth jump from 0 to 1 around input values of 0.5.
	// the slope of the jump is roughly determined by 'm'.
	// note: 'm' shouldn't go over ~16 or so (precision breaks down).

	//float t1 =     pow((  t)*2, m)*0.5;
	//float t2 = 1 - pow((1-t)*2, m)*0.5;
	//return (t > 0.5) ? t2 : t1;

	// optimized:
	float c = (t > 0.5) ? 1 : 0;
	float s = 1 - c * 2;
	return c + s*pow((c + s*t) * 2, m)*0.5;
}

float density(float3 ws)
{
#if 0
	float3 ws_orig = ws;
	float den = 0;//负数为陆地,正数为空气
	//进行世界空间坐标系包装,减少抖动误差
	//超低频度采样用于高层次地形
	float4 uulf_rand = saturate(NMQu(ws*0.000718, noiseVol0) * 2 - 0.5);
		float4 uulf_rand2 = NMQu(ws*0.000632, noiseVol1);
		float4 uulf_rand3 = NMQu(ws*0.000695, noiseVol2);

		const float prewarp_str = 25;//~77593
	float3 ulf_rand = 0;
	ulf_rand.x = NHQs(ws*0.0041*0.971, packedNoiseVol2, 1)*0.64
		+ NHQs(ws*0.0041*0.461, packedNoiseVol3, 1)*0.32;
	ulf_rand.y = NHQs(ws*0.0041*0.997, packedNoiseVol1, 1)*0.64
		+ NHQs(ws*0.0041*0.453, packedNoiseVol0, 1)*0.32;
	ulf_rand.z = NHQs(ws*0.0041*1.032, packedNoiseVol3, 1)*0.64
		+ NHQs(ws*0.0041*0.511, packedNoiseVol2, 1)*0.32;

	ws += ulf_rand.xyz * prewarp_str * saturate(uulf_rand3.x*1.4 - 0.3);

	//增加一个地板,使其更像陆地而不是水底
	den = -ws.y;//在y=0放置一个平面
	den += saturate((-4 - ws_orig.y*0.3)*3.0) * 40 * uulf_rand2.z;//模拟沉淀

	float shelf_thickness_y = 2.5;//2.5;
	float shelf_pos_y = -1;//-2;
	float shelf_strength = 9.5;   // 1-4 is good

	den = lerp(den, shelf_strength, 0.83*saturate(shelf_thickness_y - abs(ws.y - shelf_pos_y)) 
			* saturate(uulf_rand.y*1.5 - 0.5));//阶丘地形与周围融合并下陷

	{
		const float terraces_can_warp = 0.5 * uulf_rand2.y;
		const float terrace_freq_y = 0.13;
		// careful - high str here diminishes strength of noise, etc.
		const float terrace_str = 3 * saturate(uulf_rand.z * 2 - 1);  
		// careful - too much here and LODs interfere
		const float overhang_str = 1 * saturate(uulf_rand.z * 2 - 1); 
		float fy = -lerp(ws_orig.y, ws.y, terraces_can_warp)*terrace_freq_y;
		float orig_t = frac(fy);
		float t = orig_t;
		t = t*t*(3 - 2 * t);  //smooth_snap faster than using 't = t*t*(3-2*t)' four times
		t = t*t*(3 - 2 * t);
		t = t*t*(3 - 2 * t);
		t = t*t*(3 - 2 * t);
		fy = floor(fy) + t;
		den += fy*terrace_str;
		den += (t - orig_t) * overhang_str;

		
	}
	//高频Y产生山脉
	den += NLQs(ws.xyz*float3(2, 27, 2)*0.0037, noiseVol0).x * 2 * saturate(uulf_rand2.w * 2 - 1);

	float3 c6 = rot(ws, octaveMat6);
	float3 c7 = rot(ws, octaveMat7);

	//9次采样富有低频特性(山脉.峡谷) 而保持了有趣的高频特性
	//同理,你可以加入c0-c5代替ws换取更多的随机性
	den +=
	(0
	+ NLQs(ws*0.3200*0.934, noiseVol3).x*0.16*1.20// skipped for long-range ambo
	+ NLQs(ws*0.1600*1.021, noiseVol1).x*0.32*1.16// skipped for long-range ambo
	+ NLQs(ws*0.0800*0.985, noiseVol2).x*0.64*1.12// skipped for long-range ambo
	+ NLQs(ws*0.0400*1.051, noiseVol0).x*1.28*1.08 // skipped for long-range ambo
	+ NLQs(ws*0.0200*1.020, noiseVol1).x*2.56*1.04
	+ NLQs(ws*0.0100*0.968, noiseVol3).x * 5
	+ NMQs(ws*0.0050*0.994, noiseVol0).x * 10 * 1.0 // MQ
	+ NMQs(c6*0.0025*1.045, noiseVol2).x * 20 * 0.9 // MQ
	+ NHQs(c7*0.0012*0.972, packedNoiseVol3).x * 40 * 0.8 // HQ and *rotated*!
	);

	den -= wsChunkSize.x*0.009;
#else
	float3 ws_orig = ws;
		
	float den = -ws.y;
	den += noiseVol0.Sample(LinearRepeat, ws).x*0.1;
#endif
	return den;
}