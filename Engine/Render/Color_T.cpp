#include "Color_T.hpp"

namespace platform::M{
		template Color_T<float>::Color_T(float const * rhs) lnoexcept;
		template Color_T<float>::Color_T(Color const & rhs) lnoexcept;
		template Color_T<float>::Color_T(Color&& rhs) lnoexcept;
		template Color_T<float>::Color_T(float r, float g, float b, float a) lnoexcept;
		template Color_T<float>::Color_T(uint32 dw) lnoexcept;
		template void Color_T<float>::RGBA(uint8& R, uint8& G, uint8& B, uint8& A) const lnoexcept;
		template uint32 Color_T<float>::ARGB() const lnoexcept;
		template uint32 Color_T<float>::ABGR() const lnoexcept;
		template Color& Color_T<float>::operator+=(Color const & rhs) lnoexcept;
		template Color& Color_T<float>::operator-=(Color const & rhs) lnoexcept;
		template Color& Color_T<float>::operator*=(float rhs) lnoexcept;
		template Color& Color_T<float>::operator*=(Color const & rhs) lnoexcept;
		template Color& Color_T<float>::operator/=(float rhs) lnoexcept;
		template Color& Color_T<float>::operator=(Color const & rhs) lnoexcept;
		template Color& Color_T<float>::operator=(Color&& rhs) lnoexcept;
		template Color const Color_T<float>::operator+() const lnoexcept;
		template Color const Color_T<float>::operator-() const lnoexcept;
		template bool Color_T<float>::operator==(Color const & rhs) const lnoexcept;


		template <typename T>
		Color_T<T>::Color_T(T const * rhs) lnoexcept
			: col_(rhs[0],rhs[1],rhs[2],rhs[3])
		{
		}

		template <typename T>
		Color_T<T>::Color_T(Color_T const & rhs) lnoexcept
			: col_(rhs.col_)
		{
		}

		template <typename T>
		Color_T<T>::Color_T(Color_T&& rhs) lnoexcept
			: col_(std::move(rhs.col_))
		{
		}

		template <typename T>
		Color_T<T>::Color_T(T r, T g, T b, T a) lnoexcept
		{
			this->r() = r;
			this->g() = g;
			this->b() = b;
			this->a() = a;
		}

		template <typename T>
		Color_T<T>::Color_T(uint32 dw) lnoexcept
		{
			static T const f(1 / T(255));
			this->a() = f * (static_cast<T>(static_cast<uint8>(dw >> 24)));
			this->r() = f * (static_cast<T>(static_cast<uint8>(dw >> 16)));
			this->g() = f * (static_cast<T>(static_cast<uint8>(dw >> 8)));
			this->b() = f * (static_cast<T>(static_cast<uint8>(dw >> 0)));
		}

		template <typename T>
		void Color_T<T>::RGBA(uint8& R, uint8& G, uint8& B, uint8& A) const lnoexcept
		{
			R = static_cast<uint8>(leo::math::clamp(this->r(), T(0), T(1)) * 255 + 0.5f);
			G = static_cast<uint8>(leo::math::clamp(this->g(), T(0), T(1)) * 255 + 0.5f);
			B = static_cast<uint8>(leo::math::clamp(this->b(), T(0), T(1)) * 255 + 0.5f);
			A = static_cast<uint8>(leo::math::clamp(this->a(), T(0), T(1)) * 255 + 0.5f);
		}

		template <typename T>
		uint32 Color_T<T>::ARGB() const lnoexcept
		{
			uint8 r, g, b, a;
			this->RGBA(r, g, b, a);
			return (a << 24) | (r << 16) | (g << 8) | (b << 0);
		}

