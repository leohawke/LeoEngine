1.//////////////////////////////////////////////////////////////////////////// 
2.// 
3.//  Crytek Engine Shader Source File 
4.//  Copyright (C), Crytek Studios, 2001-2007 
5.// ------------------------------------------------------------------------- 
6.//  File name:   AmbientOcclusion.cfx 
7.//  Version:     v1.00 
8.//  Created:     04/12/2006 by Vladimir Kajalin 
9.//  Description: Implementation of SSAO, TerrainAO (2.5 D maps), Fill lights 
10.// ------------------------------------------------------------------------- 
//  History: 
// 
13.//////////////////////////////////////////////////////////////////////////// 
14.
15.#include "Common.cfi"
16.#include "ModificatorVT.cfi"
17.
18.// Shader global descriptions 
19.float Script : STANDARDSGLOBAL
20.<
21.  string Script =
22.           "NoPreview;"
23.           "LocalConstants;"
24.           "ShaderDrawType = Custom;"
25.           "ShaderType = PostProcess;"
26.>;
27.
28.// original depth target 
29.sampler2D sceneDepthSampler = sampler_state
30.{
	31.Texture = $ZTarget;
	32.MinFilter = POINT;
	33.MagFilter = POINT;
	34.MipFilter = POINT;
	35.AddressU = Clamp;
	36.AddressV = Clamp;
	37.};
38.
39.// downscaled depth target 
40.sampler2D sceneDepthSamplerAO = sampler_state
41.{
	42.Texture = $ZTargetScaled;
	43.MinFilter = POINT;
	44.MagFilter = POINT;
	45.MipFilter = POINT;
	46.AddressU = Clamp;
	47.AddressV = Clamp;
	48.};
49.
50.sampler2D depthTargetSampler   : register(s0);
51.sampler2D TerrainInfoSampler0  : register(s1);
52.sampler2D TerrainInfoSampler1  : register(s2);
53.
54.#include "ShadowCommon.cfi"
55.
56.//=========================================================================== 
57.
58.float4 AOSectorRange;
59.float4 TerrainAOInfo;
60.float4 FillLightPos;
61.float4 FillLightColor;
62.float4 SSAO_params
63.float4x4 CompMatrix : PI_Composite;
64.
65.///////////////////////////// 
66.// structs 
67.
68.struct pixout_cl
69.{
	70.  float4 Color  : COLOR0;
	71.};
72.
73.struct vert2fragSSAO
74.{
	75.        float4 HPosition        :       POSITION;
	76.        float4 ScreenTC  :        TEXCOORD0;
	77.        float3 WS_ViewVect:     TEXCOORD1;
	78.};
79.
80.struct app2vertShadow
81.{
	82.  IN_P
		83.  IN_TBASE
		84.  float3 viewDir : TEXCOORD1;
	85.};
86.
87.vert2fragSSAO Deferred_SSAO_Pass_VS(app2vertShadow IN)
88.{
	89.        vert2fragSSAO OUT;
	90.#ifndef OPENGL
		91.        OUT = (vert2fragSSAO)0;
	92.#endif
		93.
		94.        OUT.HPosition = mul(CompMatrix, IN.Position);
	95.        OUT.ScreenTC.xy = IN.baseTC.xy;
	96.        OUT.ScreenTC.zw = IN.baseTC.xy*g_VS_ScreenSize.xy / 4;
	97.        OUT.WS_ViewVect = IN.viewDir;
	98.
		99.        return OUT;
	100.}
