/*! \file Engine\Render\D3D12\Texture.h
\ingroup Engine
\brief 贴图接口相关封装。
*/

#ifndef LE_RENDER_D3D12_Texture_h
#define LE_RENDER_D3D12_Texture_h 1


#include "../ITexture.hpp"
#include "../RenderPassInfo.h"
#include "ResourceHolder.h"
#include <unordered_map>

namespace platform_ex::Windows::D3D12 {
	using namespace leo::inttype;

	using platform::Render::TextureCubeFaces;
	using platform::Render::ElementInitData;
	using platform::Render::TextureMapAccess;
	using namespace platform::Render::IFormat;

	class ViewSimulation;
	class ShaderResourceView;
	class RenderTargetView;
	class DepthStencilView;

	class Texture :public ResourceHolder
	{
	public:
		explicit Texture(EFormat format);

		DXGI_FORMAT GetDXGIFormat() const {
			return dxgi_format;
		}

		virtual ShaderResourceView* RetriveShaderResourceView() = 0;
	protected:
		Texture(const COMPtr<ID3D12Resource>& pResource);

		std::string HWDescription() const;

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
			ElementInitData const* init_data);

		void DoMap(EFormat format, uint32 subres, TextureMapAccess tma,
			uint16 x_offset, uint16 y_offset, uint16 z_offset,
			/*uint16 width,*/ uint16 height, uint16 depth,
			void*& data, uint32& row_pitch, uint32& slice_pitch);
		void DoUnmap(uint32 subres);

		void DoHWBuildMipSubLevels(uint8 array_size, uint8 mipmap_size, uint16 width, uint16 height = 0, uint8 facecount = 1);

	public:
		/**
		* Get the render target view for the specified mip and array slice.
		* An array slice of -1 is used to indicate that no array slice should be required.
		*/
		RenderTargetView* GetRenderTargetView(int32 MipIndex, int32 ArraySliceIndex) const
		{
			int32 ArrayIndex = MipIndex;

			if (bCreatedRTVsPerSlice)
			{
				lconstraint(ArraySliceIndex >= 0);
				ArrayIndex = MipIndex * RTVArraySize + ArraySliceIndex;
				lconstraint(ArrayIndex < RenderTargetViews.size());
			}
			else
			{
				// Catch attempts to use a specific slice without having created the texture to support it
				lconstraint(ArraySliceIndex == -1 || ArraySliceIndex == 0);
			}

			if (ArrayIndex < RenderTargetViews.size())
			{
				return RenderTargetViews[ArrayIndex].get();
			}
			return 0;
		}
		DepthStencilView* GetDepthStencilView(platform::Render::ExclusiveDepthStencil AccessType) const
		{
			return DepthStencilViews[AccessType.GetIndex()].get();
		}

		void SetNumRenderTargetViews(int32 InNumViews)
		{
			RenderTargetViews.resize(InNumViews);
		}
		void SetRenderTargetViewIndex(RenderTargetView* View, uint32 SubResourceIndex)
		{
			if (SubResourceIndex < (uint32)RenderTargetViews.size())
			{
				RenderTargetViews[SubResourceIndex].reset(View);
			}
			else
			{
				lconstraint(false);
			}
		}

		void SetDepthStencilView(DepthStencilView* View, uint32 SubResourceIndex)
		{
			if (SubResourceIndex < 4)
			{
				DepthStencilViews[SubResourceIndex].reset(View);
			}
			else
			{
				lconstraint(false);
			}
		}
	protected:
		DXGI_FORMAT dxgi_format;

		COMPtr<ID3D12Resource> texture_upload_heaps;
		COMPtr<ID3D12Resource> texture_readback_heaps;

		platform::Render::TextureMapAccess last_tma;

		std::unique_ptr<ShaderResourceView> default_srv;

		/** A render targetable view of the texture. */
		std::vector<std::shared_ptr<RenderTargetView>> RenderTargetViews;

		/** A depth-stencil targetable view of the texture. */
		std::shared_ptr<DepthStencilView> DepthStencilViews[4];

		bool bCreatedRTVsPerSlice = false;

