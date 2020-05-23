(effect
    (refer Common.lsl)
    (shader
    "
        void MainPS(WriteToSliceGeometryOutput Input, out float4 OutColor : SV_Target0)
        {
        	OutColor = 1;
        }
    "
    )
)