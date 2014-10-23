#pragma once

#include "Mgr.hpp"
#include "IndePlatform\leoint.hpp"
#ifdef DEBUG
#include <D3D11ShaderTracing.h>
#else
typedef enum D3D11_SHADER_TYPE : leo::uint8 { 
	D3D11_VERTEX_SHADER = 1,
	D3D11_HULL_SHADER = 2,
	D3D11_DOMAIN_SHADER = 3,
	D3D11_GEOMETRY_SHADER = 4,
	D3D11_PIXEL_SHADER = 5,
	D3D11_COMPUTE_SHADER = 6
} D3D11_SHADER_TYPE;
#endif
namespace leo
{
	class ShaderMgr
	{
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
		class ShaderBlob
		{
		public:
			DefGetter(const _NOEXCEPT, std::uint8_t*, BufferPointer, m_buffer.get());
			DefGetter(const _NOEXCEPT, std::size_t, BufferSize, m_size);
			DefGetter(const _NOEXCEPT, std::size_t, BufferSid, m_sid);

			ShaderBlob(const wchar_t * filename)
				:m_buffer(nullptr), m_size(0), m_sid(0)
			{
				Load(filename);
			}
			void Load(const wchar_t* filename);
		private:
			std::unique_ptr<std::uint8_t[]> m_buffer;
			std::size_t m_size;
			std::size_t m_sid;
		};
	public:
		ShaderMgr()
		{
			Delegate::GetInstance();
		}

		ShaderBlob CreateBlob(const wchar_t* filename)
		{
			return ShaderBlob(filename);
		}
		ShaderBlob CreateBlob(const std::wstring& filename)
		{
			return CreateBlob(filename.c_str());
		}

		ID3D11VertexShader* CreateVertexShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr, const D3D11_INPUT_ELEMENT_DESC * layoutdesc = nullptr, win::uint arraysize = 0, ID3D11InputLayout** pLayout = nullptr);

		ID3D11PixelShader*	CreatePixelShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr);
		ID3D11GeometryShader* CreateGeometryShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr);
		ID3D11HullShader*	CreateHullShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr);
		ID3D11DomainShader* CreateDomainShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr);
		ID3D11ComputeShader* CreateComputeShader(const ShaderBlob& blob, ID3D11ClassLinkage* linkage = nullptr);

		ID3D11InputLayout* CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC*, win::uint arraysize = 0/*hint*/);
	private:
		struct Shader
		{
			union
			{
				ID3D11VertexShader* vertex_shader;
				ID3D11PixelShader*	pixel_shader;
				ID3D11GeometryShader* geometry_shader;
				ID3D11HullShader*	hull_shader;
				ID3D11DomainShader* domain_shader;
				ID3D11ComputeShader* compute_shader;
			};
			D3D11_SHADER_TYPE  type;
		};
		static std::unordered_map<std::size_t, Shader> mShaders;
		static std::unordered_map<const D3D11_INPUT_ELEMENT_DESC*, ID3D11InputLayout*> mInputLayouts;
	};
}