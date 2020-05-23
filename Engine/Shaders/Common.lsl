(effect
    (shader
    "
        /** Output of the screen vertex shader. */
        struct ScreenVertexOutput
        {
        	noperspective float2 UV : TEXCOORD0;
        	float4 Position : SV_POSITION;
        };

        struct WriteToSliceGeometryOutput
        {
            ScreenVertexOutput Vertex;
            uint LayerIndex : SV_RenderTargetArrayIndex;
        }
    "
    )
)