101.
102.///////////////// shadows pixel shader ////////////////// 
103./*pixout_cl AmbientShadowMaskPS(vert2fragShadowNew IN)
	104.{
	105.  pixout_cl OUT;
	106.
	107.  float SceneDepth = tex2D( sceneDepthSampler, IN.ScreenTC.xy ).r;
	108.
	109.        float2 randTC;
	110.        randTC.xy = IN.ScreenTC.xy*256;
	111.        float3 rotSample = tex2D(sRotSampler AO, randTC+float2(SceneDepth,SceneDepth)).rgb;
	112.  rotSample = 2.0 * rotSample - 1.0;
	113.  rotSample.xyz = normalize(rotSample.xyz);
	114.
	115.        // define kernel
	116.        float scale = 0.02;
	117.        float3 irreg_kernel[8] =
	118.        {
	119.                float3(0.527837, -0.085868 ,0.527837)  * scale,
	120.                float3(-0.040088, 0.536087, -0.040088)  * scale,
	121.                float3(-0.670445, -0.179949, -0.670445)  * scale,
	122.                float3(-0.419418, -0.616039, -0.419418)  * scale,
	123.                float3(0.440453, -0.639399, 0.440453) * scale,
	124.                float3(-0.757088, 0.349334, -0.757088) * scale,
	125.                float3(0.574619, 0.685879,0.574619) * scale,
	126.                float3(0.03851, -0.939059, 0.03851) * scale
	127.        };
	128.
	129.        // get rotatiojn matrix
	130.        float3x3 rotMat;
	131.        GetRotationV0Terrain(rotSample, rotMat);
	132.
	133.        float fDistScale = 0.5 / saturate(SceneDepth);
	134.
	135.        // sample
	136.        float fSkyAccess = 0;
	137.        for(float i=0; i<8; i++)
	138.        {
	139.                float3 irregSample = mul(irreg_kernel, rotMat);
	140.                float2 ScreenTC2 = IN.ScreenTC.xy + irregSample.xy * fDistScale;
	141.                float SceneDepth2 = tex2D( sceneDepthSampler, ScreenTC2 ).r;
	142.                float fRange = abs(SceneDepth2-SceneDepth)*12;
	143.                fSkyAccess += lerp(saturate(SceneDepth2>(SceneDepth+irregSample.z)), .75, saturate(fRange));
	144.        }
	145.
	146.        OUT.Color = saturate(fSkyAccess*0.25);
	147.        return OUT;
	148.}*/
	149.
	150.sampler2D sRotSampler4x4_16 = sampler_state
	151.{
	152.        Texture = $16PointsOnSphere;
	153.        MinFilter = POINT;
	154.        MagFilter = POINT;
	155.        MipFilter = NONE;
	156.        AddressU = Wrap;
	157.        AddressV = Wrap;
	158.};
159./*
	160.sampler2D sRotSampler4x4 = sampler_state
	161.{
	162.  Texture = Textures/Defaults/noise4x4.dds;
	163.  MinFilter = POINT;
	164.  MagFilter = POINT;
	165.  MipFilter = NONE;
	166.  AddressU = Wrap;
	167.  AddressV = Wrap;
	168.};
	169.*/
	170.// vPlane should be normalized 
	171.// the returned vector has the same length as vDir 
	172.float3 mirror(float3 vDir, float3 vPlane)
	173.{
	174.  return vDir - 2 * vPlane * dot(vPlane, vDir);
	175.}
