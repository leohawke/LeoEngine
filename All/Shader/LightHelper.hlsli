struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular; // w = SpecPower
};

interface iBaseLight
{
	float4 Ambient(float3 normal);
	float4 Diffuse(float3 normal, float3 pos, float3 toEye);
	float4 Specular(float3 normal, int sh, float3 pos, float3 toEye);
};


class DirectionLight : iBaseLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 dir;
	float pad;
	float4 Ambient(float3 normal)
	{
		return ambient;
	}
	float4 Diffuse(float3 normal, float3 pos, float3 toEye)
	{
		float lambert = saturate(dot(normal,-dir));
		return lambert*diffuse;
	}
	float4 Specular(float3 normal, int sh, float3 pos, float3 toEye)
	{
		float lambert = dot(normal, -dir);
		float ks = 0;
		float3 r = reflect(dir, normal);
		[flatten]
		if (lambert > 0.f)
		{
			ks = pow(saturate(dot(r, toEye)), sh);
		}
		return ks*specular;
	}
};

struct PointLight : iBaseLight
{
	float3 position;
	float range;

	float4 diffuse;

	float4 Ambient(float3 normal)
	{
		return 1.f;
	}
	float4 Diffuse(float3 normal, float3 pos, float3 toEye)
	{
		float3 lightVec = position - pos;
		float d = length(lightVec);
		lightVec /= d;
		float lambert = dot(lightVec, normal);
		if (d > range)
			lambert = 0.f;
		
		float kd = 0.f;

		[flatten]
		if (lambert > 0.f)
		{
			float ka = 1.0f;// dot(att, float3(1.0f, d, d*d));
			kd = lambert*ka;
		}
		return kd*diffuse;
	}
	float4 Specular(float3 normal, int sh, float3 pos, float3 toEye)
	{
		float3 lightVec = position - pos;
		float d = length(lightVec);
		lightVec /= d;
		float lambert = dot(lightVec, normal);
		if (d > range)
			lambert = 0.f;

		float ks = 0.f;
		[flatten]
		if (lambert > 0.f)
		{
			float ka = 1.0f; // dot(att, float3(1.0f, d, d*d));
			float3 v = reflect(-lightVec, normal);
			ks = pow(saturate(dot(v, toEye)), sh);
			ks = ks*ka;
		}
		return 1.f;//ks*specular;
	}
};

struct SpotLight : iBaseLight
{

	float3 position;
	float range;

	float4 diffuse;


	float3 direction;
	float spot;

	float4 Ambient(float3 normal)
	{
		return 1.f;
	}
	float4 Diffuse(float3 normal, float3 pos, float3 toEye)
	{
		float3 lightVec = position - pos;
		float d = length(lightVec);
		lightVec /= d;
		float lambert = dot(lightVec, normal);
		if (d > range)
			lambert = 0.f;

		float kd = 0.f;

		[flatten]
		if (lambert > 0.f)
		{
			float kspot = pow(saturate(dot(-lightVec, direction)), spot);
			float ka = kspot; // dot(att, float3(1.0f, d, d*d));
			kd = lambert*ka;
		}
		return kd*diffuse;
	}
	float4 Specular(float3 normal, int sh, float3 pos, float3 toEye)
	{
		float3 lightVec = position - pos;
		float d = length(lightVec);
		lightVec /= d;
		float lambert = dot(lightVec, normal);
		if (d > range)
			lambert = 0.f;

		float ks = 0.f;
		[flatten]
		if (lambert > 0.f)
		{
			float kspot = pow(saturate(dot(-lightVec, direction)), spot);
			float ka = kspot; // dot(att, float3(1.0f, d, d*d));
			float3 v = reflect(-lightVec, normal);
			ks = pow(saturate(dot(v, toEye)), sh);
			ks = ks*ka;
		}
		return ks*1.f;
	}
};

float3 NormalSampleCalc(float3 normalMapSample, float3 unitNormal, float3 tangent)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormal;
	float3 T = normalize(tangent - dot(tangent, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
	Texture2D shadowMap,
	float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	// Texel size.
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit /= 9.0f;
}