		template <typename T>
		uint32 Color_T<T>::ABGR() const lnoexcept
		{
			uint8 r, g, b, a;
			this->RGBA(r, g, b, a);
			return (a << 24) | (b << 16) | (g << 8) | (r << 0);
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator+=(Color_T<T> const & rhs) lnoexcept
		{
			col_ += rhs.col_;
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator-=(Color_T<T> const & rhs) lnoexcept
		{
			col_ -= rhs.col_;
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator*=(T rhs) lnoexcept
		{
			col_ *= rhs;
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator*=(Color_T<T> const & rhs) lnoexcept
		{
			*this = modulate(*this, rhs);
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator/=(T rhs) lnoexcept
		{
			col_ /= rhs;
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator=(Color_T<T> const & rhs) lnoexcept
		{
			if (this != &rhs)
			{
				col_ = rhs.col_;
			}
			return *this;
		}

		template <typename T>
		Color_T<T>& Color_T<T>::operator=(Color_T<T>&& rhs) lnoexcept
		{
			col_ = std::move(rhs.col_);
			return *this;
		}

		template <typename T>
		Color_T<T> const Color_T<T>::operator+() const lnoexcept
		{
			return *this;
		}

		template <typename T>
		Color_T<T> const Color_T<T>::operator-() const lnoexcept
		{
			return Color_T(-this->r(), -this->g(), -this->b(), -this->a());
		}

		template <typename T>
		bool Color_T<T>::operator==(Color_T<T> const & rhs) const lnoexcept
		{
			return col_ == rhs.col_;
		}

		using namespace Render::IFormat;
		using namespace leo;
		using math::half;
		void ConvertToABGR32F(Render::EFormat fmt, void const * input, uint32_t num_elems, Color* output)
		{
			uint8_t const * p = static_cast<uint8_t const *>(input);
			uint32_t const elem_size = NumFormatBytes(fmt);

			switch (fmt)
			{
			case EF_A8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(0, 0, 0, *p / 255.0f);
				}
				break;

			case EF_R5G6B5:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(((p[1] >> 3) & 0x1F) / 31.0f, (((p[1] & 0x7) << 3) | (p[0] >> 5)) / 63.0f,
						(p[0] & 0x1F) / 31.0f, 1);
				}
				break;

			case EF_A1RGB5:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(((p[1] >> 2) & 0x1F) / 31.0f, (((p[1] & 0x3) << 3) | (p[0] >> 5)) / 31.0f,
						(p[0] & 0x1F) / 31.0f, (p[1] & 0x80) ? 1.0f : 0.0f);
				}
				break;

			case EF_ARGB4:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color((p[1] & 0xF) / 15.0f, (p[0] >> 4) / 15.0f, (p[0] & 0xF) / 15.0f, (p[1] >> 4) / 15.0f);
				}
				break;

			case EF_R8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(*p / 255.0f, 0, 0, 1);
				}
				break;

			case EF_GR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0] / 255.0f, p[1] / 255.0f, 0, 1);
				}
				break;

