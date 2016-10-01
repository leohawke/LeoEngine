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
				explicit Texture(EFormat format);

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
				bool ReadyHWResource() const;

				template<typename _type>
				static void DoHWCopyToTexture(_type& src, _type& target, ResourceStateTransition src_st = {}, ResourceStateTransition target_st = {});
				template<typename _type>
				static void DoHWCopyToSubTexture(_type& src, _type& target,
					uint32 dst_subres, uint16 dst_x_offset, uint16 dst_y_offset, uint16 dst_z_offset,
					uint32 src_subres, uint16 src_x_offset, uint16 src_y_offset, uint16 src_z_offset,
					uint16 width, uint16 height, uint16 depth,
					ResourceStateTransition src_st = {}, ResourceStateTransition target_st = {});

				void DoCreateHWResource(D3D12_RESOURCE_DIMENSION dim,
					uint16 width, uint16 height, uint16 depth, uint8 array_size,
					ElementInitData const * init_data);

				void DoMap(EFormat format,uint32 subres, TextureMapAccess tma,
					uint16 x_offset, uint16 y_offset, uint16 z_offset,
					/*uint16 width,*/ uint16 height, uint16 depth,
					void*& data, uint32& row_pitch, uint32& slice_pitch);
				void DoUnmap(uint32 subres);

				template<typename _type>
				ViewSimulation* const& Retrive(_type& desc, std::unordered_map<std::size_t, std::unique_ptr<ViewSimulation>>& maps);

				ViewSimulation* const & RetriveSRV(D3D12_SHADER_RESOURCE_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveUAV(D3D12_UNORDERED_ACCESS_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveRTV(D3D12_RENDER_TARGET_VIEW_DESC const & desc);
				ViewSimulation* const & RetriveDSV(D3D12_DEPTH_STENCIL_VIEW_DESC const & desc);
			protected:
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

				void Map(TextureMapAccess tma,
					void*& data,const Box1D&) override;
				void UnMap(const Sub1D&) override;

				void CopyToTexture(platform::Render::Texture1D& target) override;

				void CopyToSubTexture(platform::Render::Texture1D& target,
					const Box1D& dst,
					const Box1D& src) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;

				template<typename _type>
				static bool Equal(_type & lhs, _type & rhs) {
					return (lhs.GetWidth(0) == rhs.GetWidth(0)) &&
						(lhs.GetArraySize() == rhs.GetArraySize()) &&
						(lhs.GetNumMipMaps() == rhs.GetNumMipMaps()) &&
						(lhs.GetFormat() == rhs.GetFormat());
				}
			protected:
				void Resize(platform::Render::Texture1D& target, const Box1D& dst,
					const Box1D& src,
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

				void Map(TextureMapAccess tma,
					void*& data, uint32& row_pitch,const Box2D&) override;

				void UnMap(const Sub1D&) override;

				void CopyToTexture(platform::Render::Texture2D& target) override;

				void CopyToSubTexture(platform::Render::Texture2D& target,
					const Box2D& dst,
					const Box2D& src) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;

				template<typename _type>
				static bool Equal(_type & lhs, _type & rhs) {
					return (lhs.GetHeight(0) == rhs.GetHeight(0)) &&
						Texture1D::Equal(lhs, rhs);
				}
			protected:
				void Resize(platform::Render::Texture2D& target, const Box2D& dst,
					const Box2D& src,
					bool linear) override;
			private:
				uint16 width;
				uint16 height;
			};

			class Texture3D :public platform::Render::Texture3D,public Texture {
			public:
				explicit Texture3D(uint16 width, uint16 height, uint16 depth, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;
				uint16 GetHeight(uint8 level) const override;
				uint16 GetDepth(uint8 level) const override;

				void Map(TextureMapAccess tma,
					void*& data, uint32& row_pitch, uint32& slice_pitch,const Box3D&) override;

				void UnMap(const Sub1D&) override;

				void CopyToTexture(platform::Render::Texture3D& target) override;

				void CopyToSubTexture(platform::Render::Texture3D& target,
					const Box3D& dst,
					const Box3D& src) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;
				ViewSimulation* RetriveUnorderedAccessView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) override;



				bool Equal(Texture3D& lhs, Texture3D & rhs) {
					return (lhs.GetDepth(0) == rhs.GetDepth(0)) &&
						Texture2D::Equal(lhs, rhs);
				}
			protected:
				void Resize(platform::Render::Texture3D& target, const Box3D& dst,
					const Box3D& src,
					bool linear) override;
			private:
				uint16 width;
				uint16 height;
				uint16 depth;
			};

			class TextureCube :public platform::Render::TextureCube,public Texture {
			public:
				explicit TextureCube(uint8 size,uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

				void BuildMipSubLevels() override;

				void HWResourceCreate(ElementInitData const * init_data) override;
				void HWResourceDelete() override;
				bool HWResourceReady() const override;

				uint16 GetWidth(uint8 level) const override;
				uint16 GetHeight(uint8 level) const override;


				void Map(TextureMapAccess tma,
					void*& data, uint32& row_pitch,const BoxCube&) override;

				void UnMap(const Sub1D&,TextureCubeFaces face) override;

				void CopyToTexture(platform::Render::TextureCube& target) override;

				void CopyToSubTexture(platform::Render::TextureCube& target,
					const BoxCube& dst,
					const BoxCube& src) override;

				ViewSimulation* RetriveShaderResourceView(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) override;

				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, uint8 level) override;
				ViewSimulation* RetriveUnorderedAccessView(uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 first_array_index, uint8 array_size,uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 first_array_index, uint8 array_size,uint8 level) override;

				ViewSimulation* RetriveRenderTargetView(uint8 array_index, TextureCubeFaces face, uint8 level) override;
				ViewSimulation* RetriveDepthStencilView(uint8 array_index, TextureCubeFaces face, uint8 level) override;
			protected:
				void Resize(platform::Render::TextureCube& target, const BoxCube&,
					const BoxCube&,
					bool linear) override;
			private:
				uint16 size;
			};

		}
	}
}

#endif