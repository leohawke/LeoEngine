
using System.Runtime.InteropServices;

namespace leo.Platform.Render.Vertex
{
    public enum Usage
    {
        Position,//位置

        Normal,//法线
        Tangent,
        Binoraml,

        Diffuse,//顶点色
        Specular,//顶点高光
        BlendWeight,
        BlendIndex,

        TextureCoord,
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Element
    {
        Usage Usage { get; set; }
        byte UsageIndex { get; set; }
        EFormat Format { get; set; }
    }

}
