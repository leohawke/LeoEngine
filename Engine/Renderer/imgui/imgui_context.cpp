#include "imgui_context.h"
#include "../../Render/ICommandList.h"
#include "../../Render/PipelineStateUtility.h"
#include "../../Render/BuiltInShader.h"
#include <LBase/lmathtype.hpp>
#include <Engine/Render/ShaderParamterTraits.hpp>
#include <Engine/Render/ShaderParameterStruct.h>

using namespace platform;
using namespace platform::imgui;
using namespace platform::Render;

Texture2D* GFontTexture = nullptr;
TextureSampleDesc GFontSampler {};

GraphicsBuffer* GIndexBuffer = nullptr;
GraphicsBuffer* GVertexBuffer = nullptr;
static int GVertexBufferSize = 5000;
static int GIndexBufferSize = 10000;

void CreateDeviceObjects(Render::Context& context);

bool platform::imgui::Context_Init(Render::Context& context)
{
    // Setup back-end capabilities flags
    ImGuiIO& io = ::ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_leoengine";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    return true;
}

void platform::imgui::Context_Shutdown()
{
    delete GFontTexture;
    delete GIndexBuffer;
    delete GVertexBuffer;
}

void platform::imgui::Context_NewFrame()
{
    if (!GFontTexture)
        CreateDeviceObjects(Context::Instance());
}

void FillGraphicsPipelineState(GraphicsPipelineStateInitializer& GraphicsPSOInit);

class imguiVS :public BuiltInShader
{
public:
    BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
        SHADER_PARAMETER(leo::math::float4x4, ProjectionMatrix)
        END_SHADER_PARAMETER_STRUCT();

    EXPORTED_BUILTIN_SHADER(imguiVS);
};

class imguiPS :public BuiltInShader
{
public:
    EXPORTED_BUILTIN_SHADER(imguiPS);
};