176./*
	177.float GetVarianceAO(sampler2D depthMap, float3 p)
	178.{
	179.  float2 moments = tex2D( depthMap, p.xy ).xy;
	180.
	181.        // Variance shadow mapping
	182.  float M = moments.r; //mean
	183.
	184.  //TD invesigate: we calculate variance in non-shifted by 0.5 space here
	185.  //is it correct
	186.        float E_x2 = moments.g;
	187.        float Ex_2 = M * M;
	188.        float variance = (E_x2 - Ex_2);//decrease range of variance to increase precision for 16-bit formats
	189.
	190.  float m_d = M - p.z;
	191.        float p_max = variance / (variance + m_d * m_d);
	192.
	193.        // Standard shadow map comparison
	194.        float lit_factor = m_d > -0.001;
	195.
	196.        //select properly shadow region because of one-tailed version of inequality
	197.        float shadow = max(lit_factor, p_max);
	198.        return shadow;
	199.}
	200.*/
	201./*
		202.
		203.sampler2D SSAO_Sampler_0  : register(s4);
		204.{
		205.        MinFilter = LINEAR;
		206.        MagFilter = LINEAR;
		207.        MipFilter = NONE;
		208.        AddressU = Wrap;
		209.        AddressV = Wrap;
		210.};
		211.
		212.sampler2D SSAO_Sampler_1  : register(s5);
		213.{
		214.        MinFilter = LINEAR;
		215.        MagFilter = LINEAR;
		216.        MipFilter = NONE;
		217.        AddressU = Wrap;
		218.        AddressV = Wrap;
		219.};
		220.
		221.sampler2D SSAO_Sampler_2  : register(s6);
		222.{
		223.        MinFilter = LINEAR;
		224.        MagFilter = LINEAR;
		225.        MipFilter = NONE;
		226.        AddressU = Wrap;
		227.        AddressV = Wrap;
		228.};
		229.
		230.sampler2D SSAO_Sampler_3  : register(s7);
		231.{
		232.        MinFilter = LINEAR;
		233.        MagFilter = LINEAR;
		234.        MipFilter = NONE;
		235.        AddressU = Wrap;
		236.        AddressV = Wrap;
		237.};
		238.
		239.sampler2D SSAO_Sampler_4  : register(s8);
		240.{
		241.        MinFilter = LINEAR;
		242.        MagFilter = LINEAR;
		243.        MipFilter = NONE;
		244.        AddressU = Wrap;
		245.        AddressV = Wrap;
		246.};
		247.
		248.sampler2D SSAO_Sampler_5  : register(s9);
		249.{
		250.        MinFilter = LINEAR;
		251.        MagFilter = LINEAR;
		252.        MipFilter = NONE;
		253.        AddressU = Wrap;
		254.        AddressV = Wrap;
		255.};
		256.
		257.sampler2D SSAO_Sampler_6  : register(s10);
		258.{
		259.        MinFilter = LINEAR;
		260.        MagFilter = LINEAR;
		261.        MipFilter = NONE;
		262.        AddressU = Wrap;
		263.        AddressV = Wrap;
		264.};
		265.
		266.sampler2D SSAO_Sampler_7  : register(s11);
		267.{
		268.        MinFilter = LINEAR;
		269.        MagFilter = LINEAR;
		270.        MipFilter = NONE;
		271.        AddressU = Wrap;
		272.        AddressV = Wrap;
		273.};
		274.
		275.pixout_cl Deferred_SSAO_Pass_PS_DepthBlurBased(vert2fragSSAO IN)
		276.{
		277.  pixout_cl OUT;
		278.
		279.//  float3 p = float3( IN.ScreenTC.xy, tex2D( sceneDepthSampler, IN.ScreenTC.xy ).r );
		280.  //OUT.Color = GetVarianceAO( SSAO_Sampler_3, p );
		281.
		282.  float fSceneDepth = tex2D( sceneDepthSampler, IN.ScreenTC.xy ).r;
		283.
		284.  float4 arrSceneDepthBlur[8];
		285.  arrSceneDepthBlur[0] = tex2D( SSAO_Sampler_0, IN.ScreenTC.xy );
		286.  arrSceneDepthBlur[1] = tex2D( SSAO_Sampler_1, IN.ScreenTC.xy );
		287.  arrSceneDepthBlur[2] = tex2D( SSAO_Sampler_2, IN.ScreenTC.xy );
		288.  arrSceneDepthBlur[3] = tex2D( SSAO_Sampler_3, IN.ScreenTC.xy );
		289.  arrSceneDepthBlur[4] = tex2D( SSAO_Sampler_4, IN.ScreenTC.xy );
		290.  arrSceneDepthBlur[5] = tex2D( SSAO_Sampler_5, IN.ScreenTC.xy );
		291.  arrSceneDepthBlur[6] = tex2D( SSAO_Sampler_6, IN.ScreenTC.xy );
		292.  arrSceneDepthBlur[7] = tex2D( SSAO_Sampler_7, IN.ScreenTC.xy );
		293.
		294.  OUT.Color = 0;
		295.
		296.  int nSamples = 3;
		297.  int nStart = 0;
		298.
		299.  for(int t=nStart; t<nStart+nSamples; t++)
		300.  {
		301.    float2 blurredDepth = arrSceneDepthBlur[t].xy;
		302.          float fFix = saturate((fSceneDepth-blurredDepth.g-0.01)*(146.f / (1.f+t*2.f)));
		303.    float fRes = 0;
		304.          fRes += fFix;
		305.    fRes += saturate(1-(fSceneDepth*0.999f - blurredDepth.r)*400.f*saturate(1.f-blurredDepth.g));
		306.    fRes = saturate(fRes);
		307.
		308.    OUT.Color += fRes/nSamples;
		309.  }
		310.
		311.  return OUT;
		312.}
		313.
		314.technique Deferred_SSAO_Pass_DepthBlurBased
		315.{
		316.        //Shadow pass
		317.  pass p0
		318.  {
		319.    VertexShader = compile vs_2_0 Deferred_SSAO_Pass_VS();
		320.
		321.    ZEnable = false;
		322.    ZWriteEnable = false;
		323.    CullMode = None;
		324.
		325.    PixelShader = compile ps_2_x Deferred_SSAO_Pass_PS_DepthBlurBased();
		326.  }
		327.}
		328.*/
		329.
		330./* SSIL test by Vlad
			331.pixout_cl Deferred_SSIL_Pass_PS(vert2fragSSAO IN)
			332.{
			333.  pixout_cl OUT;
			334.
			335.        // define kernelw
			336.        const half step = 1.4;//(1.f - 1.f/8.f);
			337.        float n = .1;
			338.        const half fScale = 0.025f/2;
			339.
			340.  const int nCount = 16;
			341.
			342.        const half3 arrKernel[nCount] =
			343.        {
			344.                float3(0.527837, -0.085868 ,0.527837)  *fScale*(n*=step),
			345.                float3(-0.040088, 0.536087, -0.040088)  *fScale*(n*=step),
			346.                float3(-0.670445, -0.179949, -0.670445)  *fScale*(n*=step),
			347.                float3(-0.419418, -0.616039, -0.419418)  *fScale*(n*=step),
			348.                float3(0.440453, -0.639399, 0.440453) *fScale*(n*=step),
			349.                float3(-0.757088, 0.349334, -0.757088) *fScale*(n*=step),
			350.                float3(0.574619, 0.685879,0.574619) *fScale*(n*=step),
			351.                float3(0.03851, -0.939059, 0.03851) *fScale*(n*=step),
			352.                float3(0.527837, -0.085868 ,0.527837)  *fScale*(n*=step),
			353.                float3(-0.040088, 0.536087, -0.040088)  *fScale*(n*=step),
			354.                float3(-0.670445, -0.179949, -0.670445)  *fScale*(n*=step),
			355.                float3(-0.419418, -0.616039, -0.419418)  *fScale*(n*=step),
			356.                float3(0.440453, -0.639399, 0.440453) *fScale*(n*=step),
			357.                float3(-0.757088, 0.349334, -0.757088) *fScale*(n*=step),
			358.                float3(0.574619, 0.685879,0.574619) *fScale*(n*=step),
			359.                float3(0.03851, -0.939059, 0.03851) *fScale*(n*=step),
			360.        };
			361.
			362.        // create random rot matrix
			363.        half3 rotSample = tex2D(sRotSampler4x4_16, IN.ScreenTC.zw).rgb;
			364.        rotSample = normalize((2.0 * rotSample - 1.0));
			365.
			366.  // read actual RGB and depth
			367.  float4 vRootNNND = tex2D( sceneDepthSampler, IN.ScreenTC.xy ).yzwx;
			368.  half fSceneDepth = vRootNNND.w;
			369.
			370.  // get root pixel normal
			371.//  float fNormScale = 256; // same as in TexToTexPS
			372.        //half3 WSPos = float3(IN.ScreenTC.xy, fSceneDepth*fNormScale);
			373.  float3 vNorm = vRootNNND.xyz;//normalize(cross(ddx(WSPos),ddy(WSPos)));
			374.
			375.        // range/scale conversions
			376.  half fSceneDepthM = fSceneDepth * PS_NearFarClipDist.y;
			377.        half3 vSampleScale = SSAO_params.zzw*2;
			378.  float fDepthRangeScale = PS_NearFarClipDist.y / vSampleScale.z * 0.85f;
			379.        vSampleScale.xy *= 1.0f / fSceneDepthM;
			380.        vSampleScale.z  *= 2.0f / PS_NearFarClipDist.y;
			381.
			382.  float fDepthTestSoftness = 8.f/vSampleScale.z;
			383.
			384.  half4 vIndLight = 0.f;
			385.
			386.  float3 vRGB0 = tex2D( sceneBackBufferSampler, IN.ScreenTC.xy );
			387.
			388.  for(int i=0; i<nCount; i++)
			389.  {
			390.    half3 vIrrSample = mirror(arrKernel, rotSample) * vSampleScale;
			391.
			392.    float2 tc = IN.ScreenTC.xy + vIrrSample.xy;
			393.
			394.    float3 vRGB = tex2D( sceneBackBufferSampler, tc );
			395.
			396.    float4 vNNND = tex2D( sceneDepthSampler, tc ).yzwx;
			397.
			398.    vNNND.a += vIrrSample.z;
			399.
			400.    float fDepthTest = 1-saturate(0.75*abs(vNNND.a-fSceneDepth)*fDepthTestSoftness + 0.25*(vNNND.a-fSceneDepth)*fDepthTestSoftness);
			401.
			402.    half fDistance = fSceneDepth - vNNND.a;
			403.
			404.    float fDot = saturate((dot(-vNorm,vNNND.xyz)+1.0f));
			405.
			406.    float fDistanceScaled = fDistance * fDepthRangeScale * 2;
			407.
			408.    float fRangeIsValid = 1;//1-saturate( abs(fDistanceScaled) );
			409.
			410.    {
			411.      float fSceneDepth2 = vNNND.a;
			412.                  float fRangeIsInvalid = saturate( abs(fSceneDepth-fSceneDepth2) * fDepthRangeScale );
			413.      fRangeIsInvalid = (saturate( abs(fDistanceScaled) ) + saturate( fDistanceScaled ))/2;
			414.      vIndLight.a += lerp(saturate((fSceneDepth2-fSceneDepth)*fDepthTestSoftness*16), 1, fRangeIsInvalid);
			415.    }
			416.
			417.//    vIndLight.rgb += lerp(1.f,vRGB*2-1,sqrt(fRangeIsValid*fDepthTest*fDot));
			418.    vIndLight.rgb += lerp(1,vRGB-0.33,sqrt(fDepthTest*fDot*4));
			419.  }
			420.
			421.  vIndLight.rgb = 3*(vIndLight.rgb - (vIndLight.rgb.x+vIndLight.rgb.y+vIndLight.rgb.z)*0.3333f)
			422.    + (vIndLight.rgb.x+vIndLight.rgb.y+vIndLight.rgb.z)*0.3333f;
			423.
			424.  vIndLight = vIndLight / nCount;
			425.
			426.  OUT.Color.rgb = lerp(saturate(vIndLight.xyz), 1, saturate(vIndLight.a*6-2));
			427.//  OUT.Color.rgb = saturate(vIndLight.a*4-1.3);
			428.//  OUT.Color.rgb = saturate(vIndLight.rgb);
			429.  OUT.Color.a = 1;
			430.
			431.        return OUT;
			432.}*/
			433.
			434.pixout_cl Deferred_SSAO_Pass_PS(vert2fragSSAO IN)
			435.{
	436.  pixout_cl OUT;
	437.
		438.        // define kernel 
		439.        const half step = 1.f - 1.f / 8.f;
	440.        half n = 0;
	441.        const half fScale = 0.025f;
	442.        const half3 arrKernel[8] =
		443.        {
		444.                normalize(half3(1, 1, 1))*fScale*(n += step),
			445.                normalize(half3(-1, -1, -1))*fScale*(n += step),
			446.                normalize(half3(-1, -1, 1))*fScale*(n += step),
			447.                normalize(half3(-1, 1, -1))*fScale*(n += step),
			448.                normalize(half3(-1, 1, 1))*fScale*(n += step),
			449.                normalize(half3(1, -1, -1))*fScale*(n += step),
			450.                normalize(half3(1, -1, 1))*fScale*(n += step),
			451.                normalize(half3(1, 1, -1))*fScale*(n += step),
			452.        };
	453.
		454.        // create random rot matrix 
		455.        half3 rotSample = tex2D(sRotSampler4x4_16, IN.ScreenTC.zw).rgb;
	456.        rotSample = (2.0 * rotSample - 1.0);
	457.
		458.  half fSceneDepth = tex2D(sceneDepthSampler, IN.ScreenTC.xy).r;
	459.
		460.        // range conversions 
		461.  half fSceneDepthM = fSceneDepth * PS_NearFarClipDist.y;
	462.
		463.        half3 vSampleScale = SSAO_params.zzw
		464.                * saturate(fSceneDepthM / 5.3f) // make area smaller if distance less than 5 meters 
		465.    * (1.f + fSceneDepthM / 8.f); // make area bigger if distance more than 32 meters 
	466.
		467.  float fDepthRangeScale = PS_NearFarClipDist.y / vSampleScale.z * 0.85f;
	468.
		469.        // convert from meters into SS units 
		470.        vSampleScale.xy *= 1.0f / fSceneDepthM;
	471.        vSampleScale.z *= 2.0f / PS_NearFarClipDist.y;
	472.
		473.  float fDepthTestSoftness = 64.f / vSampleScale.z;
	474.
		475.        // sample 
		476.  half4 vSkyAccess = 0.f;
	477.  half4 arrSceneDepth2[2];
	478.  half3 vIrrSample;
	479.  half4 vDistance;
	480.  float4 fRangeIsInvalid;
	481.
		482.  const half bHQ = (GetShaderQuality() == QUALITY_HIGH);
	483.
		484.  float fHQScale = 0.5f;
	485.
		486.  for (int i = 0; i < 2; i++)
		487.  {
		488.    vIrrSample = mirror(arrKernel[i * 4 + 0], rotSample) * vSampleScale;
		489.    arrSceneDepth2[0].x = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		490.    if (bHQ)
			491.    {
			492.      vIrrSample.xyz *= fHQScale;
			493.      arrSceneDepth2[1].x = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
			494.    }
		495.
			496.    vIrrSample = mirror(arrKernel[i * 4 + 1], rotSample) * vSampleScale;
		497.    arrSceneDepth2[0].y = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		498.    if (bHQ)
			499.    {
			500.      vIrrSample.xyz *= fHQScale;
			501.      arrSceneDepth2[1].y = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
			502.    }
		503.
			504.    vIrrSample = mirror(arrKernel[i * 4 + 2], rotSample) * vSampleScale;
		505.    arrSceneDepth2[0].z = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		506.    if (bHQ)
			507.    {
			508.      vIrrSample.xyz *= fHQScale;
			509.      arrSceneDepth2[1].z = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
			510.    }
		511.
			512.    vIrrSample = mirror(arrKernel[i * 4 + 3], rotSample) * vSampleScale;
		513.    arrSceneDepth2[0].w = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		514.    if (bHQ)
			515.    {
			516.      vIrrSample.xyz *= fHQScale;
			517.      arrSceneDepth2[1].w = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
			518.    }
		519.
			520.    float fDefVal = 0.55f;
		521.
			522.    for (int s = 0; s < (bHQ ? 2 : 1); s++)
			523.    {
			524.      vDistance = fSceneDepth - arrSceneDepth2;
			525.      float4 vDistanceScaled = vDistance * fDepthRangeScale;
			526.      fRangeIsInvalid = (saturate(abs(vDistanceScaled)) + saturate(vDistanceScaled)) / 2;
			527.      vSkyAccess += lerp(saturate((-vDistance)*fDepthTestSoftness), fDefVal, fRangeIsInvalid);
			528.    }
		529.  }
	530.
		531.  OUT.Color = dot(vSkyAccess, (bHQ ? 1 / 16.0f : 1 / 8.0f)*2.0) - SSAO_params.y; // 0.075f 
	532.  OUT.Color = saturate(lerp(0.9f, OUT.Color, SSAO_params.x));
	533.
		534.        return OUT;
	535.}
