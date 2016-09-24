/*! \file Engine\Render\D3D12\Texture.h
\ingroup Engine
\brief 贴图接口相关封装。
*/

#ifndef LE_RENDER_D3D12_Texture_h
#define LE_RENDER_D3D12_Texture_h 1


#include "../ITexture.hpp"
#include "d3d12_dxgi.h"

#include <unordered_map>

namespace platform_ex {
	namespace Windows {

		namespace D3D12 {
			using platform::Render::TextureCubeFaces;
			using platform::Render::ElementInitData;
			using platform::Render::TextureMapAccess;
			using namespace platform::Render::IFormat;

			class ViewSimulation;

			class Texture
			{
			public:
				ID3D12Resource* Resource() const {
					return texture.Get();
				}

				DXGI_FORMAT GetDXGIFormat() const {
					return dxgi_format;
				}

				virtual ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) = 0;

				virtual ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) = 0;
				virtual ViewSimulation* RetriveRenderTargetView(uint8 first_array_index, uint8 num_items, uint8 level) = 0;
				virtual ViewSimulation* RetriveDepthStencilView(uint8 first_array_index, uint8 num_items, uint8 level) = 0;

				virtual ViewSimulation* RetriveUnorderedAccessView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) = 0;
				virtual ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) = 0;
				virtual ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) = 0;

				virtual ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level) = 0;

				virtual ViewSimulation* RetriveRenderTargetView(uint8 array_index, TextureCubeFaces face, uint8 level) = 0;
				virtual ViewSimulation* RetriveDepthStencilView(uint8 array_index, TextureCubeFaces face, uint8 level) = 0;

			protected:
				void DeleteHWResource();
				bool HWResourceReady();

				template<typename _type>
				static void DoHWCopyToTexture(_type& src, _type& target, RESOURCE_STATE_TRANSITION src_st = {}, RESOURCE_STATE_TRANSITION target_st = {});
				template<typename _type>
				static void DoHWCopyToSubTexture(_type& src, _type& target,
					uint32 dst_subres, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset,
					uint32 src_subres, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset,
					uint16 width, uint16 height, uint16 depth,
					RESOURCE_STATE_TRANSITION src_st = {}, RESOURCE_STATE_TRANSITION target_st = {});

				void DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim,
					uint16 width, uint16 height, uint16 depth, uint8 array_size,
					ElementInitData const * init_data);

				void DoMap(uint32 subres, TextureMapAccess tma,
					uint16 x_offset, uint16 y_offset, uint16 z_offset,
					uint16 width, uint16 height, uint16 depth,
					void*& data, uint32& row_pitch, uint32& slice_pitch);
				void DoUnmap(uint32 subres);

				ViewSimulation* const & RetriveSRV(D3D12_SHADER_RESOURCE_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveRTV(D3D12_RENDER_TARGET_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveDSV(D3D12_DEPTH_STENCIL_VIEW_DESC const & desc);
			private:
				DXGI_FORMAT dxgi_format;

				COMPtr<ID3D12Resource> texture;
				COMPtr<ID3D12Resource> texture_upload_heaps;
				COMPtr<ID3D12Resource> texture_readback_heaps;

				platform::Render::TextureMapAccess last_tma;

				std::unordered_map<std::size_t, std::unique_ptr<ViewSimulation>>  srv_maps;
				std::unordered_map<std::size_t, std::unique_ptr<ViewSimulation>> uav_maps;
				std::unordered_map<std::size_t, std::unique_ptr<ViewSimulation>> rtv_maps;
				std::unordered_map<std::size_t, std::unique_ptr<ViewSimulation>> dsv_maps;
			};

			class Texture1D :public platform::Render::Texture1D, public Texture {
			public:
				explicit Texture1D(uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;

				void Map(uint8 array_index, uint8 level, TextureMapAccess tma,
					uint16 x_offset, uint16 width,
					void*& data) override;
				void UnMap(uint8 array_index, uint8 level) override;

				void CopyToTexture(platform::Render::Texture1D& target) override;

				void CopyToSubTexture(platform::Render::Texture1D& target,
					uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_width,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_width) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
			protected:
				void Resize(platform::Render::Texture1D& target, uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_width,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_width,
					bool linear) override;
			private:
				uint16 width;
			};

			class Texture2D :public platform::Render::Texture2D,public Texture {
			public:
				explicit Texture2D(uint16 height, uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;
				uint16 GetHeight(uint8 level) const override;

				void Map(uint8 array_index, uint8 level, TextureMapAccess tma,
					uint16 x_offset, uint16 y_offset, uint16 width, uint16 height,
					void*& data, uint32& row_pitch) override;

				void UnMap(uint8 array_index, uint8 level) override;

				void CopyToTexture(platform::Render::Texture2D& target) override;

				void CopyToSubTexture(platform::Render::Texture2D& target,
					uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
			protected:
				void Resize(platform::Render::Texture2D& target, uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height,
					bool linear) override;
			private:
				uint16 width;
				uint16 height;
			};

			class Texture3D :public platform::Render::Texture3D,public Texture {
			public:
				explicit Texture3D(uint16 height, uint16 width, uint16 depth, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;
				uint16 GetHeight(uint8 level) const override;
				uint16 GetDepth(uint8 level) const override;

				void Map(uint8 array_index, uint8 level, TextureMapAccess tma,
					uint16 x_offset, uint16 y_offset, uint16 z_offset,
					uint16 width, uint16 height, uint16 depth,
					void*& data, uint32& row_pitch, uint32& slice_pitch) override;

				void UnMap(uint8 array_index, uint8 level) override;

				void CopyToTexture(platform::Render::Texture3D& target) override;

				void CopyToSubTexture(platform::Render::Texture3D& target,
					uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset, uint16 dst_width, uint16 dst_height, uint16 dst_depth,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset, uint16 src_width, uint16 src_height, uint16 src_depth) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;
				ViewSimulation* RetriveUnorderedAccessView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
			protected:
				void Resize(platform::Render::Texture3D& target, uint8 dst_array_index, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset, uint16 dst_width, uint16 dst_height, uint16 dst_depth,
					uint8 src_array_index, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset, uint16 src_width, uint16 src_height, uint16 src_depth,
					bool linear) override;
			private:
				uint16 width;
				uint16 height;
				uint16 depth;
			};

			class TextureCube :public platform::Render::TextureCube,public Texture {
			public:
				explicit TextureCube(uint8 size, uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;
				uint16 GetHeight(uint8 level) const override;


				void Map(uint8 array_index, TextureCubeFaces face, uint8 level, TextureMapAccess tma,
					uint16 x_offset, uint16 y_offset, uint16 width, uint16 height,
					void*& data, uint32& row_pitch) override;

				void UnMap(uint8 array_index, TextureCubeFaces face, uint8 level) override;

				void CopyToTexture(platform::Render::TextureCube& target) override;

				void CopyToSubTexture(platform::Render::TextureCube& target,
					uint8 dst_array_index, TextureCubeFaces dst_face, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height,
					uint8 src_array_index, TextureCubeFaces src_face, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;
				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, TextureCubeFaces face, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, TextureCubeFaces face, uint8 level) override;
			protected:
				void Resize(platform::Render::TextureCube& target, uint8 dst_array_index, CubeFaces dst_face, uint8 dst_level, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_width, uint16 dst_height,
					uint8 src_array_index, CubeFaces src_face, uint8 src_level, uint16 src_x_offset, uint16 src_y_offset, uint16 src_width, uint16 src_height,
					bool linear) override;
			private:
				uint16 size;
			};

		}
	}
}

#endif