			case EF_SIGNED_GR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0] / 127.0f, s[1] / 127.0f, 0, 1);
				}
				break;

			case EF_BGR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, 1);
				}
				break;

			case EF_SIGNED_BGR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0] / 127.0f, s[1] / 127.0f, s[2] / 127.0f, 1);
				}
				break;

			case EF_ARGB8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[2] / 255.0f, p[1] / 255.0f, p[0] / 255.0f, p[3] / 255.0f);
				}
				break;

			case EF_ABGR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, p[3] / 255.0f);
				}
				break;

			case EF_SIGNED_ABGR8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0] / 127.0f, s[1] / 127.0f, s[2] / 127.0f, s[3] / 127.0f);
				}
				break;

			case EF_A2BGR10:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					*output = Color((s & 0x03FF) / 1023.0f, ((s >> 10) & 0x03FF) / 1023.0f,
						((s >> 20) & 0x03FF) / 1023.0f, ((s >> 30) & 0x03) / 3.0f);
				}
				break;

			case EF_SIGNED_A2BGR10:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					uint32_t r = (s >> 0) & 0x03FF;
					uint32_t g = (s >> 10) & 0x03FF;
					uint32_t b = (s >> 20) & 0x03FF;
					uint32_t a = (s >> 30) & 0x0003;
					if (r & 0x0200)
					{
						r |= 0xFFFFFC00;
					}
					if (g & 0x0200)
					{
						g |= 0xFFFFFC00;
					}
					if (b & 0x0200)
					{
						b |= 0xFFFFFC00;
					}
					if (a & 0x0002)
					{
						a |= 0xFFFFFFFC;
					}
					*output = Color(*reinterpret_cast<int32_t*>(&r) / 511.0f, *reinterpret_cast<int32_t*>(&g) / 511.0f,
						*reinterpret_cast<int32_t*>(&b) / 511.0f, *reinterpret_cast<int32_t*>(&a) / 1.0f);
				}
				break;

			case EF_R8UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(*p, 0, 0, 1);
				}
				break;

			case EF_R8I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(*s, 0, 0, 1);
				}
				break;

			case EF_GR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0], p[1], 0, 1);
				}
				break;

			case EF_GR8I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0], s[1], 0, 1);
				}
				break;

			case EF_BGR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0], p[1], p[2], 1);
				}
				break;

			case EF_BGR8I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0], s[1], s[2], 1);
				}
				break;

			case EF_ABGR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(p[0], p[1], p[2], p[3]);
				}
				break;

			case EF_ABGR8I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int8_t const * s = reinterpret_cast<int8_t const *>(p);
					*output = Color(s[0], s[1], s[2], s[3]);
				}
				break;

			case EF_A2BGR10UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s & 0x03FF), static_cast<float>((s >> 10) & 0x03FF),
						static_cast<float>((s >> 20) & 0x03FF), static_cast<float>((s >> 30) & 0x03));
				}
				break;

			case EF_A2BGR10I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					uint32_t r = (s >> 0) & 0x03FF;
					uint32_t g = (s >> 10) & 0x03FF;
					uint32_t b = (s >> 20) & 0x03FF;
					uint32_t a = (s >> 30) & 0x0003;
					if (r & 0x0200)
					{
						r |= 0xFFFFFC00;
					}
					if (g & 0x0200)
					{
						g |= 0xFFFFFC00;
					}
					if (b & 0x0200)
					{
						b |= 0xFFFFFC00;
					}
					if (a & 0x0002)
					{
						a |= 0xFFFFFFFC;
					}
					*output = Color(static_cast<float>(*reinterpret_cast<int32_t*>(&r)),
						static_cast<float>(*reinterpret_cast<int32_t*>(&g)),
						static_cast<float>(*reinterpret_cast<int32_t*>(&b)),
						static_cast<float>(*reinterpret_cast<int32_t*>(&a)));
				}
				break;

			case EF_R16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const s = *reinterpret_cast<uint16_t const *>(p);
					*output = Color(s / 65535.0f, 0, 0, 1);
				}
				break;

			case EF_SIGNED_R16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const s = *reinterpret_cast<int16_t const *>(p);
					*output = Color(s / 32767.0f, 0, 0, 1);
				}
				break;

			case EF_GR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0] / 65535.0f, s[1] / 65535.0f, 0, 1);
				}
				break;

			case EF_SIGNED_GR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0] / 32767.0f, s[1] / 32767.0f, 0, 1);
				}
				break;

			case EF_BGR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0] / 65535.0f, s[1] / 65535.0f, s[2] / 65535.0f, 1);
				}
				break;

			case EF_SIGNED_BGR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0] / 32767.0f, s[1] / 32767.0f, s[2] / 32767.0f, 1);
				}
				break;

			case EF_ABGR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0] / 65535.0f, s[1] / 65535.0f, s[2] / 65535.0f, s[3] / 65535.0f);
				}
				break;

			case EF_SIGNED_ABGR16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0] / 32767.0f, s[1] / 32767.0f, s[2] / 32767.0f, s[3] / 32767.0f);
				}
				break;

			case EF_R32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s) / 0xFFFFFFFF, 0, 0, 1);
				}
				break;

			case EF_SIGNED_R32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const s = *reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s) / 0x7FFFFFFF, 0, 0, 1);
				}
				break;

			case EF_GR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0xFFFFFFFF, static_cast<float>(s[1]) / 0xFFFFFFFF, 0, 1);
				}
				break;

			case EF_SIGNED_GR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0x7FFFFFFF, static_cast<float>(s[1]) / 0x7FFFFFFF, 0, 1);
				}
				break;

			case EF_BGR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0xFFFFFFFF, static_cast<float>(s[1]) / 0xFFFFFFFF,
						static_cast<float>(s[2]) / 0xFFFFFFFF, 1);
				}
				break;

			case EF_SIGNED_BGR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0x7FFFFFFF, static_cast<float>(s[1]) / 0x7FFFFFFF,
						static_cast<float>(s[2]) / 0x7FFFFFFF, 1);
				}
				break;

			case EF_ABGR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0xFFFFFFFF, static_cast<float>(s[1]) / 0xFFFFFFFF,
						static_cast<float>(s[2]) / 0xFFFFFFFF, static_cast<float>(s[3]) / 0xFFFFFFFF);
				}
				break;

			case EF_SIGNED_ABGR32:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]) / 0x7FFFFFFF, static_cast<float>(s[1]) / 0x7FFFFFFF,
						static_cast<float>(s[2]) / 0x7FFFFFFF, static_cast<float>(s[3]) / 0x7FFFFFFF);
				}
				break;


			case EF_R16UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const s = *reinterpret_cast<uint16_t const *>(p);
					*output = Color(s, 0, 0, 1);
				}
				break;

			case EF_R16I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const s = *reinterpret_cast<int16_t const *>(p);
					*output = Color(s, 0, 0, 1);
				}
				break;

			case EF_GR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0], s[1], 0, 1);
				}
				break;

			case EF_GR16I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0], s[1], 0, 1);
				}
				break;

			case EF_BGR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0], s[1], s[2], 1);
				}
				break;

			case EF_BGR16I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0], s[1], s[2], 1);
				}
				break;

			case EF_ABGR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const * s = reinterpret_cast<uint16_t const *>(p);
					*output = Color(s[0], s[1], s[2], s[3]);
				}
				break;

			case EF_ABGR16I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int16_t const * s = reinterpret_cast<int16_t const *>(p);
					*output = Color(s[0], s[1], s[2], s[3]);
				}
				break;

			case EF_R32UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s), 0, 0, 1);
				}
				break;

			case EF_R32I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const s = *reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s), 0, 0, 1);
				}
				break;

			case EF_GR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]), 0, 1);
				}
				break;

			case EF_GR32I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]), 0, 1);
				}
				break;

			case EF_BGR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2]), 1);
				}
				break;

			case EF_BGR32I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]), static_cast<float>(s[2]), 1);
				}
				break;

			case EF_ABGR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const * s = reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]),
						static_cast<float>(s[2]), static_cast<float>(s[3]));
				}
				break;

			case EF_ABGR32I:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					int32_t const * s = reinterpret_cast<int32_t const *>(p);
					*output = Color(static_cast<float>(s[0]), static_cast<float>(s[1]),
						static_cast<float>(s[2]), static_cast<float>(s[3]));
				}
				break;


			case EF_R16F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					half const s = *reinterpret_cast<half const *>(p);
					*output = Color(float(s), 0, 0, 1);
				}
				break;

			case EF_GR16F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					half const * s = reinterpret_cast<half const *>(p);
					*output = Color(float(s[0]), float(s[1]), 0, 1);
				}
				break;

			case EF_B10G11R11F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					// E5B5 E5G6 E5R6
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);

					union FNI
					{
						float f;
						int32_t i;
					} result[4];
					uint32_t mantissa;
					uint32_t exponent;

					for (int j = 0; j < 2; ++j)
					{
						// X & Y Channel (6-bit mantissa)
						mantissa = (s >> (j * 11)) & 0x3F;
						exponent = (s >> (j * 11 + 6)) & 0x1F;

						if (0x1F == exponent) // INF or NAN
						{
							result[0].i = 0x7F800000 | (mantissa << 17);
						}
						else
						{
							if (0 == exponent)
							{
								if (mantissa != 0)
								{
									// The value is denormalized

									// Normalize the value in the resulting float
									exponent = 1;

									do
									{
										--exponent;
										mantissa <<= 1;
									} while (0 == (mantissa & 0x40));

									mantissa &= 0x3F;
								}
								else
								{
									// The value is zero

									exponent = static_cast<uint32_t>(-112);
								}
							}

							result[j].i = ((exponent + 112) << 23) | (mantissa << 17);
						}
					}

					// Z Channel (5-bit mantissa)
					mantissa = (s >> 22) & 0x3F;
					exponent = (s >> 27) & 0x1F;

					if (0x1F == exponent) // INF or NAN
					{
						result[2].i = 0x7F800000 | (mantissa << 17);
					}
					else
					{
						if (0 == exponent)
						{
							if (mantissa != 0)
							{
								exponent = 1;

								do
								{
									--exponent;
									mantissa <<= 1;
								} while (0 == (mantissa & 0x20));

								mantissa &= 0x1F;
							}
							else
							{
								exponent = static_cast<uint32_t>(-112);
							}
						}

						result[2].i = ((exponent + 112) << 23) | (mantissa << 18);
					}

					*output = Color(result[0].f, result[1].f, result[2].f, 1);
				}
				break;

			case EF_BGR16F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					half const * s = reinterpret_cast<half const *>(p);
					*output = Color(float(s[0]), float(s[1]),float(s[2]), 1);
				}
				break;

			case EF_ABGR16F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					half const * s = reinterpret_cast<half const *>(p);
					*output = Color(float(s[0]), float(s[1]), float(s[2]), float(s[3]));
				}
				break;

			case EF_R32F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					float const s = *reinterpret_cast<float const *>(p);
					*output = Color(s, 0, 0, 1);
				}
				break;

			case EF_GR32F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					float const * s = reinterpret_cast<float const *>(p);
					*output = Color(s[0], s[1], 0, 1);
				}
				break;

			case EF_BGR32F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					float const * s = reinterpret_cast<float const *>(p);
					*output = Color(s[0], s[1], s[2], 1);
				}
				break;

			case EF_ABGR32F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					float const * s = reinterpret_cast<float const *>(p);
					*output = Color(s[0], s[1], s[2], s[3]);
				}
				break;


			case EF_D16:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint16_t const s = *reinterpret_cast<uint16_t const *>(p);
					*output = Color(s / 65535.0f, 0, 0, 0);
				}
				break;

			case EF_D24S8:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					uint32_t const s = *reinterpret_cast<uint32_t const *>(p);
					*output = Color(static_cast<float>(s & 0x00FFFFFF) / 0x00FFFFFF, 0, 0, 0);
				}
				break;

			case EF_D32F:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					float const s = *reinterpret_cast<float const *>(p);
					*output = Color(s, 0, 0, 0);
				}
				break;


			case EF_ARGB8_SRGB:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(srgb_to_linear(p[2] / 255.0f),
						srgb_to_linear(p[1] / 255.0f),
						srgb_to_linear(p[0] / 255.0f),
						srgb_to_linear(p[3] / 255.0f));
				}
				break;

			case EF_ABGR8_SRGB:
				for (uint32_t i = 0; i < num_elems; ++i, p += elem_size, ++output)
				{
					*output = Color(srgb_to_linear(p[0] / 255.0f),
						srgb_to_linear(p[1] / 255.0f),
						srgb_to_linear(p[2] / 255.0f),
						srgb_to_linear(p[3] / 255.0f));
				}
				break;

			default:
				lassume(false);
				break;
			}
		}

		void ConvertFromABGR32F(EFormat fmt, Color const * input, uint32_t num_elems, void* output)
		{
			uint8_t* p = static_cast<uint8_t*>(output);
			uint32_t const elem_size = NumFormatBytes(fmt);

			switch (fmt)
			{
			case EF_A8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					*p = static_cast<uint8_t>(math::clamp(static_cast<int>(input->a() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_R5G6B5:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int r = math::clamp(static_cast<int>(input->r() * 31.0f + 0.5f), 0, 31);
					int g = math::clamp(static_cast<int>(input->g() * 63.0f + 0.5f), 0, 63);
					int b = math::clamp(static_cast<int>(input->b() * 31.0f + 0.5f), 0, 31);
					p[0] = static_cast<uint8_t>(((g & 0x7) << 5) | b);
					p[1] = static_cast<uint8_t>((r << 3) | (g >> 3));
				}
				break;

			case EF_A1RGB5:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int r = math::clamp(static_cast<int>(input->r() * 31.0f + 0.5f), 0, 31);
					int g = math::clamp(static_cast<int>(input->g() * 31.0f + 0.5f), 0, 31);
					int b = math::clamp(static_cast<int>(input->b() * 31.0f + 0.5f), 0, 31);
					int a = (input->a() >= 0.5f) ? 1 : 0;
					p[0] = static_cast<uint8_t>(((g & 0x7) << 5) | b);
					p[1] = static_cast<uint8_t>((a << 7) | (r << 2) | (g >> 3));
				}
				break;

			case EF_ARGB4:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int r = math::clamp(static_cast<int>(input->r() * 15.0f + 0.5f), 0, 15);
					int g = math::clamp(static_cast<int>(input->g() * 15.0f + 0.5f), 0, 15);
					int b = math::clamp(static_cast<int>(input->b() * 15.0f + 0.5f), 0, 15);
					int a = math::clamp(static_cast<int>(input->a() * 15.0f + 0.5f), 0, 15);
					p[0] = static_cast<uint8_t>((g << 4) | b);
					p[1] = static_cast<uint8_t>((a << 4) | r);
				}
				break;

			case EF_R8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					*p = static_cast<uint8_t>(math::clamp(static_cast<int>(input->r() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_GR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->r() * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->g() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_SIGNED_GR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<int8_t>(math::clamp(static_cast<int>(input->r() * 127.0f + 0.5f), -127, 127));
					p[1] = static_cast<int8_t>(math::clamp(static_cast<int>(input->g() * 127.0f + 0.5f), -127, 127));
				}
				break;

			case EF_BGR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->r() * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->g() * 255.0f + 0.5f), 0, 255));
					p[2] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->b() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_SIGNED_BGR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<int8_t>(math::clamp(static_cast<int>(input->r() * 127.0f + 0.5f), -127, 127));
					p[1] = static_cast<int8_t>(math::clamp(static_cast<int>(input->g() * 127.0f + 0.5f), -127, 127));
					p[2] = static_cast<int8_t>(math::clamp(static_cast<int>(input->b() * 127.0f + 0.5f), -127, 127));
				}
				break;

			case EF_ARGB8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->b() * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->g() * 255.0f + 0.5f), 0, 255));
					p[2] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->r() * 255.0f + 0.5f), 0, 255));
					p[3] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->a() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_ABGR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->r() * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->g() * 255.0f + 0.5f), 0, 255));
					p[2] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->b() * 255.0f + 0.5f), 0, 255));
					p[3] = static_cast<uint8_t>(math::clamp(static_cast<int>(input->a() * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_SIGNED_ABGR8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<int8_t>(math::clamp(static_cast<int>(input->r() * 127.0f + 0.5f), -127, 127));
					p[1] = static_cast<int8_t>(math::clamp(static_cast<int>(input->g() * 127.0f + 0.5f), -127, 127));
					p[2] = static_cast<int8_t>(math::clamp(static_cast<int>(input->b() * 127.0f + 0.5f), -127, 127));
					p[3] = static_cast<int8_t>(math::clamp(static_cast<int>(input->a() * 127.0f + 0.5f), -127, 127));
				}
				break;

			case EF_A2BGR10:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int r = math::clamp(static_cast<int>(input->r() * 1023.0f + 0.5f), 0, 1023);
					int g = math::clamp(static_cast<int>(input->g() * 1023.0f + 0.5f), 0, 1023);
					int b = math::clamp(static_cast<int>(input->b() * 1023.0f + 0.5f), 0, 1023);
					int a = math::clamp(static_cast<int>(input->a() * 3.0f + 0.5f), 0, 3);

					*reinterpret_cast<uint32_t*>(p) = r | (g << 10) | (b << 20) | (a << 30);
				}
				break;

			case EF_SIGNED_A2BGR10:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t r = static_cast<uint32_t>(math::clamp(static_cast<int>(input->r() * 511.0f + 0.5f), -511, 511));
					uint32_t g = static_cast<uint32_t>(math::clamp(static_cast<int>(input->g() * 511.0f + 0.5f), -511, 511));
					uint32_t b = static_cast<uint32_t>(math::clamp(static_cast<int>(input->b() * 511.0f + 0.5f), -511, 511));
					uint32_t a = static_cast<uint32_t>(math::clamp(static_cast<int>(input->a() * 1.0f + 0.5f), -1, 1));

					*reinterpret_cast<uint32_t*>(p) = (r & 0x03FF) | ((g & 0x03FF) << 10) | ((b & 0x03FF) << 20) | ((a & 0x0003) << 30);
				}
				break;

			case EF_R8UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					*p = static_cast<uint8_t>(input->r());
				}
				break;

			case EF_R8I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int8_t* s = reinterpret_cast<int8_t*>(p);
					*s = static_cast<int8_t>(input->r());
				}
				break;

			case EF_GR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(input->r());
					p[1] = static_cast<uint8_t>(input->g());
				}
				break;

			case EF_GR8I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int8_t* s = reinterpret_cast<int8_t*>(p);
					s[0] = static_cast<int8_t>(input->r());
					s[1] = static_cast<int8_t>(input->g());
				}
				break;

			case EF_BGR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(input->r());
					p[1] = static_cast<uint8_t>(input->g());
					p[2] = static_cast<uint8_t>(input->b());
				}
				break;

			case EF_BGR8I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int8_t* s = reinterpret_cast<int8_t*>(p);
					s[0] = static_cast<int8_t>(input->r());
					s[1] = static_cast<int8_t>(input->g());
					s[2] = static_cast<int8_t>(input->b());
				}
				break;

			case EF_ABGR8UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(input->r());
					p[1] = static_cast<uint8_t>(input->g());
					p[2] = static_cast<uint8_t>(input->b());
					p[3] = static_cast<uint8_t>(input->a());
				}
				break;

			case EF_ABGR8I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int8_t* s = reinterpret_cast<int8_t*>(p);
					s[0] = static_cast<int8_t>(input->r());
					s[1] = static_cast<int8_t>(input->g());
					s[2] = static_cast<int8_t>(input->b());
					s[3] = static_cast<int8_t>(input->a());
				}
				break;

			case EF_A2BGR10UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t r = static_cast<uint32_t>(input->r());
					uint32_t g = static_cast<uint32_t>(input->g());
					uint32_t b = static_cast<uint32_t>(input->b());
					uint32_t a = static_cast<uint32_t>(input->a());
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = (r & 0x03FF) | ((g & 0x03FF) << 10) | ((b & 0x03FF) << 20) | ((a & 0x03) << 30);
				}
				break;

			case EF_A2BGR10I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t r = static_cast<int32_t>(input->r());
					int32_t g = static_cast<int32_t>(input->g());
					int32_t b = static_cast<int32_t>(input->b());
					int32_t a = static_cast<int32_t>(input->a());
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = (static_cast<uint32_t>(r) & 0x03FF) | ((static_cast<uint32_t>(g) & 0x03FF) << 10)
						| ((static_cast<uint32_t>(b) & 0x03FF) << 20) | ((static_cast<uint32_t>(a) & 0x03) << 30);
				}
				break;

			case EF_R16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					*s = static_cast<uint16_t>(math::clamp(static_cast<int>(input->r() * 65535.0f + 0.5f), 0, 65535));
				}
				break;

			case EF_SIGNED_R16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					*s = static_cast<int16_t>(math::clamp(static_cast<int>(input->r() * 32767.0f + 0.5f), -32767, 32767));
				}
				break;

			case EF_GR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->r() * 65535.0f + 0.5f), 0, 65535));
					s[1] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->g() * 65535.0f + 0.5f), 0, 65535));
				}
				break;

			case EF_SIGNED_GR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(math::clamp(static_cast<int>(input->r() * 32767.0f + 0.5f), -32767, 32767));
					s[1] = static_cast<int16_t>(math::clamp(static_cast<int>(input->g() * 32767.0f + 0.5f), -32767, 32767));
				}
				break;

			case EF_BGR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->r() * 65535.0f + 0.5f), 0, 65535));
					s[1] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->g() * 65535.0f + 0.5f), 0, 65535));
					s[2] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->b() * 65535.0f + 0.5f), 0, 65535));
				}
				break;

			case EF_SIGNED_BGR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(math::clamp(static_cast<int>(input->r() * 32767.0f + 0.5f), -32767, 32767));
					s[1] = static_cast<int16_t>(math::clamp(static_cast<int>(input->g() * 32767.0f + 0.5f), -32767, 32767));
					s[2] = static_cast<int16_t>(math::clamp(static_cast<int>(input->b() * 32767.0f + 0.5f), -32767, 32767));
				}
				break;

			case EF_ABGR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->r() * 65535.0f + 0.5f), 0, 65535));
					s[1] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->g() * 65535.0f + 0.5f), 0, 65535));
					s[2] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->b() * 65535.0f + 0.5f), 0, 65535));
					s[3] = static_cast<uint16_t>(math::clamp(static_cast<int>(input->a() * 65535.0f + 0.5f), 0, 65535));
				}
				break;

			case EF_SIGNED_ABGR16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(math::clamp(static_cast<int>(input->r() * 32767.0f + 0.5f), -32767, 32767));
					s[1] = static_cast<int16_t>(math::clamp(static_cast<int>(input->g() * 32767.0f + 0.5f), -32767, 32767));
					s[2] = static_cast<int16_t>(math::clamp(static_cast<int>(input->b() * 32767.0f + 0.5f), -32767, 32767));
					s[3] = static_cast<int16_t>(math::clamp(static_cast<int>(input->a() * 32767.0f + 0.5f), -32767, 32767));
				}
				break;

			case EF_R32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->r() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
				}
				break;

			case EF_SIGNED_R32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					*s = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->r() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
				}
				break;

			case EF_GR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->r() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[1] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->g() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
				}
				break;

			case EF_SIGNED_GR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->r() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[1] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->g() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
				}
				break;

			case EF_BGR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->r() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[1] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->g() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[2] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->b() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
				}
				break;

			case EF_SIGNED_BGR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->r() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[1] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->g() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[2] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->b() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
				}
				break;

			case EF_ABGR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->r() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[1] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->g() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[2] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->b() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
					s[3] = static_cast<uint32_t>(math::clamp<uint64_t>(static_cast<uint64_t>(input->a() * 0xFFFFFFFFULL + 0.5f), 0ULL, 0xFFFFFFFFULL));
				}
				break;

			case EF_SIGNED_ABGR32:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->r() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[1] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->g() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[2] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->b() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
					s[3] = static_cast<int32_t>(math::clamp<int64_t>(static_cast<int64_t>(input->a() * 0x7FFFFFFFLL + 0.5f), -0x7FFFFFFFLL, 0x7FFFFFFFLL));
				}
				break;


			case EF_R16UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					*s = static_cast<uint16_t>(input->r());
				}
				break;

			case EF_R16I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					*s = static_cast<int16_t>(input->r());
				}
				break;

			case EF_GR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(input->r());
					s[1] = static_cast<uint16_t>(input->g());
				}
				break;

			case EF_GR16I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(input->r());
					s[1] = static_cast<int16_t>(input->g());
				}
				break;

			case EF_BGR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(input->r());
					s[1] = static_cast<uint16_t>(input->g());
					s[2] = static_cast<uint16_t>(input->b());
				}
				break;

			case EF_BGR16I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(input->r());
					s[1] = static_cast<int16_t>(input->g());
					s[2] = static_cast<int16_t>(input->b());
				}
				break;

			case EF_ABGR16UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					s[0] = static_cast<uint16_t>(input->r());
					s[1] = static_cast<uint16_t>(input->g());
					s[2] = static_cast<uint16_t>(input->b());
					s[3] = static_cast<uint16_t>(input->a());
				}
				break;

			case EF_ABGR16I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int16_t* s = reinterpret_cast<int16_t*>(p);
					s[0] = static_cast<int16_t>(input->r());
					s[1] = static_cast<int16_t>(input->g());
					s[2] = static_cast<int16_t>(input->b());
					s[3] = static_cast<int16_t>(input->a());
				}
				break;

			case EF_R32UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = static_cast<uint32_t>(input->r());
				}
				break;

			case EF_R32I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					*s = static_cast<int32_t>(input->r());
				}
				break;

			case EF_GR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(input->r());
					s[1] = static_cast<uint32_t>(input->g());
				}
				break;

			case EF_GR32I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(input->r());
					s[1] = static_cast<int32_t>(input->g());
				}
				break;

			case EF_BGR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(input->r());
					s[1] = static_cast<uint32_t>(input->g());
					s[2] = static_cast<uint32_t>(input->b());
				}
				break;

			case EF_BGR32I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(input->r());
					s[1] = static_cast<int32_t>(input->g());
					s[2] = static_cast<int32_t>(input->b());
				}
				break;

			case EF_ABGR32UI:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					s[0] = static_cast<uint32_t>(input->r());
					s[1] = static_cast<uint32_t>(input->g());
					s[2] = static_cast<uint32_t>(input->b());
					s[3] = static_cast<uint32_t>(input->a());
				}
				break;

			case EF_ABGR32I:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					int32_t* s = reinterpret_cast<int32_t*>(p);
					s[0] = static_cast<int32_t>(input->r());
					s[1] = static_cast<int32_t>(input->g());
					s[2] = static_cast<int32_t>(input->b());
					s[3] = static_cast<int32_t>(input->a());
				}
				break;


			case EF_R16F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					half* s = reinterpret_cast<half*>(p);
					*s = half(input->r());
				}
				break;

			case EF_GR16F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					half* s = reinterpret_cast<half*>(p);
					s[0] = half(input->r());
					s[1] = half(input->g());
				}
				break;

			case EF_B10G11R11F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					// E5B5 E5G6 E5R6
					uint32_t result[3];

					union FNI
					{
						float f;
						int32_t i;
					} ivalue[4];
					ivalue[0].f = input->r();
					ivalue[1].f = input->g();
					ivalue[2].f = input->b();

					// X & Y Channels (5-bit exponent, 6-bit mantissa)
					for (int j = 0; j < 2; ++j)
					{
						uint32_t sign = ivalue[j].i & 0x80000000;
						uint32_t ip = ivalue[j].i & 0x7FFFFFFF;

						if (0x7F800000 == (ip & 0x7F800000))
						{
							// INF or NAN
							result[j] = 0x07C0;
							if ((ip & 0x007FFFFF) != 0)
							{
								result[j] = 0x07C0 | (((ip >> 17) | (ip > 11) | (ip >> 6) | ip) & 0x3F);
							}
							else if (sign)
							{
								// -INF is clamped to 0 since 3PK is positive only
								result[j] = 0;
							}
						}
						else if (sign)
						{
							// 3PK is positive only, so clamp to zero
							result[j] = 0;
						}
						else if (ip > 0x477E0000)
						{
							// The number is too large to be represented as a float11, set to max
							result[j] = 0x07BF;
						}
						else
						{
							if (ip < 0x38800000)
							{
								// The number is too small to be represented as a normalized float11
								// Convert it to a denormalized value.
								uint32_t shift = 113 - (ip >> 23);
								ip = (0x800000U | (ip & 0x7FFFFF)) >> shift;
							}
							else
							{
								// Rebias the exponent to represent the value as a normalized float11
								ip += 0xC8000000;
							}

							result[j] = ((ip + 0xFFFF + ((ip >> 17) & 1)) >> 17) & 0x07FF;
						}
					}

					// Z Channel (5-bit exponent, 5-bit mantissa)
					uint32_t sign = ivalue[2].i & 0x80000000;
					uint32_t ip = ivalue[2].i & 0x7FFFFFFF;

					if (0x7F800000 == (ip & 0x7F800000))
					{
						// INF or NAN
						result[2] = 0x03E0;
						if (ip & 0x007FFFFF)
						{
							result[2] = 0x03E0 | (((ip >> 18) | (ip > 13) | (ip >> 3) | i) & 0x1F);
						}
						else if (sign)
						{
							// -INF is clamped to 0 since 3PK is positive only
							result[2] = 0;
						}
					}
					else if (sign)
					{
						// 3PK is positive only, so clamp to zero
						result[2] = 0;
					}
					else if (ip > 0x477C0000)
					{
						// The number is too large to be represented as a float10, set to max
						result[2] = 0x03DF;
					}
					else
					{
						if (ip < 0x38800000)
						{
							// The number is too small to be represented as a normalized float10
							// Convert it to a denormalized value.
							uint32_t shift = 113 - (ip >> 23);
							ip = (0x800000 | (ip & 0x7FFFFF)) >> shift;
						}
						else
						{
							// Rebias the exponent to represent the value as a normalized float10
							ip += 0xC8000000;
						}

						result[2] = ((ip + 0x01FFFF + ((ip >> 18) & 1)) >> 18) & 0x03FF;
					}

					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = (result[0] & 0x07FF)
						| ((result[1] & 0x07FF) << 11)
						| ((result[2] & 0x03FF) << 22);
				}
				break;

			case EF_BGR16F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					half* s = reinterpret_cast<half*>(p);
					s[0] = half(input->r());
					s[1] = half(input->g());
					s[2] = half(input->b());
				}
				break;

			case EF_ABGR16F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					half* s = reinterpret_cast<half*>(p);
					s[0] = half(input->r());
					s[1] = half(input->g());
					s[2] = half(input->b());
					s[3] = half(input->a());
				}
				break;

			case EF_R32F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					float* s = reinterpret_cast<float*>(p);
					*s = input->r();
				}
				break;

			case EF_GR32F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					float* s = reinterpret_cast<float*>(p);
					s[0] = input->r();
					s[1] = input->g();
				}
				break;

			case EF_BGR32F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					float* s = reinterpret_cast<float*>(p);
					s[0] = input->r();
					s[1] = input->g();
					s[2] = input->b();
				}
				break;

			case EF_ABGR32F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					float* s = reinterpret_cast<float*>(p);
					s[0] = input->r();
					s[1] = input->g();
					s[2] = input->b();
					s[3] = input->a();
				}
				break;


			case EF_D16:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint16_t* s = reinterpret_cast<uint16_t*>(p);
					*s = static_cast<uint16_t>(math::clamp(static_cast<int>(input->r() * 65535.0f + 0.5f), 0, 65535));
				}
				break;

			case EF_D24S8:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					uint32_t* s = reinterpret_cast<uint32_t*>(p);
					*s = static_cast<uint32_t>(math::clamp(static_cast<int>(input->r() * 0x00FFFFFF + 0.5f), 0, 0x00FFFFFF));
				}
				break;

			case EF_D32F:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					float* s = reinterpret_cast<float*>(p);
					*s = input->r();
				}
				break;


			case EF_ARGB8_SRGB:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->b()) * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->g()) * 255.0f + 0.5f), 0, 255));
					p[2] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->r()) * 255.0f + 0.5f), 0, 255));
					p[3] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->a()) * 255.0f + 0.5f), 0, 255));
				}
				break;

			case EF_ABGR8_SRGB:
				for (uint32_t i = 0; i < num_elems; ++i, ++input, p += elem_size)
				{
					p[0] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->r()) * 255.0f + 0.5f), 0, 255));
					p[1] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->g()) * 255.0f + 0.5f), 0, 255));
					p[2] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->b()) * 255.0f + 0.5f), 0, 255));
					p[3] = static_cast<uint8_t>(math::clamp(static_cast<int>(linear_to_srgb(input->a()) * 255.0f + 0.5f), 0, 255));
				}
				break;

			default:
				lassume(false);
				break;
			}
		}
}