536.
537./*pixout_cl Deferred_SSAO_Pass_PS_Ref(vert2fragSSAO IN)
	538.{
	539.  pixout_cl OUT;
	540.
	541.        // define kernel
	542.        const half step = 1.f - 1.f/8.f;
	543.        half n = 0;
	544.        const half fScale = 0.025f;
	545.        const half3 arrKernel[8] =
	546.        {
	547.                normalize(half3( 1, 1, 1))*fScale*(n+=step),
	548.                normalize(half3(-1,-1,-1))*fScale*(n+=step),
	549.                normalize(half3(-1,-1, 1))*fScale*(n+=step),
	550.                normalize(half3(-1, 1,-1))*fScale*(n+=step),
	551.                normalize(half3(-1, 1 ,1))*fScale*(n+=step),
	552.                normalize(half3( 1,-1,-1))*fScale*(n+=step),
	553.                normalize(half3( 1,-1, 1))*fScale*(n+=step),
	554.                normalize(half3( 1, 1,-1))*fScale*(n+=step),
	555.        };
	556.
	557.        // create random rot matrix
	558.        half3 rotSample = tex2D(sRotSampler4x4_16, IN.ScreenTC.zw).rgb;
	559.        rotSample = normalize(2.0 * rotSample - 1.0);
	560.        half3x3 rotMat;
	561.        GetRotationV0(rotSample, rotMat);
	562.
	563.  half fSceneDepth = tex2D( sceneDepthSampler, IN.ScreenTC.xy ).r;
	564.
	565.        // range conversions
	566.  half fSceneDepthM = fSceneDepth * PS_NearFarClipDist.y;
	567.
	568.  // define sampling area size
	569.        half3 vSampleScale = SSAO_params.zzw
	570.                * saturate(fSceneDepthM / 5.3f) // make area smaller if distance less than 5 meters
	571.    * (1.f + fSceneDepthM / 8.f ); // make area bigger if distance more than 32 meters
	572.
	573.        float fDepthRangeScale = PS_NearFarClipDist.y / vSampleScale.z * 0.75f;
	574.
	575.        // convert from meters into SS units
	576.        vSampleScale.xy *= 1.0f / fSceneDepthM;
	577.        vSampleScale.z  *= 2.0f / PS_NearFarClipDist.y;
	578.
	579.        // sample
	580.        half fSkyAccess = 0.f;
	581.  float fDepthTestSoftness = 32.f/vSampleScale.z;
	582.        half3 irregSample;
	583.  half fSceneDepth2;
	584.  float fRangeIsInvalid;
	585.        for(int i=0; i<8; i++)
	586.        {
	587.                irregSample = mul(arrKernel, rotMat) * vSampleScale;
	588.    fSceneDepth2 = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + irregSample.xy ).r-irregSample.z;
	589.                fRangeIsInvalid = saturate( abs(fSceneDepth-fSceneDepth2) * fDepthRangeScale );
	590.    fSkyAccess += lerp(saturate((fSceneDepth2-fSceneDepth)*fDepthTestSoftness), 0.6f, fRangeIsInvalid);
	591.  }
	592.
	593.        OUT.Color = saturate(1.3f+((SSAO_params.x-1.f)*0.35)-pow(1.f-fSkyAccess/7.0f,3)*2.5f*SSAO_params.x);
	594.
	595.        return OUT;
	596.}*/
	597.
	598.//////////////////////////////// technique //////////////// 
	599.
	600.pixout_cl Deferred_FillLight_Pass_PS(vert2fragSSAO IN)
	601.{
	602.  pixout_cl OUT;
	603.
		604.  // reconstruct WS position 
		605.  half SceneDepth = tex2D(depthTargetSampler, IN.ScreenTC.xy).r;
	606.        half3 WSPos = vfViewPos.xyz + IN.WS_ViewVect * SceneDepth;
	607.
		608.  // simple lighting 
		609.  half3 vLightDir = FillLightPos.xyz - WSPos.xyz;
	610.  OUT.Color = saturate(1.f - length(vLightDir) / FillLightPos.w);
	611.
		612.#if %USE_SM30
		613.  float NdotL = saturate(dot(normalize(vLightDir), normalize(cross(ddy(WSPos), ddx(WSPos)))));
	614.  OUT.Color *= (NdotL*0.6666f + 0.3333f);
	615.#else
		616.  OUT.Color = pow(OUT.Color, 2);
	617.#endif
		618.
		619.  // range scale 
		620.  OUT.Color *= FillLightColor.x / 8.f;
	621.
		622.  return OUT;
	623.}
