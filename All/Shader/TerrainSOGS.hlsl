struct VertexOut {
	float H : HEIGHT;
	uint Id : ID;
};

[maxvertexcount(1)]
void main(
	point VertexOut input[1],
	inout PointStream< VertexOut > output
)
{
	VertexOut element;
	element.H = input[0].H;
	element.Id = input[0].Id;

	output.Append(element);
}