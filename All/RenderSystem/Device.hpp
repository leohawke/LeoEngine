// CopyRight 2014. LeoHawke.

#ifndef RenderSystem_Device_Hpp
#define RenderSystem_Device_Hpp

#include "d3dx.hpp"
/*
IndePlatform\memory.hpp
IndePlatform\platform.hpp
<d3d11.h>
<d3d11shader.h>
<d3dcompiler.h>
<d3d11sdklayers.h>
<d3d11shadertracing.h>
*/
#include "..\IndePlatform\ThreadSync.hpp"


namespace DirectX
{
	class Device
	{
	private:
		ID3D11Device* mD3D11Device = nullptr;
		//ImmediateContext
		ID3D11DeviceContext* mD3D11DeviceContext = nullptr;
		// Storing class linkage
		ID3D11ClassLinkage* mClassLinkage = nullptr;

		struct ThreadInfo
		{
			ID3D11DeviceContext* mContext = nullptr;
			leo::Event mEvent;

			ThreadInfo(ID3D11DeviceContext*context)
				:mContext(context), mEvent(false)
			{}

			~ThreadInfo()
			{
				ReleaseCOM(mContext);
			}
		};

		Device();

#ifdef DEBUG
		//A debug interface controls debug settings, validates pipeline 
		//Warning: only be used if the debug layer is turned on
		ID3D11Debug* mDebug = nullptr;
		//An information-queue interface stores, retrieves, and filters debug messages
		ID3D11InfoQueue * mInfoQuene = nullptr;
#endif
	public:
		Device(ID3D11Device* device);
		~Device();

		//ImmediateContext
		inline ID3D11DeviceContext* GetContext()
		{
			return mD3D11DeviceContext;
		}

		inline ID3D11ClassLinkage* GetClassLinkage()
		{
			return mClassLinkage;
		}

		void Release();
		
		inline operator ID3D11Device *()
		{
			return mD3D11Device;
		}

		inline ID3D11Device * operator->() const
		{
			assert(mD3D11Device);
#ifdef DEBUG
			if (EL_NO_EXCEPTION != mExceptionsLevel)
			{
				clearStoredErrorMessages();
			}
#endif
			return mD3D11Device;
		}

		ID3D11Device * operator=(ID3D11Device * device);

		operator bool() const;

#ifdef DEBUG
		const void clearStoredErrorMessages() const;

		const String getErrorDescription(const HRESULT hr = NO_ERROR) const;

		inline const bool isError() const
		{
			if (D3D_NO_EXCEPTION == mExceptionsErrorLevel)
			{
				return  false;
			}

			return _getErrorsFromQueue();
		}

		const bool _getErrorsFromQueue() const;
		

		enum ExceptionsLevel
		{
			EL_NO_EXCEPTION,
			EL_CORRUPTION,
			EL_ERROR,
			EL_WARNING,
			EL_INFO,
		};

		static ExceptionsLevel mExceptionsLevel;
		static void SetExceptionsLevel(const ExceptionsLevel exceptionslevel);
		static const ExceptionsLevel getExceptionsLevel();
#endif
	};
}

#endif