624.
625.pixout_cl Deferred_TerrainAO_Pass_PS(vert2fragSSAO IN)
626.{
	627.  pixout_cl OUT;
	628.
		629.  // reconstruct pixel world position 
		630.  half SceneDepth = tex2D(depthTargetSampler, IN.ScreenTC.xy).r;
	631.        float3 vWSPos = vfViewPos.xyz + IN.WS_ViewVect * SceneDepth;
	632.
		633.  // find terrain texture coordinates 
		634.  float2 texCoord = float2((vWSPos.y - AOSectorRange.y), (vWSPos.x - AOSectorRange.x)) * TerrainAOInfo.w;
	635.
		636.  // get terrain and vegetation elevations 
		637.        half4 dataS0 = tex2D(TerrainInfoSampler0, texCoord);
	638.        half4 dataS1 = tex2D(TerrainInfoSampler1, texCoord);
	639.        half fTerrainZ = dataS1.a*(AOSectorRange.w - AOSectorRange.z) + AOSectorRange.z;
	640.        half fVegetZMax = fTerrainZ + dataS1.g*32.f;
	641.
		642.  // get initial sky amount, TODO: try pow() here 
		643.        OUT.Color = saturate(1.f - TerrainAOInfo.g*(fVegetZMax - vWSPos.z));
	644.
		645.  // scale based on sky amount precomputed for terrain 
		646.        half fTerrainSkyAmount = dataS0.a * saturate(1.f - (fTerrainZ - vWSPos.z)*0.025f);
	647.  OUT.Color = lerp(OUT.Color, 1.f, fTerrainSkyAmount);
	648.
		649.  // lerp into pure terrain sky amount near the ground 
		650.  half fHeightFactor = saturate((vWSPos.z - fTerrainZ)*0.5f);
	651.  OUT.Color = lerp(fTerrainSkyAmount, OUT.Color, fHeightFactor);
	652.
		653.  // apply sky brightening and fade on distance 
		654.  half fDistAtt = saturate(pow(SceneDepth*PS_NearFarClipDist.y / 1024.f, 3));
	655.        OUT.Color = lerp(1.f, OUT.Color, (1.f - TerrainAOInfo.r)*(1.f - fDistAtt));
	656.
		657.  return OUT;
	658.}
