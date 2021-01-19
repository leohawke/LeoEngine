(effect
    (include Common.h)
    (float4x4 ProjectionMatrix)
    (texture2D texture0)
	(sampler sampler0)
    (shader
    "
        struct VS_INPUT
        {
            float2 pos : POSITION;
            float4 col : COLOR0;
            float2 uv  : TEXCOORD0;
        };
        
        struct PS_INPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR0;
            float2 uv  : TEXCOORD0;
        };
        
        PS_INPUT MainVS(VS_INPUT input)
        {
            PS_INPUT output;
            output.pos = mul(float4(input.pos.xy, 0.f, 1.f), ProjectionMatrix);
            output.col = input.col;
            output.uv  = input.uv;
            return output;
        }
       
        float4 MainPS(PS_INPUT input) : SV_Target
        {
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); 
            return out_col; 
        }
    "
    )
)