void platform::imgui::Context_RenderDrawData(ImDrawData* draw_data)
{
    auto& CmdList = Render::GetCommandList();

    //store  viewport& scissor

     // Create and grow vertex/index buffers if needed
    if (!GVertexBuffer || GVertexBufferSize < draw_data->TotalVtxCount)
    {
        if (GVertexBuffer)
            delete GVertexBuffer;

        GVertexBufferSize = draw_data->TotalVtxCount + 5000;

        GVertexBuffer = Context::Instance().GetDevice().CreateVertexBuffer(Buffer::Usage::Dynamic,
            Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_CPUWrite,
            GVertexBufferSize * sizeof(ImDrawVert),
            Render::EF_Unknown);
    }

    if (!GIndexBuffer || GIndexBufferSize < draw_data->TotalIdxCount)
    {
        if (GIndexBuffer)
            delete GIndexBuffer;

        GIndexBufferSize = draw_data->TotalIdxCount + 5000;

        GIndexBuffer = Context::Instance().GetDevice().CreateIndexBuffer(Buffer::Usage::Dynamic,
            Render::EAccessHint::EA_GPURead | Render::EAccessHint::EA_CPUWrite,
            GIndexBufferSize * sizeof(ImDrawIdx),
            Render::EF_R16UI);
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    {
        Buffer::Mapper vtx_resource(*GVertexBuffer, Buffer::Write_Only);
        Buffer::Mapper idx_resource(*GIndexBuffer, Buffer::Write_Only);
        ImDrawVert* vtx_dst = vtx_resource.Pointer<ImDrawVert>();
        ImDrawIdx* idx_dst = idx_resource.Pointer<ImDrawIdx>();
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
    }

    // Setup pipelinestate
    Render::GraphicsPipelineStateInitializer GraphicsPSOInit {};
    CmdList.FillRenderTargetsInfo(GraphicsPSOInit);

    FillGraphicsPipelineState(GraphicsPSOInit);

    auto SetupRenderState = [&](ImDrawData* draw_data)
    {
        Render::SetGraphicsPipelineState(CmdList, GraphicsPSOInit);

        // Setup viewport
        CmdList.SetViewport(0, 0, 0,static_cast<uint32>(draw_data->DisplaySize.x), static_cast<uint32>(draw_data->DisplaySize.y), 1);
        {
            auto VertexShader = Render::GetBuiltInShaderMap()->GetShader<imguiVS>();

            // Setup orthographic projection matrix into our constant buffer
            // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
            imguiVS::Parameters vsParameters;

            float L = draw_data->DisplayPos.x;
            float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
            float T = draw_data->DisplayPos.y;
            float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

            leo::math::float4x4 mvp =
            {
                 { 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
                { 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
                { 0.0f,         0.0f,           0.5f,       0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
            };
            vsParameters.ProjectionMatrix = leo::math::transpose(mvp);

            SetShaderParameters(CmdList, VertexShader, VertexShader->GetVertexShader(), vsParameters);
        }

        CmdList.SetVertexBuffer(0, GVertexBuffer);
        CmdList.SetShaderSampler(GraphicsPSOInit.ShaderPass.PixelShader, 0, GFontSampler);
    };

    SetupRenderState(draw_data);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(draw_data);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Apply scissor/clipping rectangle
                CmdList.SetScissorRect(true, (uint32)(pcmd->ClipRect.x - clip_off.x), (uint32)(pcmd->ClipRect.y - clip_off.y), (uint32)(pcmd->ClipRect.z - clip_off.x), (uint32)(pcmd->ClipRect.w - clip_off.y));

                // Bind texture, Draw
                Texture* texture = (Texture*)pcmd->TextureId;
                CmdList.SetShaderTexture(GraphicsPSOInit.ShaderPass.PixelShader,0, texture);
                CmdList.DrawIndexedPrimitive(GIndexBuffer,
                    pcmd->VtxOffset + global_vtx_offset, 
                    0, //FirstInstance
                    cmd_list->VtxBuffer.Size,
                    pcmd->IdxOffset + global_idx_offset,
                    pcmd->ElemCount/3,
                    1);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }

    //Restore viewport& scissor
}



void FillGraphicsPipelineState(GraphicsPipelineStateInitializer& GraphicsPSOInit)
{
    {
        platform::Render::BlendDesc BlendState {};
        BlendState.blend_factor = LinearColor(0, 0, 0, 0);
        BlendState.alpha_to_coverage_enable = false;
        BlendState.blend_enable[0] = true;
        BlendState.src_blend[0] = BlendFactor::Src_Alpha;
        BlendState.dst_blend[0] = BlendFactor::Inv_Src_Alpha;
        BlendState.blend_op[0] = BlendOp::Add;
        BlendState.src_blend_alpha[0] = BlendFactor::Inv_Src_Alpha;
        BlendState.dst_blend_alpha[0] = BlendFactor::Zero;
        BlendState.blend_op_alpha[0] = BlendOp::Add;
        BlendState.color_write_mask[0] = (leo::uint8)ColorMask::All;

        GraphicsPSOInit.BlendState = BlendState;
    }
    {
        auto& RasterizerState = GraphicsPSOInit.RasterizerState;
        RasterizerState.cull = CullMode::None;
        RasterizerState.scissor_enable = true;
        RasterizerState.multisample_enable = false;
    }
    {
        GraphicsPSOInit.DepthStencilState.depth_enable = false;
        GraphicsPSOInit.DepthStencilState.depth_func = CompareOp::Pass;
    }
    {
        auto& VertexDeclaration = GraphicsPSOInit.ShaderPass.VertexDeclaration;
        VertexElement pos = CtorVertexElement(0,loffsetof(ImDrawVert,pos),Vertex::Usage::Position,0,EF_GR32F,sizeof(ImDrawVert));
        VertexElement uv = CtorVertexElement(0,loffsetof(ImDrawVert,uv),Vertex::Usage::TextureCoord,0,EF_GR32F,sizeof(ImDrawVert));
        VertexElement color = CtorVertexElement( 0,loffsetof(ImDrawVert,col),Vertex::Usage::Diffuse,0,EF_ARGB8,sizeof(ImDrawVert));

        GraphicsPSOInit.ShaderPass.VertexDeclaration.emplace_back(pos);
        GraphicsPSOInit.ShaderPass.VertexDeclaration.emplace_back(uv);
        GraphicsPSOInit.ShaderPass.VertexDeclaration.emplace_back(color);

        GraphicsPSOInit.Primitive = PrimtivteType::TriangleList;
    }
    {
        auto VertexShader = Render::GetBuiltInShaderMap()->GetShader<imguiVS>();

        auto PixelShader = Render::GetBuiltInShaderMap()->GetShader<imguiPS>();

        GraphicsPSOInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();

        GraphicsPSOInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();
    }
}

void CreateFontsTexture(Render::Context& context)
{
    // Build texture atlas
    ImGuiIO& io = ::ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    // Upload texture to graphics system
    {
        ElementInitData subResource;
        subResource.data = pixels;
        subResource.row_pitch = width * 4;
        subResource.slice_pitch = 0;
        GFontTexture = context.GetDevice().CreateTexture(
            static_cast<uint16>(width),
            static_cast<uint16>(height),
            1,1,
            EF_ARGB8,
            (uint32)Render::EAccessHint::EA_GPURead,
            SampleDesc(1,0),
            &subResource
        );
    }

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)GFontTexture;

    GFontSampler.address_mode_u = GFontSampler.address_mode_v = GFontSampler.address_mode_w = TexAddressingMode::Wrap;
    GFontSampler.filtering = TexFilterOp::Min_Mag_Mip_Linear;
    GFontSampler.min_lod = 0;
    GFontSampler.max_lod = 0;
    GFontSampler.cmp_func = CompareOp::Pass;
}

IMPLEMENT_BUILTIN_SHADER(imguiVS, "imgui/imgui.lsl", "MainVS", platform::Render::VertexShader);

IMPLEMENT_BUILTIN_SHADER(imguiPS, "imgui/imgui.lsl", "MainPS", platform::Render::PixelShader);

void CreateDeviceObjects(Render::Context& context)
{
    auto VertexShader = Render::GetBuiltInShaderMap()->GetShader<imguiVS>();
    lconstraint(VertexShader);
    auto PixelShader = Render::GetBuiltInShaderMap()->GetShader<imguiPS>();
    lconstraint(PixelShader);

    CreateFontsTexture(context);
}
