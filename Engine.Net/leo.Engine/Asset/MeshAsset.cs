
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using leo.Platform.Render;

namespace leo.Asset
{
    public class MeshAsset
    {
        public struct SubMeshDescrption {
            byte MaterialIndex { get; set; }
            [StructLayout(LayoutKind.Sequential)]
            public struct LodDescription
            {
                uint VertexNum { get; set; }
                uint VertexBase { get; set; }
                uint IndexNum { get; set; }
                uint IndexBase { get; set; }
            }

            public extern ReadOnlySpan<LodDescription> LodsDescription {
                [MethodImpl(MethodImplOptions.Unmanaged)]
                get;
            }
        }

        [MethodImpl(MethodImplOptions.Unmanaged)]
        public extern MeshAsset();

        public extern ReadOnlySpan<Platform.Render.Vertex.Element> VertexElements
        {
            [MethodImpl(MethodImplOptions.Unmanaged)]
            get;
        }

        public extern EFormat IndexFormat
        {
            [MethodImpl(MethodImplOptions.Unmanaged)]
            get;
        }

        public extern ReadOnlySpan<SubMeshDescrption> SubMeshs
        {
            [MethodImpl(MethodImplOptions.Unmanaged)]
            get;
        }


        [MethodImpl(MethodImplOptions.Unmanaged)]
        public extern void AddVertexStream(Platform.Render.Vertex.Element element, Span<Byte> stream);

        [MethodImpl(MethodImplOptions.Unmanaged)]
        public extern void SetIndexStream(EFormat format, Span<Byte> stream);

        [MethodImpl(MethodImplOptions.Unmanaged)]
        public extern void AddSubMeshDescrption(SubMeshDescrption desc);
    }
}