659.
660.technique Deferred_SSAO_Pass
661.{
	662.        //Shadow pass 
		663.  pass p0
		664.  {
		665.    VertexShader = compile vs_2_0 Deferred_SSAO_Pass_VS();
		666.
			667.    ZEnable = false;
		668.    ZWriteEnable = false;
		669.    CullMode = None;
		670.
			671.    PixelShader = compile ps_2_x Deferred_SSAO_Pass_PS();
		672.  }
	673.}
674.
675.technique Deferred_TerrainAO_Pass
676.{
	677.        //Shadow pass 
		678.  pass p0
		679.  {
		680.    VertexShader = compile vs_2_0 Deferred_SSAO_Pass_VS();
		681.
			682.    ZEnable = false;
		683.    ZWriteEnable = false;
		684.    CullMode = None;
		685.
			686.    PixelShader = compile ps_2_0 Deferred_TerrainAO_Pass_PS();
		687.  }
	688.}
technique Deferred_FillLight_Pass
{
	//Shadow pass 
	pass p0
	{
		VertexShader = compile vs_2_0 Deferred_SSAO_Pass_VS();

		ZEnable = false;
		ZWriteEnable = false;
		CullMode = None;

#if %USE_SM30
		PixelShader = compile ps_3_0 Deferred_FillLight_Pass_PS();
#else
		PixelShader = compile ps_2_x Deferred_FillLight_Pass_PS();
#endif

	}
}
sampler2D sRotSampler4x4_16 = sampler_state
{
        Texture = $16PointsOnSphere;
	      MinFilter = POINT;
      MagFilter = POINT;
	     MipFilter = NONE;
	       AddressU = Wrap;
	       AddressV = Wrap;
};


