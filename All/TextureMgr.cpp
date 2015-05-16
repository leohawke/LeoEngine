#include "TextureMgr.h"
#include "Core\COM.hpp"
#include "DirectXTex.h"
#include "file.hpp"
#include "exception.hpp"

namespace leo
{
	std::unordered_map<std::size_t, ID3D11ShaderResourceView*> TextureMgr::mShaderResourceViews{};
	std::unordered_map<std::size_t, ID3D11Resource*> TextureMgr::mTextures{};
	wchar_t TextureMgr::buffer[buffer_size] {};


	TextureMgr::Delegate::~Delegate()
	{
		for (auto &srv : mShaderResourceViews)
		{
			leo::win::ReleaseCOM(srv.second);
		}
		for (auto &tex : mTextures)
		{
			leo::win::ReleaseCOM(tex.second);
		}
	}

	ID3D11ShaderResourceView* TextureMgr::LoadTextureSRV(const wchar_t* filename)
	{
		ID3D11ShaderResourceView* srv = 0;

		auto sid = hash(filename);

		static DirectX::TexMetadata info;
		static DirectX::ScratchImage image;

		// Does it already exist?
		if (mShaderResourceViews.find(sid) != mShaderResourceViews.end())
		{
			srv = mShaderResourceViews[sid];
		}
		else
		{
			switch (leo::win::file::GetFileExt(filename))
			{
			case win::file::FILE_TYPE::DDS:
				dxcall(DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &info, image));
				break;
			case win::file::FILE_TYPE::TGA:
				dxcall(DirectX::LoadFromTGAFile(filename, &info, image));
				break;
			case win::file::FILE_TYPE::OTHER_TEX_BEGIN:
			case win::file::FILE_TYPE::OTHER_TEX_END:
				dxcall(DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, &info, image));
				break;
			default:
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "不支持的数据格式");
				break;
			}
			dxcall(DirectX::CreateShaderResourceView(global::globalD3DDevice, image.GetImages(),
				image.GetImageCount(), info, &srv));
			leo::dx::DebugCOM(srv, filename);
			mShaderResourceViews[sid] = srv;
		}
		return srv;
	}

	ID3D11Resource* TextureMgr::LoadTexture(const wchar_t* filename, D3D11_USAGE usage, UINT bindFlags, UINT cpuAccessFlags, UINT miscFlags)
	{
		auto sid = hash(filename);
		if (mTextures.find(sid) != mTextures.end())
			return mTextures[sid];

		static DirectX::TexMetadata info;
		static DirectX::ScratchImage image;

		switch (leo::win::file::GetFileExt(filename))
		{
		case win::file::FILE_TYPE::DDS:
			dxcall(DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &info, image));
			break;
		case win::file::FILE_TYPE::TGA:
			dxcall(DirectX::LoadFromTGAFile(filename, &info, image));
			break;
		case win::file::FILE_TYPE::OTHER_TEX_BEGIN:
		case win::file::FILE_TYPE::OTHER_TEX_END:
			dxcall(DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, &info, image));
			break;
		default:
			Raise_Error_Exception(ERROR_INVALID_PARAMETER, "不支持的数据格式");
			break;
		}

		ID3D11Resource* tex = nullptr;

		CreateTextureEx(global::globalD3DDevice, image.GetImages(), image.GetImageCount(), info, usage, bindFlags, cpuAccessFlags, miscFlags, false, &tex);

		mTextures[sid] = tex;

		return tex;
	}
}