		int32 RTVArraySize;
	};

	class Texture1D final :public Texture, public platform::Render::Texture1D {
	public:
		explicit Texture1D(uint16 width, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);
	protected:
		//\brief encode = UTF-8
		std::string Description() const override;
	public:
		void BuildMipSubLevels() override;

		void HWResourceCreate(ElementInitData const* init_data) override;
		void HWResourceDelete() override;
		bool HWResourceReady() const override;

		uint16 GetWidth(uint8 level) const override;

		void Map(TextureMapAccess tma,
			void*& data, const Box1D&) override;
		void UnMap(const Sub1D&) override;

		void CopyToTexture(platform::Render::Texture1D& target) override;

		void CopyToSubTexture(platform::Render::Texture1D& target,
			const Box1D& dst,
			const Box1D& src) override;

		template<typename _type>
		static bool Equal(_type& lhs, _type& rhs) {
			return (lhs.GetWidth(0) == rhs.GetWidth(0)) &&
				(lhs.GetArraySize() == rhs.GetArraySize()) &&
				(lhs.GetNumMipMaps() == rhs.GetNumMipMaps()) &&
				(lhs.GetFormat() == rhs.GetFormat());
		}

		ShaderResourceView* RetriveShaderResourceView() override;

		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;
		D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

	protected:
		void Resize(platform::Render::Texture1D& target, const Box1D& dst,
			const Box1D& src,
			bool linear) override;
	private:
		uint16 width;
	};

	class Texture2D final :public Texture, public platform::Render::Texture2D {
	public:
		explicit Texture2D(uint16 width, uint16 height, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);

		explicit Texture2D(const COMPtr<ID3D12Resource>& pResource);
	protected:
		//\brief encode = UTF-8
		std::string Description() const override;
	public:
		void BuildMipSubLevels() override;

		void HWResourceCreate(ElementInitData const* init_data) override;
		void HWResourceDelete() override;
		bool HWResourceReady() const override;

		uint16 GetWidth(uint8 level) const override;
		uint16 GetHeight(uint8 level) const override;

		void Map(TextureMapAccess tma,
			void*& data, uint32& row_pitch, const Box2D&) override;

		void UnMap(const Sub1D&) override;

		void CopyToTexture(platform::Render::Texture2D& target) override;

		void CopyToSubTexture(platform::Render::Texture2D& target,
			const Box2D& dst,
			const Box2D& src) override;

		template<typename _type>
		static bool Equal(_type& lhs, _type& rhs) {
			return (lhs.GetHeight(0) == rhs.GetHeight(0)) &&
				Texture1D::Equal(lhs, rhs);
		}

		ShaderResourceView* RetriveShaderResourceView() override;

		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;
	protected:
		void Resize(platform::Render::Texture2D& target, const Box2D& dst,
			const Box2D& src,
			bool linear) override;
	private:
		uint16 width;
		uint16 height;
	};

	class Texture3D final : public Texture, public platform::Render::Texture3D {
	public:
		explicit Texture3D(uint16 width, uint16 height, uint16 depth, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, uint32 NumSamples);
	protected:
		//\brief encode = UTF-8
		std::string  Description() const override;
	public:
		void BuildMipSubLevels() override;

		void HWResourceCreate(ElementInitData const* init_data) override;
		void HWResourceDelete() override;
		bool HWResourceReady() const override;

		uint16 GetWidth(uint8 level) const override;
		uint16 GetHeight(uint8 level) const override;
		uint16 GetDepth(uint8 level) const override;

		void Map(TextureMapAccess tma,
			void*& data, uint32& row_pitch, uint32& slice_pitch, const Box3D&) override;

		void UnMap(const Sub1D&) override;

		void CopyToTexture(platform::Render::Texture3D& target) override;

		void CopyToSubTexture(platform::Render::Texture3D& target,
			const Box3D& dst,
			const Box3D& src) override;

		bool Equal(Texture3D& lhs, Texture3D& rhs) {
			return (lhs.GetDepth(0) == rhs.GetDepth(0)) &&
				Texture2D::Equal(lhs, rhs);
		}

		ShaderResourceView* RetriveShaderResourceView() override;

		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint16 first_slice, uint16 num_slices, uint8 level) const;

		D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) const;

		D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level) const;
	protected:
		void Resize(platform::Render::Texture3D& target, const Box3D& dst,
			const Box3D& src,
			bool linear) override;
	private:
		uint16 width;
		uint16 height;
		uint16 depth;
	};

	class TextureCube final : public Texture, public platform::Render::TextureCube {
	public:
		explicit TextureCube(uint16 size, uint8 numMipMaps, uint8 array_size, EFormat format, uint32 access_hint, platform::Render::SampleDesc sample_info);
	protected:
		//\brief encode = UTF-8
		std::string Description() const override;
	public:
		void BuildMipSubLevels() override;

		void HWResourceCreate(ElementInitData const* init_data) override;
		void HWResourceDelete() override;
		bool HWResourceReady() const override;

		uint16 GetWidth(uint8 level) const override;
		uint16 GetHeight(uint8 level) const override;


		void Map(TextureMapAccess tma,
			void*& data, uint32& row_pitch, const BoxCube&) override;

		void UnMap(const Sub1D&, TextureCubeFaces face) override;

		void CopyToTexture(platform::Render::TextureCube& target) override;

		void CopyToSubTexture(platform::Render::TextureCube& target,
			const BoxCube& dst,
			const BoxCube& src) override;

		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level) const;

		D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(uint8 first_array_index, uint8 num_items, uint8 level) const;

		D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(uint8 array_index, TextureCubeFaces face, uint8 level) const;

		D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(uint8 array_index, TextureCubeFaces face, uint8 level) const;

		ShaderResourceView* RetriveShaderResourceView() override;

	protected:
		void Resize(platform::Render::TextureCube& target, const BoxCube&,
			const BoxCube&,
			bool linear) override;
	private:
		uint16 size;
	};

	D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 first_level, uint8 num_levels);

	D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level);
	D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level);
	D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, uint8 level);

	D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(const platform::Render::Texture& tex, uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level);
	D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(const platform::Render::Texture& tex, uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level);
	ViewSimulation* CreateDSVDesc(const platform::Render::Texture& tex, uint8 array_index, uint16 first_slice, uint16 num_slices, uint8 level);

	D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(const platform::Render::Texture& tex, uint8 first_array_index, uint8 num_items, TextureCubeFaces first_face, uint8 num_faces, uint8 level);

	D3D12_RENDER_TARGET_VIEW_DESC CreateRTVDesc(const platform::Render::Texture& tex, uint8 array_index, TextureCubeFaces face, uint8 level);
	D3D12_DEPTH_STENCIL_VIEW_DESC CreateDSVDesc(const platform::Render::Texture& tex, uint8 array_index, TextureCubeFaces face, uint8 level);

}

#endif