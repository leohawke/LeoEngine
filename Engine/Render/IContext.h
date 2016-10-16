/*! \file Engine\Render\IContext.h
\ingroup Engine
\brief 绘制创建接口类。
*/
#ifndef LE_RENDER_IContext_h
#define LE_RENDER_IContext_h 1

#include "DeviceCaps.h"
#include "ITexture.hpp"

namespace platform {
	namespace Render {
		const class delayptr_t
		{
		public:
			template<typename _type>
			inline
				operator _type*() const
			{
				return reinterpret_cast<_type*>(-1);
			}

			template<class _tClass, typename _type>
			inline
				operator _type _tClass::*() const
			{
				return reinterpret_cast<_type _tClass::*>(-1);
			}
			template<typename _type>
			bool
				equals(const _type& rhs) const
			{
				return rhs == reinterpret_cast<_type>(-1);
			}

			void operator&() const = delete;
		} delayptr = {};

		template<typename _type>
		inline bool
			operator==(delayptr_t lhs, const _type& rhs)
		{
			return lhs.equals(rhs);
		}
		template<typename _type>
		inline bool
			operator==(const _type& lhs, delayptr_t rhs)
		{
			return rhs.equals(lhs);
		}

		template<typename _type>
		inline bool
			operator!=(delayptr_t lhs, const _type& rhs)
		{
			return !lhs.equals(rhs);
		}
		template<typename _type>
		inline bool
			operator!=(const _type& lhs, delayptr_t rhs)
		{
			return !rhs.equals(lhs);
		}

		class Device {
		public:
			virtual Caps& GetCaps() = 0;

			virtual Texture1D* CreateTexture(uint16 width, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, ElementInitData const * init_data = nullptr) = 0;

			virtual Texture2D* CreateTexture(uint16 width, uint16 height, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, ElementInitData const * init_data = nullptr) = 0;

			virtual Texture3D* CreateTexture(uint16 width, uint16 height, uint16 depth, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, ElementInitData const * init_data = nullptr) = 0;

			virtual TextureCube* CreateTextureCube(uint16 size, uint8 num_mipmaps, uint8 array_size,
				EFormat format, uint32 access, SampleDesc sample_info, ElementInitData const * init_data = nullptr) = 0;
		};

		class Context {
		public:
			virtual Device& GetDevice() = 0;
		private:
			virtual void CreateDeviceAndDisplay() = 0;
		public:
			static Context& Instance();
		};
	}
}

#endif