(effect
    (refer Common.lsl)
    (int MinZ)
    (float4 UVScaleBias)
    (shader
    "
        struct WriteToSliceVertexOutput
        {
            ScreenVertexOutput Vertex;
            uint LayerIndex : SV_RenderTargetArrayIndex;
        }

        /** Vertex shader that writes to a range of slices of a volume texture. */
        void WriteToSliceMainVS(
        	float2 InPosition : POSITION,
        	float2 InUV       : TEXCOORD0,
        	uint LayerIndex : SV_InstanceID,
        	out WriteToSliceVertexOutput Output
        	)
        {
        	Output.Vertex.Position = float4( InPosition, 0, 1 );
        	// Remap UVs based on the subregion of the volume texture being rendered to
            Output.Vertex.UV = InUV * UVScaleBias.xy + UVScaleBias.zw;
        	Output.LayerIndex = LayerIndex + MinZ;
        }

        /** Geometry shader that writes to a range of slices of a volume texture. */
        [maxvertexcount(3)]
        void WriteToSliceMainGS(triangle WriteToSliceVertexOutput Input[3], inout TriangleStream<WriteToSliceGeometryOutput> OutStream)
        {
        	WriteToSliceGeometryOutput Vertex0;
        	Vertex0.Vertex = Input[0].Vertex;
        	Vertex0.LayerIndex = Input[0].LayerIndex;
        
        	WriteToSliceGeometryOutput Vertex1;
        	Vertex1.Vertex = Input[1].Vertex;
        	Vertex1.LayerIndex = Input[1].LayerIndex;
        
        	WriteToSliceGeometryOutput Vertex2;
        	Vertex2.Vertex = Input[2].Vertex;
        	Vertex2.LayerIndex = Input[2].LayerIndex;
        
        	OutStream.Append(Vertex0);
        	OutStream.Append(Vertex1);
        	OutStream.Append(Vertex2);
        }
    "
    )
)