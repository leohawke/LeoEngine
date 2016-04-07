#include <platform.h>

#include <DXGIFormat.h>
#include <assert.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#pragma warning(disable : 4458)// warning C4458: declaration of param hides class member
#include <d2d1.h>
#include <wincodec.h>
#pragma warning(pop)

#include <memory>

#include "debugoutput.hpp"
#include "d3dx11.hpp"
#include "exception.hpp"

#if(_WIN32_WINNT >= 0x0602) && !defined(DXGI_12_FORMATS)
#define DXGI_12_FORMATS
#endif

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP) || (_WIN32_WINNT > _WIN32_WINNT_WIN8)

#include <wincodec.h>
#endif
#include <atlbase.h>
#include "dds.h"

#include "D3D11\D3D11RenderSystem.hpp"

#include "raii.hpp"

struct struct_wicguid2dxgiformat
{
	GUID wicguid;
	DXGI_FORMAT dxgiformat;
};

static struct_wicguid2dxgiformat formatsmap[] =
{
	{ GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT },

	{ GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT },
	{ GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM },

	{ GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM },
	{ GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

	{ GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM },
	{ GUID_WICPixelFormat32bppRGBE, DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

#ifdef DXGI_12_FORMATS

	{ GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM },
	{ GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM },

#endif // DXGI_12_FORMATS

	{ GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT },
	{ GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT },
	{ GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM },
	{ GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM },

	{ GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM },

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
	{ GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT },
#endif

};

struct struct_wicguidconvert
{
	GUID source;
	GUID target;
};

static struct_wicguidconvert guidsmap[] =
{
	// Note target GUID in this conversion table must be one of those directly supported formats (above).

	{ GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

	{ GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

	{ GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
	{ GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

	{ GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
	{ GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

#ifdef DXGI_12_FORMATS
	{ GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM
#else

	{ GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat16bppBGRA5551, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat16bppBGR565, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

#endif // DXGI_12_FORMATS

	{ GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

	{ GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

	{ GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

	{ GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
	{ GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

	{ GUID_WICPixelFormat96bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
	{ GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

	{ GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
	{ GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
	{ GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
#endif

	// We don't support n-channel formats
};

bool& IsWIC2() {
	static bool is_wic2 = false;
	return is_wic2;
}
static IWICImagingFactory* GetWICFactory()
{
	static IWICImagingFactory* s_Factory = nullptr;

	if (s_Factory)
		return s_Factory;

#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory2),
		(LPVOID*)&s_Factory
		);

	if (SUCCEEDED(hr))
	{
		// WIC2 is available on Windows 8 and Windows 7 SP1 with KB 2670838 installed
		IsWIC2() = true;
	}
	else
	{
		hr = CoCreateInstance(
			CLSID_WICImagingFactory1,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			(LPVOID*)&s_Factory
			);

		if (FAILED(hr))
		{
			s_Factory = nullptr;
			return nullptr;
		}
	}
#else
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		(LPVOID*)&s_Factory
		);

	if (FAILED(hr))
	{
		s_Factory = nullptr;
		return nullptr;
	}
#endif

	return s_Factory;

}

static DXGI_FORMAT WIC2DXGI(const GUID& guid)
{
	for (size_t i = 0; i < _countof(formatsmap); ++i)
	{
		if (memcmp(&formatsmap[i].wicguid, &guid, sizeof(GUID) == 0))
			return formatsmap[i].dxgiformat;
	}
	return DXGI_FORMAT_UNKNOWN;
}

static size_t WICBitsPerPixel(const GUID& targetGuid)
{
	IWICImagingFactory* pWIC = GetWICFactory();
	if (!pWIC)
		return 0;
	leo::dx::ScopedCOMObject<IWICComponentInfo> cinfo;
	if (FAILED(pWIC->CreateComponentInfo(targetGuid, &cinfo)))
		return 0;
	WICComponentType type;
	if (FAILED(cinfo->GetComponentType(&type)))
		return 0;
	if (type != WICPixelFormat)
		return 0;
	leo::dx::ScopedCOMObject<IWICPixelFormatInfo> finfo;
	if (FAILED(cinfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(&finfo))))
		return 0;
	UINT bpp;
	if (FAILED(finfo->GetBitsPerPixel(&bpp)))
		return 0;
	return bpp;
}

static HRESULT CreateTextureFromWIC(_In_ ID3D11Device* device,
	_In_opt_ ID3D11DeviceContext* deviceContext,
	_In_ IWICBitmapFrameDecode *frame,
	_Out_opt_ ID3D11Resource** texture,
	_Out_opt_ ID3D11ShaderResourceView** textureView,
	_In_ size_t maxsize)
{
	UINT width = 0, height = 0;
	HRESULT hr = frame->GetSize(&width, &height);
	auto querymaxsize = [&device,&maxsize](){
		switch (device->GetFeatureLevel())
		{
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
			maxsize = 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
			break;

		case D3D_FEATURE_LEVEL_9_3:
			maxsize = 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
			break;

		case D3D_FEATURE_LEVEL_10_0:
		case D3D_FEATURE_LEVEL_10_1:
			maxsize = 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
			break;

		default:
			maxsize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
			break;
	}};
	if (!maxsize)
	{
		querymaxsize();
	}
	assert(width > 0 && height > 0);
	assert(maxsize > 0);

	UINT twidth, theight;
	if (width > maxsize || height > maxsize)
	{
		float ar = static_cast<float>(height) / static_cast<float>(width);
		if (width > height)
		{
			twidth = static_cast<UINT>(maxsize);
			theight = static_cast<UINT>(static_cast<float>(maxsize)* ar);
		}
		else
		{
			theight = static_cast<UINT>(maxsize);
			twidth = static_cast<UINT>(static_cast<float>(maxsize) / ar);
		}
		assert(twidth <= maxsize && theight <= maxsize);
	}
	else
	{
		twidth = width;
		theight = height;
	}

	// Determine format
	WICPixelFormatGUID pixelFormat;
	hr = frame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr))
		return hr;

	WICPixelFormatGUID convertGUID;
	memcpy(&convertGUID, &pixelFormat, sizeof(WICPixelFormatGUID));

	size_t bpp = 0;

	DXGI_FORMAT format = WIC2DXGI(pixelFormat);
	if (format == DXGI_FORMAT_UNKNOWN)
	{
		for (size_t i = 0; i < _countof(guidsmap); ++i)
		{
			if (memcmp(&guidsmap[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
			{
				memcpy(&convertGUID, &guidsmap[i].target, sizeof(WICPixelFormatGUID));

				format = WIC2DXGI(guidsmap[i].target);
				assert(format != DXGI_FORMAT_UNKNOWN);
				bpp = WICBitsPerPixel(convertGUID);
				break;
			}
		}

		if (format == DXGI_FORMAT_UNKNOWN)
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}
	else
	{
		bpp = WICBitsPerPixel(pixelFormat);
	}

	if (!bpp)
		return E_FAIL;

	// Verify our target format is supported by the current device
	// (handles WDDM 1.0 or WDDM 1.1 device driver cases as well as DirectX 11.0 Runtime without 16bpp format support)
	UINT support = 0;
	hr = device->CheckFormatSupport(format, &support);
	if (FAILED(hr) || !(support & D3D11_FORMAT_SUPPORT_TEXTURE2D))
	{
		// Fallback to RGBA 32-bit format which is supported by all devices
		memcpy(&convertGUID, &GUID_WICPixelFormat32bppRGBA, sizeof(WICPixelFormatGUID));
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bpp = 32;
	}

	// Allocate temporary memory for image
	size_t rowPitch = (twidth * bpp + 7) / 8;
	size_t imageSize = rowPitch * theight;

	std::unique_ptr<uint8_t[]> temp(new uint8_t[imageSize]);

	// Load image data
	if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
		&& twidth == width
		&& theight == height)
	{
		// No format conversion or resize needed
		hr = frame->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
		if (FAILED(hr))
			return hr;
	}
	else if (twidth != width || theight != height)
	{
		// Resize
		IWICImagingFactory* pWIC = GetWICFactory();
		if (!pWIC)
			return E_NOINTERFACE;

		leo::dx::ScopedCOMObject<IWICBitmapScaler> scaler;
		hr = pWIC->CreateBitmapScaler(&scaler);
		if (FAILED(hr))
			return hr;

		hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
		if (FAILED(hr))
			return hr;

		WICPixelFormatGUID pfScaler;
		hr = scaler->GetPixelFormat(&pfScaler);
		if (FAILED(hr))
			return hr;

		if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
		{
			// No format conversion needed
			hr = scaler->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
			if (FAILED(hr))
				return hr;
		}
		else
		{
			leo::dx::ScopedCOMObject<IWICFormatConverter> FC;
			hr = pWIC->CreateFormatConverter(&FC);
			if (FAILED(hr))
				return hr;

			hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
			if (FAILED(hr))
				return hr;

			hr = FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
			if (FAILED(hr))
				return hr;
		}
	}
	else
	{
		// Format conversion but no resize
		IWICImagingFactory* pWIC = GetWICFactory();
		if (!pWIC)
			return E_NOINTERFACE;

		leo::dx::ScopedCOMObject<IWICFormatConverter> FC;
		hr = pWIC->CreateFormatConverter(&FC);
		if (FAILED(hr))
			return hr;

		hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr))
			return hr;

		hr = FC->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get());
		if (FAILED(hr))
			return hr;
	}

	// See if format is supported for auto-gen mipmaps (varies by feature level)
	bool autogen = false;
	if (deviceContext != 0 && textureView != 0) // Must have context and shader-view to auto generate mipmaps
	{
		UINT fmtSupport = 0;
		hr = device->CheckFormatSupport(format, &fmtSupport);
		if (SUCCEEDED(hr) && (fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN))
		{
			autogen = true;
		}
	}

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = twidth;
	desc.Height = theight;
	desc.MipLevels = (autogen) ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = (autogen) ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = (autogen) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = temp.get();
	initData.SysMemPitch = static_cast<UINT>(rowPitch);
	initData.SysMemSlicePitch = static_cast<UINT>(imageSize);

	ID3D11Texture2D* tex = nullptr;
	hr = device->CreateTexture2D(&desc, (autogen) ? nullptr : &initData, &tex);
	if (SUCCEEDED(hr) && tex != 0)
	{
		if (textureView != 0)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			memset(&SRVDesc, 0, sizeof(SRVDesc));
			SRVDesc.Format = format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = (autogen) ? -1 : 1;

			hr = device->CreateShaderResourceView(tex, &SRVDesc, textureView);
			if (FAILED(hr))
			{
				tex->Release();
				return hr;
			}

			if (autogen)
			{
				assert(deviceContext != 0);
				deviceContext->UpdateSubresource(tex, 0, nullptr, temp.get(), static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize));
				deviceContext->GenerateMips(*textureView);
			}
		}

		if (texture != 0)
		{
			*texture = tex;
		}
		else
		{
			leo::dx::DebugCOM(tex, sizeof("WICTextureLoader") - 1, "WICTextureLoader");
			tex->Release();
		}
	}

	return hr;
}

namespace leo
{
	//load imp
	namespace dx
	{
		HRESULT CreateWICTextureByWIC(_In_ ID3D11Device * device,
			_Out_ ID3D11Resource* *texture,
			_In_bytecount_(wicDatasize) const std::uint8_t * wicData,
			_In_ size_t wicDataSize,
			_In_opt_ ID3D11DeviceContext * deviceContext,
			_Out_opt_ ID3D11ShaderResourceView* *textureView,
			_In_ size_t maxsize
			)
		{
			if (!device || !wicData || !texture)
			{
				return E_INVALIDARG;
			}
			if (!wicDataSize)
			{
				return E_FAIL;
			}

#ifdef _M_AMD64
			if (wicDataSize > 0xFFFFFFFF)
				return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
#endif

			IWICImagingFactory* pWIC = GetWICFactory();
			if (!pWIC)
				return E_NOINTERFACE;

			// Create input stream for memory
			leo::dx::ScopedCOMObject<IWICStream> stream;
			HRESULT hr = pWIC->CreateStream(&stream);
			if (FAILED(hr))
				return hr;

			hr = stream->InitializeFromMemory(const_cast<uint8_t*>(wicData), static_cast<DWORD>(wicDataSize));
			if (FAILED(hr))
				return hr;

			// Initialize WIC
			leo::dx::ScopedCOMObject<IWICBitmapDecoder> decoder;
			hr = pWIC->CreateDecoderFromStream(stream.Get(), 0, WICDecodeMetadataCacheOnDemand, &decoder);
			if (FAILED(hr))
				return hr;

			leo::dx::ScopedCOMObject<IWICBitmapFrameDecode> frame;
			hr = decoder->GetFrame(0, &frame);
			if (FAILED(hr))
				return hr;

			hr = CreateTextureFromWIC(device, deviceContext, frame.Get(), texture, textureView, maxsize);
			if (FAILED(hr))
				return hr;

			if (texture != 0)
			{
				leo::dx::DebugCOM(*texture, sizeof("WICTextureLoader") - 1, "WICTextureLoader");
			}
			if (textureView != 0)
			{
				leo::dx::DebugCOM(*textureView, sizeof("WICTextureLoader") - 1, "WICTextureLoader");
			}
			return hr;
		}

		//--------------------------------------------------------------------------------------
		HRESULT CreateWICTextureByWIC(_In_ ID3D11Device* device,
			_Out_ ID3D11Resource* * texture,
			_In_z_ const wchar_t* szFileName,
			_In_opt_ ID3D11DeviceContext* deviceContext,
			_Out_opt_ ID3D11ShaderResourceView* * textureView,
			_In_ size_t maxsize
			)
		{
			if (!device || !szFileName || !texture )
			{
				return E_INVALIDARG;
			}

			IWICImagingFactory* pWIC = GetWICFactory();
			if (!pWIC)
				return E_NOINTERFACE;

			// Initialize WIC
			leo::dx::ScopedCOMObject<IWICBitmapDecoder> decoder;
			HRESULT hr = pWIC->CreateDecoderFromFilename(szFileName, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
			if (FAILED(hr))
				return hr;

			leo::dx::ScopedCOMObject<IWICBitmapFrameDecode> frame;
			hr = decoder->GetFrame(0, &frame);
			if (FAILED(hr))
				return hr;

			hr = CreateTextureFromWIC(device, deviceContext, frame.Get(), texture, textureView, maxsize);
			if (FAILED(hr))
				return hr;

#if defined(_DEBUG) || defined(PROFILE)
			if (texture != 0 || textureView != 0)
			{
				CHAR strFileA[MAX_PATH];
				WideCharToMultiByte(CP_ACP,
					WC_NO_BEST_FIT_CHARS,
					szFileName,
					-1,
					strFileA,
					MAX_PATH,
					nullptr,
					FALSE
					);
				const CHAR* pstrName = strrchr(strFileA, '\\');
				if (!pstrName)
				{
					pstrName = strFileA;
				}
				else
				{
					pstrName++;
				}

				if (texture != 0)
				{
					leo::dx::DebugCOM((*texture), static_cast<UINT>(strnlen_s(pstrName, MAX_PATH)), pstrName);
				}

				if (textureView != 0)
				{
					leo::dx::DebugCOM((*textureView), static_cast<UINT>(strnlen_s(pstrName, MAX_PATH)), pstrName);
				}
			}
#endif

			return hr;
		}


		//--------------------------------------------------------------------------------------
		// File: ScreenGrab.cpp
		//
		// Function for capturing a 2D texture and saving it to a file (aka a 'screenshot'
		// when used on a Direct3D 11 Render Target).
		//
		// Note these functions are useful as a light-weight runtime screen grabber. For
		// full-featured texture capture, DDS writer, and texture processing pipeline,
		// see the 'Texconv' sample and the 'DirectXTex' library.
		//

		// Does not capture 1D textures or 3D textures (volume maps)

		// Does not capture mipmap chains, only the top-most texture level is saved

		// For 2D array textures and cubemaps, it captures only the first image in the array

		template<typename T>
		using ComPtr = ATL::CComPtr<T>;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

#pragma pack(push,1)

#define DDS_MAGIC 0x20534444 // "DDS "

		struct DDS_PIXELFORMAT
		{
			uint32_t    size;
			uint32_t    flags;
			uint32_t    fourCC;
			uint32_t    RGBBitCount;
			uint32_t    RBitMask;
			uint32_t    GBitMask;
			uint32_t    BBitMask;
			uint32_t    ABitMask;
		};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE

		typedef struct
		{
			uint32_t        size;
			uint32_t        flags;
			uint32_t        height;
			uint32_t        width;
			uint32_t        pitchOrLinearSize;
			uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
			uint32_t        mipMapCount;
			uint32_t        reserved1[11];
			DDS_PIXELFORMAT ddspf;
			uint32_t        caps;
			uint32_t        caps2;
			uint32_t        caps3;
			uint32_t        caps4;
			uint32_t        reserved2;
		} DDS_HEADER;

		typedef struct
		{
			DXGI_FORMAT     dxgiFormat;
			uint32_t        resourceDimension;
			uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
			uint32_t        arraySize;
			uint32_t        reserved;
		} DDS_HEADER_DXT10;

#pragma pack(pop)

		static const DDS_PIXELFORMAT DDSPF_DXT1 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_DXT3 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_DXT5 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_BC4_UNORM =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','U'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_BC4_SNORM =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','S'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_BC5_UNORM =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','U'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_BC5_SNORM =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','S'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('R','G','B','G'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('G','R','G','B'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_YUY2 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0 };

		static const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

		static const DDS_PIXELFORMAT DDSPF_X8R8G8B8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

		static const DDS_PIXELFORMAT DDSPF_A8B8G8R8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

		static const DDS_PIXELFORMAT DDSPF_G16R16 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

		static const DDS_PIXELFORMAT DDSPF_R5G6B5 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

		static const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

		static const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
		{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

		static const DDS_PIXELFORMAT DDSPF_L8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };

		static const DDS_PIXELFORMAT DDSPF_L16 =
		{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };

		static const DDS_PIXELFORMAT DDSPF_A8L8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };

		static const DDS_PIXELFORMAT DDSPF_A8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

		static const DDS_PIXELFORMAT DDSPF_V8U8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000 };

		static const DDS_PIXELFORMAT DDSPF_Q8W8V8U8 =
		{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

		static const DDS_PIXELFORMAT DDSPF_V16U16 =
		{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

		// DXGI_FORMAT_R10G10B10A2_UNORM should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue

		// This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
		static const DDS_PIXELFORMAT DDSPF_DX10 =
		{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','1','0'), 0, 0, 0, 0, 0 };


		namespace
		{
			class auto_delete_file
			{
			public:
				auto_delete_file(HANDLE hFile) : m_handle(hFile) {}

				auto_delete_file(const auto_delete_file&) = delete;
				auto_delete_file& operator=(const auto_delete_file&) = delete;

				~auto_delete_file()
				{
					if (m_handle)
					{
						FILE_DISPOSITION_INFO info = { 0 };
						info.DeleteFile = TRUE;
						(void)SetFileInformationByHandle(m_handle, FileDispositionInfo, &info, sizeof(info));
					}
				}

				void clear() { m_handle = 0; }

			private:
				HANDLE m_handle;
			};

			
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP) || (_WIN32_WINNT > _WIN32_WINNT_WIN8)

			class auto_delete_file_wic
			{
			public:
				auto_delete_file_wic(ComPtr<IWICStream>& hFile, LPCWSTR szFile) : m_handle(hFile), m_filename(szFile) {}

				auto_delete_file_wic(const auto_delete_file_wic&) = delete;
				auto_delete_file_wic& operator=(const auto_delete_file_wic&) = delete;

				~auto_delete_file_wic()
				{
					if (m_filename)
					{
						m_handle.Release();
						DeleteFileW(m_filename);
					}
				}

				void clear() { m_filename = 0; }

			private:
				LPCWSTR m_filename;
				ComPtr<IWICStream>& m_handle;
			};
#endif
		}
#pragma warning(disable:4065)
		//--------------------------------------------------------------------------------------
		// Return the BPP for a particular format
		//--------------------------------------------------------------------------------------
		static size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
		{
			switch (fmt)
			{
#if defined(_XBOX_ONE) && defined(_TITLE)

			case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
				return 32;

			case DXGI_FORMAT_D16_UNORM_S8_UINT:
			case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
				return 24;

			case DXGI_FORMAT_R4G4_UNORM:
				return 8;

#endif // _XBOX_ONE && _TITLE

			default:
				return NumFormatBits(D3D11Mapping::MappingFormat(fmt));
			}
		}

		//--------------------------------------------------------------------------------------
		// Determines if the format is block compressed
		//--------------------------------------------------------------------------------------
		static bool IsCompressed(_In_ DXGI_FORMAT fmt)
		{
			return IsCompressedFormat(D3D11Mapping::MappingFormat(fmt));
		}


		//--------------------------------------------------------------------------------------
		// Get surface information for a particular format
		//--------------------------------------------------------------------------------------
		static void GetSurfaceInfo(_In_ size_t width,
			_In_ size_t height,
			_In_ DXGI_FORMAT fmt,
			_Out_opt_ size_t* outNumBytes,
			_Out_opt_ size_t* outRowBytes,
			_Out_opt_ size_t* outNumRows)
		{
			size_t numBytes = 0;
			size_t rowBytes = 0;
			size_t numRows = 0;

			bool bc = false;
			bool packed = false;
			bool planar = false;
			size_t bpe = 0;
			switch (fmt)
			{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				bc = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				bc = true;
				bpe = 16;
				break;

			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_YUY2:
				packed = true;
				bpe = 4;
				break;

			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				packed = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
				planar = true;
				bpe = 2;
				break;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				planar = true;
				bpe = 4;
				break;

#if defined(_XBOX_ONE) && defined(_TITLE)

			case DXGI_FORMAT_D16_UNORM_S8_UINT:
			case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
				planar = true;
				bpe = 4;
				break;

#endif
			}

			if (bc)
			{
				size_t numBlocksWide = 0;
				if (width > 0)
				{
					numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
				}
				size_t numBlocksHigh = 0;
				if (height > 0)
				{
					numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
				}
				rowBytes = numBlocksWide * bpe;
				numRows = numBlocksHigh;
				numBytes = rowBytes * numBlocksHigh;
			}
			else if (packed)
			{
				rowBytes = ((width + 1) >> 1) * bpe;
				numRows = height;
				numBytes = rowBytes * height;
			}
			else if (fmt == DXGI_FORMAT_NV11)
			{
				rowBytes = ((width + 3) >> 2) * 4;
				numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
				numBytes = rowBytes * numRows;
			}
			else if (planar)
			{
				rowBytes = ((width + 1) >> 1) * bpe;
				numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
				numRows = height + ((height + 1) >> 1);
			}
			else
			{
				size_t bpp = BitsPerPixel(fmt);
				rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
				numRows = height;
				numBytes = rowBytes * height;
			}

			if (outNumBytes)
			{
				*outNumBytes = numBytes;
			}
			if (outRowBytes)
			{
				*outRowBytes = rowBytes;
			}
			if (outNumRows)
			{
				*outNumRows = numRows;
			}
		}


		//--------------------------------------------------------------------------------------
		static DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt)
		{
			// Assumes UNORM or FLOAT; doesn't use UINT or SINT
			switch (fmt)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
			case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
			case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
			case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
			case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
			case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
			case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
			case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
			case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
			case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
			case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
			case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
			default:                                return fmt;
			}
		}


		//--------------------------------------------------------------------------------------
		static HRESULT CaptureTexture(_In_ ID3D11DeviceContext* pContext,
			_In_ ID3D11Resource* pSource,
			_Inout_ D3D11_TEXTURE2D_DESC& desc,
			_Inout_ ComPtr<ID3D11Texture2D>& pStaging)
		{
			if (!pContext || !pSource)
				return E_INVALIDARG;

			D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
			pSource->GetType(&resType);

			if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

			ComPtr<ID3D11Texture2D> pTexture;
#if defined(_XBOX_ONE) && defined(_TITLE)
			HRESULT hr = pSource->QueryInterface(IID_GRAPHICS_PPV_ARGS(&pTexture));
#else
			HRESULT hr = pSource->QueryInterface(&pTexture);
#endif
			if (FAILED(hr))
				return hr;

			assert(pTexture);

			pTexture->GetDesc(&desc);

			ComPtr<ID3D11Device> d3dDevice;
			pContext->GetDevice(&d3dDevice);

			if (desc.SampleDesc.Count > 1)
			{
				// MSAA content must be resolved before being copied to a staging texture
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;

				ComPtr<ID3D11Texture2D> pTemp;
				hr = d3dDevice->CreateTexture2D(&desc, 0,&pTemp);
				if (FAILED(hr))
					return hr;

				assert(pTemp);

				DXGI_FORMAT fmt = EnsureNotTypeless(desc.Format);

				UINT support = 0;
				hr = d3dDevice->CheckFormatSupport(fmt, &support);
				if (FAILED(hr))
					return hr;

				if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE))
					return E_FAIL;

				for (UINT item = 0; item < desc.ArraySize; ++item)
				{
					for (UINT level = 0; level < desc.MipLevels; ++level)
					{
						UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
						pContext->ResolveSubresource(pTemp, index, pSource, index, fmt);
					}
				}

				desc.BindFlags = 0;
				desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				desc.Usage = D3D11_USAGE_STAGING;

				hr = d3dDevice->CreateTexture2D(&desc, 0,&pStaging);
				if (FAILED(hr))
					return hr;

				assert(pStaging);

				pContext->CopyResource(pStaging, pTemp);
			}
			else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
			{
				// Handle case where the source is already a staging texture we can use directly
				pStaging = pTexture;
			}
			else
			{
				// Otherwise, create a staging texture from the non-MSAA source
				desc.BindFlags = 0;
				desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				desc.Usage = D3D11_USAGE_STAGING;

				hr = d3dDevice->CreateTexture2D(&desc, 0,&pStaging);
				if (FAILED(hr))
					return hr;

				assert(pStaging);

				pContext->CopyResource(pStaging, pSource);
			}

#if defined(_XBOX_ONE) && defined(_TITLE)

			if (d3dDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_IMMEDIATE_CONTEXT_FAST_SEMANTICS)
			{
				ComPtr<ID3D11DeviceX> d3dDeviceX;
				hr = d3dDevice->QueryInterface(&d3dDeviceX);
				if (FAILED(hr))
					return hr;

				ComPtr<ID3D11DeviceContextX> d3dContextX;
				hr = pContext->QueryInterface(IID_GRAPHICS_PPV_ARGS(&d3dContextX));
				if (FAILED(hr))
					return hr;

				UINT64 copyFence = d3dContextX->InsertFence(0);

				while (d3dDeviceX->IsFencePending(copyFence))
				{
					SwitchToThread();
				}
			}

#endif

			return S_OK;
		}


		//--------------------------------------------------------------------------------------
		HRESULT SaveDDSTextureToFile(_In_ ID3D11DeviceContext* pContext,
			_In_ ID3D11Resource* pSource,
			_In_z_ LPCWSTR fileName)
		{
			if (!fileName)
				return E_INVALIDARG;

			D3D11_TEXTURE2D_DESC desc = { 0 };
			ComPtr<ID3D11Texture2D> pStaging;
			HRESULT hr = CaptureTexture(pContext, pSource, desc, pStaging);
			if (FAILED(hr))
				return hr;

			// Create file
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
			UniqueHandle<HandleCloser> hFile(CreateFile2(fileName, GENERIC_WRITE | DELETE, 0, CREATE_ALWAYS, 0));
#else
			UniqueHandle<HandleCloser> hFile(CreateFileW(fileName, GENERIC_WRITE | DELETE, 0, 0, CREATE_ALWAYS, 0, 0));
#endif
			if (!hFile)
				return HRESULT_FROM_WIN32(GetLastError());

			auto_delete_file delonfail(hFile.Get());

			// Setup header
			const size_t MAX_HEADER_SIZE = sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
			uint8_t fileHeader[MAX_HEADER_SIZE];

			*reinterpret_cast<uint32_t*>(&fileHeader[0]) = DDS_MAGIC;

			auto header = reinterpret_cast<DDS_HEADER*>(&fileHeader[0] + sizeof(uint32_t));
			size_t headerSize = sizeof(uint32_t) + sizeof(DDS_HEADER);
			memset(header, 0, sizeof(DDS_HEADER));
			header->size = sizeof(DDS_HEADER);
			header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
			header->height = desc.Height;
			header->width = desc.Width;
			header->mipMapCount = 1;
			header->caps = DDS_SURFACE_FLAGS_TEXTURE;

			// Try to use a legacy .DDS pixel format for better tools support, otherwise fallback to 'DX10' header extension
			DDS_HEADER_DXT10* extHeader = nullptr;
			switch (desc.Format)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8B8G8R8, sizeof(DDS_PIXELFORMAT));    break;
			case DXGI_FORMAT_R16G16_UNORM:          memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_G16R16, sizeof(DDS_PIXELFORMAT));      break;
			case DXGI_FORMAT_R8G8_UNORM:            memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8L8, sizeof(DDS_PIXELFORMAT));        break;
			case DXGI_FORMAT_R16_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_L16, sizeof(DDS_PIXELFORMAT));         break;
			case DXGI_FORMAT_R8_UNORM:              memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_L8, sizeof(DDS_PIXELFORMAT));          break;
			case DXGI_FORMAT_A8_UNORM:              memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8, sizeof(DDS_PIXELFORMAT));          break;
			case DXGI_FORMAT_R8G8_B8G8_UNORM:       memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_R8G8_B8G8, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_G8R8_G8B8_UNORM:       memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_G8R8_G8B8, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_BC1_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT1, sizeof(DDS_PIXELFORMAT));        break;
			case DXGI_FORMAT_BC2_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT3, sizeof(DDS_PIXELFORMAT));        break;
			case DXGI_FORMAT_BC3_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT5, sizeof(DDS_PIXELFORMAT));        break;
			case DXGI_FORMAT_BC4_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC4_UNORM, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_BC4_SNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC4_SNORM, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_BC5_UNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC5_UNORM, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_BC5_SNORM:             memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC5_SNORM, sizeof(DDS_PIXELFORMAT));   break;
			case DXGI_FORMAT_B5G6R5_UNORM:          memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_R5G6B5, sizeof(DDS_PIXELFORMAT));      break;
			case DXGI_FORMAT_B5G5R5A1_UNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A1R5G5B5, sizeof(DDS_PIXELFORMAT));    break;
			case DXGI_FORMAT_R8G8_SNORM:            memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_V8U8, sizeof(DDS_PIXELFORMAT));        break;
			case DXGI_FORMAT_R8G8B8A8_SNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_Q8W8V8U8, sizeof(DDS_PIXELFORMAT));    break;
			case DXGI_FORMAT_R16G16_SNORM:          memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_V16U16, sizeof(DDS_PIXELFORMAT));      break;
			case DXGI_FORMAT_B8G8R8A8_UNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8R8G8B8, sizeof(DDS_PIXELFORMAT));    break; // DXGI 1.1
			case DXGI_FORMAT_B8G8R8X8_UNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_X8R8G8B8, sizeof(DDS_PIXELFORMAT));    break; // DXGI 1.1
			case DXGI_FORMAT_YUY2:                  memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_YUY2, sizeof(DDS_PIXELFORMAT));        break; // DXGI 1.2
			case DXGI_FORMAT_B4G4R4A4_UNORM:        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A4R4G4B4, sizeof(DDS_PIXELFORMAT));    break; // DXGI 1.2

																																						 // Legacy D3DX formats using D3DFMT enum value as FourCC
			case DXGI_FORMAT_R32G32B32A32_FLOAT:    header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 116; break; // D3DFMT_A32B32G32R32F
			case DXGI_FORMAT_R16G16B16A16_FLOAT:    header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 113; break; // D3DFMT_A16B16G16R16F
			case DXGI_FORMAT_R16G16B16A16_UNORM:    header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 36;  break; // D3DFMT_A16B16G16R16
			case DXGI_FORMAT_R16G16B16A16_SNORM:    header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 110; break; // D3DFMT_Q16W16V16U16
			case DXGI_FORMAT_R32G32_FLOAT:          header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 115; break; // D3DFMT_G32R32F
			case DXGI_FORMAT_R16G16_FLOAT:          header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 112; break; // D3DFMT_G16R16F
			case DXGI_FORMAT_R32_FLOAT:             header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 114; break; // D3DFMT_R32F
			case DXGI_FORMAT_R16_FLOAT:             header->ddspf.size = sizeof(DDS_PIXELFORMAT); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 111; break; // D3DFMT_R16F

			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
			case DXGI_FORMAT_A8P8:
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

			default:
				memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DX10, sizeof(DDS_PIXELFORMAT));

				headerSize += sizeof(DDS_HEADER_DXT10);
				extHeader = reinterpret_cast<DDS_HEADER_DXT10*>(reinterpret_cast<uint8_t*>(&fileHeader[0]) + sizeof(uint32_t) + sizeof(DDS_HEADER));
				memset(extHeader, 0, sizeof(DDS_HEADER_DXT10));
				extHeader->dxgiFormat = desc.Format;
				extHeader->resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
				extHeader->arraySize = 1;
				break;
			}

			size_t rowPitch, slicePitch, rowCount;
			GetSurfaceInfo(desc.Width, desc.Height, desc.Format, &slicePitch, &rowPitch, &rowCount);

			if (IsCompressed(desc.Format))
			{
				header->flags |= DDS_HEADER_FLAGS_LINEARSIZE;
				header->pitchOrLinearSize = static_cast<uint32_t>(slicePitch);
			}
			else
			{
				header->flags |= DDS_HEADER_FLAGS_PITCH;
				header->pitchOrLinearSize = static_cast<uint32_t>(rowPitch);
			}

			// Setup pixels
			std::unique_ptr<uint8_t[]> pixels(new (std::nothrow) uint8_t[slicePitch]);
			if (!pixels)
				return E_OUTOFMEMORY;

			D3D11_MAPPED_SUBRESOURCE mapped;
			hr = pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped);
			if (FAILED(hr))
				return hr;

			auto sptr = reinterpret_cast<const uint8_t*>(mapped.pData);
			if (!sptr)
			{
				pContext->Unmap(pStaging, 0);
				return E_POINTER;
			}

			uint8_t* dptr = pixels.get();

			size_t msize = std::min<size_t>(rowPitch, mapped.RowPitch);
			for (size_t h = 0; h < rowCount; ++h)
			{
				memcpy_s(dptr, rowPitch, sptr, msize);
				sptr += mapped.RowPitch;
				dptr += rowPitch;
			}

			pContext->Unmap(pStaging, 0);

			// Write header & pixels
			DWORD bytesWritten;
			if (!WriteFile(hFile.Get(), fileHeader, static_cast<DWORD>(headerSize), &bytesWritten, 0))
				return HRESULT_FROM_WIN32(GetLastError());

			if (bytesWritten != headerSize)
				return E_FAIL;

			if (!WriteFile(hFile.Get(), pixels.get(), static_cast<DWORD>(slicePitch), &bytesWritten, 0))
				return HRESULT_FROM_WIN32(GetLastError());

			if (bytesWritten != slicePitch)
				return E_FAIL;

			delonfail.clear();

			return S_OK;
		}

		//--------------------------------------------------------------------------------------
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP) || (_WIN32_WINNT > _WIN32_WINNT_WIN8)

		namespace DirectX
		{
			extern bool _IsWIC2();
			extern IWICImagingFactory* _GetWIC();
		}

		HRESULT SaveWICTextureToFile(_In_ ID3D11DeviceContext* pContext,
			_In_ ID3D11Resource* pSource,
			_In_ REFGUID guidContainerFormat,
			_In_z_ LPCWSTR fileName,
			_In_opt_ const GUID* targetFormat,
			_In_opt_ std::function<void(IPropertyBag2*)> setCustomProps)
		{
			if (!fileName)
				return E_INVALIDARG;

			D3D11_TEXTURE2D_DESC desc = { 0 };
			ComPtr<ID3D11Texture2D> pStaging;
			HRESULT hr = CaptureTexture(pContext, pSource, desc, pStaging);
			if (FAILED(hr))
				return hr;

			// Determine source format's WIC equivalent
			WICPixelFormatGUID pfGuid;
			bool sRGB = false;
			switch (desc.Format)
			{
			case DXGI_FORMAT_R32G32B32A32_FLOAT:            pfGuid = GUID_WICPixelFormat128bppRGBAFloat; break;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:            pfGuid = GUID_WICPixelFormat64bppRGBAHalf; break;
			case DXGI_FORMAT_R16G16B16A16_UNORM:            pfGuid = GUID_WICPixelFormat64bppRGBA; break;
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:    pfGuid = GUID_WICPixelFormat32bppRGBA1010102XR; break; // DXGI 1.1
			case DXGI_FORMAT_R10G10B10A2_UNORM:             pfGuid = GUID_WICPixelFormat32bppRGBA1010102; break;
			case DXGI_FORMAT_B5G5R5A1_UNORM:                pfGuid = GUID_WICPixelFormat16bppBGRA5551; break;
			case DXGI_FORMAT_B5G6R5_UNORM:                  pfGuid = GUID_WICPixelFormat16bppBGR565; break;
			case DXGI_FORMAT_R32_FLOAT:                     pfGuid = GUID_WICPixelFormat32bppGrayFloat; break;
			case DXGI_FORMAT_R16_FLOAT:                     pfGuid = GUID_WICPixelFormat16bppGrayHalf; break;
			case DXGI_FORMAT_R16_UNORM:                     pfGuid = GUID_WICPixelFormat16bppGray; break;
			case DXGI_FORMAT_R8_UNORM:                      pfGuid = GUID_WICPixelFormat8bppGray; break;
			case DXGI_FORMAT_A8_UNORM:                      pfGuid = GUID_WICPixelFormat8bppAlpha; break;

			case DXGI_FORMAT_R8G8B8A8_UNORM:
				pfGuid = GUID_WICPixelFormat32bppRGBA;
				break;

			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
				pfGuid = GUID_WICPixelFormat32bppRGBA;
				sRGB = true;
				break;

			case DXGI_FORMAT_B8G8R8A8_UNORM: // DXGI 1.1
				pfGuid = GUID_WICPixelFormat32bppBGRA;
				break;

			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: // DXGI 1.1
				pfGuid = GUID_WICPixelFormat32bppBGRA;
				sRGB = true;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM: // DXGI 1.1
				pfGuid = GUID_WICPixelFormat32bppBGR;
				break;

			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: // DXGI 1.1
				pfGuid = GUID_WICPixelFormat32bppBGR;
				sRGB = true;
				break;

			default:
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}

			IWICImagingFactory* pWIC = GetWICFactory();
			if (!pWIC)
				return E_NOINTERFACE;

			ComPtr<IWICStream> stream;
			hr = pWIC->CreateStream(&stream);
			if (FAILED(hr))
				return hr;

			hr = stream->InitializeFromFilename(fileName, GENERIC_WRITE);
			if (FAILED(hr))
				return hr;

			auto_delete_file_wic delonfail(stream, fileName);

			ComPtr<IWICBitmapEncoder> encoder;
			hr = pWIC->CreateEncoder(guidContainerFormat, 0,&encoder);
			if (FAILED(hr))
				return hr;

			hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
			if (FAILED(hr))
				return hr;

			ComPtr<IWICBitmapFrameEncode> frame;
			ComPtr<IPropertyBag2> props;
			hr = encoder->CreateNewFrame(&frame,&props);
			if (FAILED(hr))
				return hr;

			if (targetFormat && memcmp(&guidContainerFormat, &GUID_ContainerFormatBmp, sizeof(WICPixelFormatGUID)) == 0 && IsWIC2())
			{
				// Opt-in to the WIC2 support for writing 32-bit Windows BMP files with an alpha channel
				PROPBAG2 option = { 0 };
				option.pstrName = L"EnableV5Header32bppBGRA";

				VARIANT varValue;
				varValue.vt = VT_BOOL;
				varValue.boolVal = VARIANT_TRUE;
				(void)props->Write(1, &option, &varValue);
			}

			if (setCustomProps)
			{
				setCustomProps(props);
			}

			hr = frame->Initialize(props);
			if (FAILED(hr))
				return hr;

			hr = frame->SetSize(desc.Width, desc.Height);
			if (FAILED(hr))
				return hr;

			hr = frame->SetResolution(72, 72);
			if (FAILED(hr))
				return hr;

			// Pick a target format
			WICPixelFormatGUID targetGuid;
			if (targetFormat)
			{
				targetGuid = *targetFormat;
			}
			else
			{
				// Screenshots don�t typically include the alpha channel of the render target
				switch (desc.Format)
				{
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
				case DXGI_FORMAT_R32G32B32A32_FLOAT:
				case DXGI_FORMAT_R16G16B16A16_FLOAT:
					if (IsWIC2())
					{
						targetGuid = GUID_WICPixelFormat96bppRGBFloat;
					}
					else
					{
						targetGuid = GUID_WICPixelFormat24bppBGR;
					}
					break;
#endif

				case DXGI_FORMAT_R16G16B16A16_UNORM: targetGuid = GUID_WICPixelFormat48bppBGR; break;
				case DXGI_FORMAT_B5G5R5A1_UNORM:     targetGuid = GUID_WICPixelFormat16bppBGR555; break;
				case DXGI_FORMAT_B5G6R5_UNORM:       targetGuid = GUID_WICPixelFormat16bppBGR565; break;

				case DXGI_FORMAT_R32_FLOAT:
				case DXGI_FORMAT_R16_FLOAT:
				case DXGI_FORMAT_R16_UNORM:
				case DXGI_FORMAT_R8_UNORM:
				case DXGI_FORMAT_A8_UNORM:
					targetGuid = GUID_WICPixelFormat8bppGray;
					break;

				default:
					targetGuid = GUID_WICPixelFormat24bppBGR;
					break;
				}
			}

			hr = frame->SetPixelFormat(&targetGuid);
			if (FAILED(hr))
				return hr;

			if (targetFormat && memcmp(targetFormat, &targetGuid, sizeof(WICPixelFormatGUID)) != 0)
			{
				// Requested output pixel format is not supported by the WIC codec
				return E_FAIL;
			}

			// Encode WIC metadata
			ComPtr<IWICMetadataQueryWriter> metawriter;
			if (SUCCEEDED(frame->GetMetadataQueryWriter(&metawriter)))
			{
				PROPVARIANT value;
				PropVariantInit(&value);

				value.vt = VT_LPSTR;
				value.pszVal = "DirectXTK";

				if (memcmp(&guidContainerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) == 0)
				{
					// Set Software name
					(void)metawriter->SetMetadataByName(L"/tEXt/{str=Software}", &value);

					// Set sRGB chunk
					if (sRGB)
					{
						value.vt = VT_UI1;
						value.bVal = 0;
						(void)metawriter->SetMetadataByName(L"/sRGB/RenderingIntent", &value);
					}
				}
#if defined(_XBOX_ONE) && defined(_TITLE)
				else if (memcmp(&guidContainerFormat, &GUID_ContainerFormatJpeg, sizeof(GUID)) == 0)
				{
					// Set Software name
					(void)metawriter->SetMetadataByName(L"/app1/ifd/{ushort=305}", &value);

					if (sRGB)
					{
						// Set EXIF Colorspace of sRGB
						value.vt = VT_UI2;
						value.uiVal = 1;
						(void)metawriter->SetMetadataByName(L"/app1/ifd/exif/{ushort=40961}", &value);
					}
				}
				else if (memcmp(&guidContainerFormat, &GUID_ContainerFormatTiff, sizeof(GUID)) == 0)
				{
					// Set Software name
					(void)metawriter->SetMetadataByName(L"/ifd/{ushort=305}", &value);

					if (sRGB)
					{
						// Set EXIF Colorspace of sRGB
						value.vt = VT_UI2;
						value.uiVal = 1;
						(void)metawriter->SetMetadataByName(L"/ifd/exif/{ushort=40961}", &value);
					}
				}
#else
				else
				{
					// Set Software name
					(void)metawriter->SetMetadataByName(L"System.ApplicationName", &value);

					if (sRGB)
					{
						// Set EXIF Colorspace of sRGB
						value.vt = VT_UI2;
						value.uiVal = 1;
						(void)metawriter->SetMetadataByName(L"System.Image.ColorSpace", &value);
					}
				}
#endif
			}

			D3D11_MAPPED_SUBRESOURCE mapped;
			hr = pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped);
			if (FAILED(hr))
				return hr;

			if (memcmp(&targetGuid, &pfGuid, sizeof(WICPixelFormatGUID)) != 0)
			{
				// Conversion required to write
				ComPtr<IWICBitmap> source;
				hr = pWIC->CreateBitmapFromMemory(desc.Width, desc.Height, pfGuid,
					mapped.RowPitch, mapped.RowPitch * desc.Height,
					reinterpret_cast<BYTE*>(mapped.pData),&source);
				if (FAILED(hr))
				{
					pContext->Unmap(pStaging, 0);
					return hr;
				}

				ComPtr<IWICFormatConverter> FC;
				hr = pWIC->CreateFormatConverter(&FC);
				if (FAILED(hr))
				{
					pContext->Unmap(pStaging, 0);
					return hr;
				}

				BOOL canConvert = FALSE;
				hr = FC->CanConvert(pfGuid, targetGuid, &canConvert);
				if (FAILED(hr) || !canConvert)
				{
					return E_UNEXPECTED;
				}

				hr = FC->Initialize(source, targetGuid, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);
				if (FAILED(hr))
				{
					pContext->Unmap(pStaging, 0);
					return hr;
				}

				WICRect rect = { 0, 0, static_cast<INT>(desc.Width), static_cast<INT>(desc.Height) };
				hr = frame->WriteSource(FC, &rect);
				if (FAILED(hr))
				{
					pContext->Unmap(pStaging, 0);
					return hr;
				}
			}
			else
			{
				// No conversion required
				hr = frame->WritePixels(desc.Height, mapped.RowPitch, mapped.RowPitch * desc.Height, reinterpret_cast<BYTE*>(mapped.pData));
				if (FAILED(hr))
					return hr;
			}

			pContext->Unmap(pStaging, 0);

			hr = frame->Commit();
			if (FAILED(hr))
				return hr;

			hr = encoder->Commit();
			if (FAILED(hr))
				return hr;

			delonfail.clear();

			return S_OK;
		}