.struct vert2fragSSAO
{
     float4 HPosition        :       POSITION;
      float4 ScreenTC  :        TEXCOORD0;
	       float3 WS_ViewVect:     TEXCOORD1;
	};

struct app2vertShadow
{
  IN_P
IN_TBASE
	float3 viewDir : TEXCOORD1;
};

vert2fragSSAO Deferred_SSAO_Pass_VS(app2vertShadow IN)
{
     vert2fragSSAO OUT;

	
	  OUT.HPosition = mul(CompMatrix, IN.Position);
      OUT.ScreenTC.xy = IN.baseTC.xy;
      OUT.ScreenTC.zw = IN.baseTC.xy*g_VS_ScreenSize.xy / 4;
	  OUT.WS_ViewVect = IN.viewDir;
	
		return OUT;
}

pixout_cl Deferred_SSAO_Pass_PS(vert2fragSSAO IN)
{
	pixout_cl OUT;

	// define kernel 
	const half step = 1.f - 1.f / 8.f;
	half n = 0;
	const half fScale = 0.025f;
	const half3 arrKernel[8] =
	{
		normalize(half3(1, 1, 1))*fScale*(n += step),
		normalize(half3(-1, -1, -1))*fScale*(n += step),
		normalize(half3(-1, -1, 1))*fScale*(n += step),
		normalize(half3(-1, 1, -1))*fScale*(n += step),
		normalize(half3(-1, 1, 1))*fScale*(n += step),
		normalize(half3(1, -1, -1))*fScale*(n += step),
		normalize(half3(1, -1, 1))*fScale*(n += step),
		normalize(half3(1, 1, -1))*fScale*(n += step),
	};

	// create random rot matrix 
	half3 rotSample = tex2D(sRotSampler4x4_16, IN.ScreenTC.zw).rgb;
	rotSample = (2.0 * rotSample - 1.0);

	half fSceneDepth = tex2D(sceneDepthSampler, IN.ScreenTC.xy).r;

	// range conversions 
	half fSceneDepthM = fSceneDepth * PS_NearFarClipDist.y;

	half3 vSampleScale = SSAO_params.zzw
		* saturate(fSceneDepthM / 5.3f) // make area smaller if distance less than 5 meters 
		* (1.f + fSceneDepthM / 8.f); // make area bigger if distance more than 32 meters 

	float fDepthRangeScale = PS_NearFarClipDist.y / vSampleScale.z * 0.85f;

	// convert from meters into SS units 
	vSampleScale.xy *= 1.0f / fSceneDepthM;
	vSampleScale.z *= 2.0f / PS_NearFarClipDist.y;

	float fDepthTestSoftness = 64.f / vSampleScale.z;

	// sample 
	half4 vSkyAccess = 0.f;
	half4 arrSceneDepth2[2];
	half3 vIrrSample;
	half4 vDistance;
	float4 fRangeIsInvalid;

	const half bHQ = (GetShaderQuality() == QUALITY_HIGH);

	float fHQScale = 0.5f;

	for (int i = 0; i < 2; i++)
	{
		vIrrSample = mirror(arrKernel[i * 4 + 0], rotSample) * vSampleScale;
		arrSceneDepth2[0].x = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		if (bHQ)
		{
			vIrrSample.xyz *= fHQScale;
			arrSceneDepth2[1].x = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		}

		vIrrSample = mirror(arrKernel[i * 4 + 1], rotSample) * vSampleScale;
		arrSceneDepth2[0].y = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		if (bHQ)
		{
			vIrrSample.xyz *= fHQScale;
			arrSceneDepth2[1].y = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		}

		vIrrSample = mirror(arrKernel[i * 4 + 2], rotSample) * vSampleScale;
		arrSceneDepth2[0].z = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		if (bHQ)
		{
			vIrrSample.xyz *= fHQScale;
			arrSceneDepth2[1].z = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		}

		vIrrSample = mirror(arrKernel[i * 4 + 3], rotSample) * vSampleScale;
		arrSceneDepth2[0].w = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		if (bHQ)
		{
			vIrrSample.xyz *= fHQScale;
			arrSceneDepth2[1].w = tex2D(sceneDepthSamplerAO, IN.ScreenTC.xy + vIrrSample.xy).r + vIrrSample.z;
		}

		float fDefVal = 0.55f;

		for (int s = 0; s < (bHQ ? 2 : 1); s++)
		{
			vDistance = fSceneDepth - arrSceneDepth2[s];
			float4 vDistanceScaled = vDistance * fDepthRangeScale;
			fRangeIsInvalid = (saturate(abs(vDistanceScaled)) + saturate(vDistanceScaled)) / 2;
			vSkyAccess += lerp(saturate((-vDistance)*fDepthTestSoftness), fDefVal, fRangeIsInvalid);
		}
	}

	OUT.Color = dot(vSkyAccess, (bHQ ? 1 / 16.0f : 1 / 8.0f)*2.0) - SSAO_params.y; // 0.075f 
	OUT.Color = saturate(lerp(0.9f, OUT.Color, SSAO_params.x));

	return OUT;
}
