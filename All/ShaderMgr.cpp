#include <fstream>

#include "ShaderMgr.h"
#include "COM.hpp"
#include "DirectXTex.h"
#include "file.hpp"
#include "exception.hpp"
#include "file.hpp"
namespace leo
{
	std::unordered_map<std::size_t, ShaderMgr::Shader> ShaderMgr::mShaders{};
	std::unordered_map<const D3D11_INPUT_ELEMENT_DESC *, ID3D11InputLayout*> ShaderMgr::mInputLayouts{};

	ShaderMgr::Delegate::~Delegate()
	{
		for (auto & shader : mShaders)
		{
			switch (shader.second.type)
			{
			case D3D11_SHADER_TYPE::D3D11_VERTEX_SHADER:
				leo::win::ReleaseCOM(shader.second.vertex_shader);
				break;
			case D3D11_SHADER_TYPE::D3D11_PIXEL_SHADER:
				leo::win::ReleaseCOM(shader.second.pixel_shader);
				break;
			case D3D11_SHADER_TYPE::D3D11_GEOMETRY_SHADER:
				leo::win::ReleaseCOM(shader.second.geometry_shader);
				break;
			case D3D11_SHADER_TYPE::D3D11_HULL_SHADER:
				leo::win::ReleaseCOM(shader.second.hull_shader);
				break;
			case D3D11_SHADER_TYPE::D3D11_DOMAIN_SHADER:
				leo::win::ReleaseCOM(shader.second.domain_shader);
				break;
			case D3D11_SHADER_TYPE::D3D11_COMPUTE_SHADER:
				leo::win::ReleaseCOM(shader.second.compute_shader);
				break;
			default:
				break;
			}
		}
		for (auto & layout : mInputLayouts)
		{
			leo::win::ReleaseCOM(layout.second);
		}
	}

	void ShaderMgr::ShaderBlob::Load(const wchar_t * filename)
	{
		auto fin = win::File::OpenNoThrow(filename, win::File::TO_READ | win::File::NO_CREATE);
		m_sid = hash(filename);
		m_size = static_cast<size_t>(fin->GetSize());
		m_buffer = std::make_unique<std::uint8_t[]>(m_size);
		fin->Read(m_buffer.get(),m_size,0);
	}

	ID3D11VertexShader* ShaderMgr::CreateVertexShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage, const D3D11_INPUT_ELEMENT_DESC * layoutdesc, win::uint arraysize, ID3D11InputLayout** pLayout)
	{
		ID3D11VertexShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.vertex_shader;
			if (layoutdesc && pLayout)
				*pLayout = mInputLayouts[layoutdesc];
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_VERTEX_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreateVertexShader(pointer, size, linkage, &shader);
			if (mInputLayouts.find(layoutdesc) == mInputLayouts.end() && layoutdesc)
			{
				global::globalD3DDevice->CreateInputLayout(layoutdesc, arraysize, pointer, size, pLayout);
				mInputLayouts[layoutdesc] = *pLayout;
			}
			Shader shaderelem;
			shaderelem.vertex_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_VERTEX_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}
	
	ID3D11PixelShader*	ShaderMgr::CreatePixelShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage)
	{
		ID3D11PixelShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.pixel_shader;
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_PIXEL_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreatePixelShader(pointer, size, linkage, &shader);
			Shader shaderelem;
			shaderelem.pixel_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_PIXEL_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}
	
	ID3D11GeometryShader*ShaderMgr::CreateGeometryShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage)
	{
		ID3D11GeometryShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.geometry_shader;
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_GEOMETRY_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreateGeometryShader(pointer, size, linkage, &shader);
			Shader shaderelem;
			shaderelem.geometry_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_GEOMETRY_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}
	
	ID3D11HullShader*	ShaderMgr::CreateHullShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage)
	{
		ID3D11HullShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.hull_shader;
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_HULL_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreateHullShader(pointer, size, linkage, &shader);
			Shader shaderelem;
			shaderelem.hull_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_HULL_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}

	ID3D11DomainShader* ShaderMgr::CreateDomainShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage)
	{
		ID3D11DomainShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.domain_shader;
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_DOMAIN_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreateDomainShader(pointer, size, linkage, &shader);
			Shader shaderelem;
			shaderelem.domain_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_DOMAIN_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}

	ID3D11ComputeShader* ShaderMgr::CreateComputeShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage)
	{
		ID3D11ComputeShader* shader = 0;

		auto sid = blob.GetBufferSid();
		// Does it already exist?
		if (mShaders.find(sid) != mShaders.end())
		{
			auto & shaderelem = mShaders[sid];
			shader = shaderelem.compute_shader;
			if (shaderelem.type != D3D11_SHADER_TYPE::D3D11_COMPUTE_SHADER)
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "内存里的值错误");
		}
		else
		{
			auto size = blob.GetBufferSize();
			auto pointer = blob.GetBufferPointer();
			global::globalD3DDevice->CreateComputeShader(pointer, size, linkage, &shader);
			Shader shaderelem;
			shaderelem.compute_shader = shader;
			shaderelem.type = D3D11_SHADER_TYPE::D3D11_COMPUTE_SHADER;
			mShaders[sid] = shaderelem;
		}
		return shader;
	}

	ID3D11InputLayout* ShaderMgr::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layoutdesc, win::uint arraysize/*hint*/)
	{
		auto it = mInputLayouts.find(layoutdesc);
		if (it != mInputLayouts.end())
			return it->second;
		return nullptr;
	}
}