#endif // !WINAPI_FAMILY || (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP) || (_WIN32_WINNT > _WIN32_WINNT_WIN8)


	}

	//helper imp
	namespace dx
	{
		void d3d11_timer::start(std::uint64_t currframe, ID3D11Device* device, ID3D11DeviceContext* context)
		{
			assert(!started);
			assert(!finished);

			if (!disjoint[currframe])
			{
				D3D11_QUERY_DESC desc;
				desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
				desc.MiscFlags = 0;
				//there should checke error and throw exception,but i am not write exception hpp
				device->CreateQuery(&desc, &disjoint[currframe]);
				desc.Query = D3D11_QUERY_TIMESTAMP;
				device->CreateQuery(&desc, &timestampstart[currframe]);
				device->CreateQuery(&desc, &timestampend[currframe]);
			}

			context->Begin(disjoint[currframe]);
			context->End(timestampstart[currframe]);

			started = true;
		}

		void d3d11_timer::end(std::uint64_t currframe, ID3D11DeviceContext* context)
		{
			assert(started);
			assert(!finished);
			//this mean insert the timestampend
			//http://msdn.microsoft.com/en-us/library/windows/desktop/ff476191(v=vs.85).aspx
			context->End(timestampend[currframe]);
			context->End(disjoint[currframe]);

			started = false;
			finished = true;
		}

		float d3d11_timer::time(std::uint64_t currframe, ID3D11DeviceContext* context)
		{
			if (!finished)
				return 0.f;
			finished = false;
			if (!disjoint[currframe])
				return 0.f;

			//maybe you can record query time

			std::uint64_t starttime = 0;
			while (context->GetData(timestampstart[currframe], &starttime, sizeof(starttime), 0) != S_OK)
				;
			std::uint64_t endtime = 0;
			while (context->GetData(timestampend[currframe], &endtime, sizeof(endtime), 0) != S_OK)
				;
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointdata;
			while (context->GetData(disjoint[currframe], &disjointdata, sizeof(disjointdata), 0) != S_OK)
				;

			float time = 0.0f;
			if (!disjointdata.Disjoint)
			{
				std::uint64_t delta = endtime - starttime;
				float frequency = static_cast<float>(disjointdata.Frequency);
				time = (delta / frequency);
			}
			return time;
		}

		profiler profiler::global_profiler;

		void profiler::init(ID3D11Device* dev, ID3D11DeviceContext* con)
		{
			this->device = dev;
			this->context = con;
		}

		void profiler::startprofile(const std::wstring& name)
		{
			//Todo : query "game setting"
			auto & profile = profiles[name];
			profile.start(currframe, device, context);
		}

		void profiler::endprofile(const std::wstring& name)
		{
			//Todo : query "game setting"
			auto & profile = profiles[name];
			profile.end(currframe, context);
		}
	

		profileblock::profileblock(const std::wstring& name)
			:name(name)
		{
			profiler::global_profiler.startprofile(name);
		}

		profileblock::~profileblock()
		{
			profiler::global_profiler.endprofile(name);
		}

	}
}

#include  <leo2DMath.hpp>

namespace leo {
	namespace dx {
		D3D11_RECT ScissorRectFromNDCRect(const ops::Rect& ndcRect, const std::pair<uint16, uint16>& size) {
			auto Top = static_cast<LONG>(ndcRect.tlbr.x * size.second);
			auto Left = static_cast<LONG>(ndcRect.tlbr.y * size.first);
			auto Bottom = static_cast<LONG>(ndcRect.tlbr.z * size.second);
			auto Right = static_cast<LONG>(ndcRect.tlbr.w * size.first);

			return CD3D11_RECT{ Left,Top,Right,Bottom };
		}
	}
}