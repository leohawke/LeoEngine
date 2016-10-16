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
			: col_(rhs)
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


}