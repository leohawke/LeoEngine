struct GSOutput{
	float3 PosW : POSITION;
};


[maxvertexcount(3)]
void main(
	triangle float3 input[3] : POSITION,
	inout TriangleStream< GSOutput > output
)
{
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.PosW = input[i];
		output.Append(element);
	}
}