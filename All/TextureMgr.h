#pragma once
#include "Mgr.hpp"


namespace leo
{
	class TextureMgr
	{
	private:
		class Delegate : public Singleton<Delegate>
		{
		public:
			~Delegate();
		public:
			static const std::unique_ptr<Delegate>& GetInstance()
			{
				static auto mInstance = unique_raw(new Delegate());
				return mInstance;
			}
		};
	public:
		TextureMgr()
		{
			Delegate::GetInstance();
		}
		//mark: support any picture format and any dimension
		ID3D11ShaderResourceView* LoadTextureSRV(const wchar_t* filename);
		ID3D11ShaderResourceView* LoadTextureSRV(const std::wstring& filename)
		{
			return LoadTextureSRV(filename.c_str());
		}

		template<std::size_t size>
		//mark: support any picture format and must be 2 dimension(but all picture must have same width and height)
		ID3D11ShaderResourceView* LoadTexture2DArraySRV(const std::array<const wchar_t*, size>& filenames)
		{
			auto sid = CatAndHash(filenames);

			if (mShaderResourceViews.find(sid) != mShaderResourceViews.end())
				return mShaderResourceViews[sid];

			std::array<ID3D11Resource*, size> srcRess;

#if defined(DEBUG) || defined(_DEBUG)
			D3D11_RESOURCE_DIMENSION resource_dimension;
#endif

			for (std::size_t i = 0; i != size; ++i)
			{
				srcRess[i] = LoadTexture(filenames[i], D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ);
#if defined(DEBUG) || defined(_DEBUG)
				srcRess[i]->GetType(&resource_dimension);
				assert(resource_dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D);
#endif
			}

			//Test
			//EndTest


			//
			// Create the texture array.  Each element in the texture 
			// array has the same format/dimensions.
			//
			ID3D11Texture2D* tex = nullptr;
			dxcall(srcRess[0]->QueryInterface(IID_ID3D11Texture2D, (void**)&tex));
			D3D11_TEXTURE2D_DESC texElementDesc{};
			tex->GetDesc(&texElementDesc);
			win::ReleaseCOM(tex);


			D3D11_TEXTURE2D_DESC texArrayDesc;
			texArrayDesc.Width = texElementDesc.Width;
			texArrayDesc.Height = texElementDesc.Height;
			texArrayDesc.MipLevels = texElementDesc.MipLevels;
			texArrayDesc.ArraySize = size;
			texArrayDesc.Format = texElementDesc.Format;
			texArrayDesc.SampleDesc.Count = 1;
			texArrayDesc.SampleDesc.Quality = 0;
			texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
			texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texArrayDesc.CPUAccessFlags = 0;
			texArrayDesc.MiscFlags = 0;

			ID3D11Texture2D * texArray = 0;
			dxcall(global::globalD3DDevice->CreateTexture2D(&texArrayDesc, 0, &texArray));

			//
			// Copy Individual texture elements into texture array.
			//
			for (UINT texElement = 0; texElement != size; ++texElement)
				for (UINT mipLevel = 0; mipLevel != texElementDesc.MipLevels; ++mipLevel)
				{
				D3D11_MAPPED_SUBRESOURCE mappedTex2D;
				dxcall(global::globalD3DContext->Map(srcRess[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

				global::globalD3DContext->UpdateSubresource(texArray,
					D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
					0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

				global::globalD3DContext->Unmap(srcRess[texElement], mipLevel);
				}

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = texArrayDesc.Format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.ArraySize = size;

			ID3D11ShaderResourceView* texArraySRV = nullptr;
			dxcall(global::globalD3DDevice->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

			leo::win::ReleaseCOM(texArray);

			mShaderResourceViews[sid] = texArraySRV;
			return texArraySRV;
		}
		template<std::size_t size>
		//mark: support any picture format and must be 2 dimension
		ID3D11ShaderResourceView* LoadTexture2DArraySRV(const std::array<std::reference_wrapper<std::wstring>, size>& filenames)
		{
			std::array<const wchar_t*, size> t_filenames;
			for (std::size_t i = 0; i != size; ++i)
			{
				t_filenames[i] = filenames[i].c_str();
			}
			LoadTexture2DArraySRV(t_filenames);
		}

		ID3D11Resource* LoadTexture(const wchar_t* filename, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE, UINT cpuAccessFlags = 0, UINT miscFlags = 0);
		ID3D11Resource* LoadTexture(const std::wstring& filename, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE, UINT cpuAccessFlags = 0, UINT miscFlags = 0)
		{
			return LoadTexture(filename.c_str(), usage, bindFlags, cpuAccessFlags, miscFlags);
		}

		void UnLoadTexture(const wchar_t* filename)
		{
			auto sid = hash(filename);
			auto it = mTextures.find(sid);
#if defined (DEBUG) || defined(_DEBUG)
			if (it == mTextures.end())
			{
				DebugPrintf(L"错误(E_UNLOAD): 该文件未被载入为纹理,文件名: %s\n", filename);
				throw std::runtime_error("错误(E_UNLOAD): 该文件未被载入为纹理");
			}
#endif
			win::ReleaseCOM(it->second);
			mTextures.erase(it);
		}
		void UnLoadTexture(const std::wstring& filename)
		{
			UnLoadTexture(filename.c_str());
		}

		void UnLoadSRV(const wchar_t* filename)
		{
			auto sid = hash(filename);
			auto it = mShaderResourceViews.find(sid);
#if defined (DEBUG) || defined(_DEBUG)
			if (it == mShaderResourceViews.end())
			{
				DebugPrintf(L"错误(E_UNLOAD): 该文件未被载入为资源,文件名: %s\n", filename);
				throw std::runtime_error("错误(E_UNLOAD): 该文件未被载入为资源");
			}
#endif
			win::ReleaseCOM(it->second);
			mShaderResourceViews.erase(it);
		}
		void UnLoadSRV(const std::wstring& filename)
		{
			UnLoadSRV(filename.c_str());
		}

		template<std::size_t size>
		void UnLoadTexture2DArraySRV(const std::array<const wchar_t*, size>& filenames)
		{
			for (auto s : filenames)
				UnLoadTexture(s);

			auto sid = CatAndHash(filenames);

			auto it = mShaderResourceViews.find(sid);
#if defined (DEBUG) || defined(_DEBUG)
			if (it == mShaderResourceViews.end())
			{
				DebugPrintf(L"错误(E_UNLOAD): 该文件组未被载入为资源,拼接文件名: %s\n", buffer);
				throw std::runtime_error("错误(E_UNLOAD): 该文件组未被载入为资源");
			}
#endif
			win::ReleaseCOM(it->second);
			mShaderResourceViews.erase(it);
		}
		template<std::size_t size>
		void UnLoadTexture2DArraySRV(const std::array<std::reference_wrapper<std::wstring>, size>& filenames)
		{
			for (auto s : filenames)
				UnLoadTexture(s);

			auto sid = CatAndHash(filenames);

			auto it = mShaderResourceViews.find(sid);
#if defined (DEBUG) || defined(_DEBUG)
			if (it == mShaderResourceViews.end())
			{
				DebugPrintf(L"错误(E_UNLOAD): 该文件组未被载入为资源,拼接文件名: %s\n", buffer);
				throw std::runtime_error("错误(E_UNLOAD): 该文件组未被载入为资源");
			}
#endif
			win::ReleaseCOM(it->second);
			mShaderResourceViews.erase(it);
		}
	private:
		const static std::size_t buffer_size = 4096 / sizeof(wchar_t);
		static wchar_t buffer[buffer_size];

		template<std::size_t size>
		std::size_t CatAndHash(const std::array<const wchar_t*, size>& filenames)
		{
			std::size_t buffer_pos = 0;
			std::size_t string_len = 0;
			for (auto s : filenames)
			{
				string_len = wcslen(s);
				if (buffer_pos + string_len > buffer_size)
				{
					buffer_size - buffer_pos > 0 ? std::memcpy(&buffer[buffer_pos], s, buffer_size - buffer_pos) : 0;
					buffer_pos = 4097;
					break;
				}
				std::memcpy(&buffer[buffer_pos], s, string_len);
				buffer_pos += string_len;
			}
			buffer[buffer_pos] = wchar_t();
			auto sid = hash(buffer);
			return sid;
		}

		template<std::size_t size>
		std::size_t CatAndHash(const std::array<std::reference_wrapper<std::wstring>, size>& filenames)
		{
			std::size_t buffer_pos = 0;
			std::size_t string_len = 0;
			for (auto s : filenames)
			{
				string_len = s.size();
				if (buffer_pos + string_len > buffer_size)
				{
					buffer_size - buffer_pos > 0 ? memcpy(&buffer[buffer_pos], s.c_str(), buffer_size - buffer_pos) : 0;
					buffer_pos = 4097;
					break;
				}
				memcpy(&buffer[buffer_pos], s.c_str(), string_len);
				buffer_pos += string_len;
			}
			buffer[buffer_pos] = wchar_t();
			auto sid = hash(buffer);
			return sid;
		}
	private:
		static std::unordered_map<std::size_t, ID3D11ShaderResourceView*> mShaderResourceViews;
		static std::unordered_map < std::size_t, ID3D11Resource* >
			mTextures;
	};
}