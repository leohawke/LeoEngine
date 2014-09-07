#pragma once


#include <IndePlatform\Singleton.hpp>
#include <IndePlatform\memory.hpp>
struct ID3D11Device;
struct ID3D11Buffer;
namespace leo
{
	class Camera;

	class Axis : public leo::Singleton<Axis>
	{
	public:
		Axis(ID3D11Device* device);
		~Axis();
	public:
		void Render(ID3D11DeviceContext* context, const Camera& camera);
	public:
		static const std::unique_ptr<Axis>& GetInstance(ID3D11Device* device = nullptr);
	private:
		ID3D11Buffer* mVertexBuffer = nullptr;
	};
}