#include "effect.h"
#include "CoreObject.hpp"
namespace leo{
	class EffectSkeleton :public Effect, ABSTRACT{
	public:
		void Apply(ID3D11DeviceContext* context);

		void WorldMatrix(CXMMATRIX matrix, ID3D11DeviceContext* context = nullptr);
		void WorldViewProjMatrix(CXMMATRIX matrix, ID3D11DeviceContext* context = nullptr);
		void EyePos(const float3& pos, ID3D11DeviceContext* context = nullptr);

		void SkinMatrix(float4x4Object * globalmatrix, std::uint32_t numJoint);
		void Light(const DirectionLight& dl, ID3D11DeviceContext* context = nullptr);
		void Light(const PointLight& pl, ID3D11DeviceContext* context = nullptr);
		void Light(const SpotLight& sl, ID3D11DeviceContext* context = nullptr);

		void Mat(const Material& mat, ID3D11DeviceContext* context = nullptr);

		void DiffuseSRV(ID3D11ShaderResourceView * const diff, ID3D11DeviceContext* context = nullptr);
		void NormalMapSRV(ID3D11ShaderResourceView * const nmap, ID3D11DeviceContext* context = nullptr);

		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
	public:
		static const std::unique_ptr<EffectSkeleton>& GetInstance(ID3D11Device* device = nullptr);
	};
}