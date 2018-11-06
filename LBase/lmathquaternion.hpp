#ifndef LBase_LMATHQUATERNION_HPP
#define LBase_LMATHQUATERNION_HPP 1

#include <LBase/type_traits.hpp>
#include <LBase/operators.hpp>
#include <LBase/lmathtype.hpp>

namespace leo::math {
	template<typename scalar>
	struct lalignas(16) basic_quaternion final :private
		addable<basic_quaternion<scalar>>,
		subtractable<basic_quaternion<scalar>>,
		dividable<basic_quaternion<scalar>, scalar>,
		multipliable<basic_quaternion<scalar>>,
		multipliable<basic_quaternion<scalar>, scalar>,
		equality_comparable<basic_quaternion<scalar>>
	{
		static_assert(is_floating_point<scalar>::value);

		constexpr size_t size() const {
			return 4;
		}

		constexpr basic_quaternion() noexcept = default;

		constexpr basic_quaternion(scalar x, scalar y, scalar z, scalar w)
			:x(x),y(y),z(z),w(w)
		{}

		basic_quaternion& operator=(const basic_quaternion&) noexcept = default;
		basic_quaternion& operator=(basic_quaternion&&) noexcept = default;

		const static basic_quaternion identity;

		constexpr const scalar_type* begin() const noexcept {
			return data + 0;
		}

		constexpr const scalar_type* end() const noexcept {
			return data + size();
		}

		scalar_type* begin() noexcept {
			return data + 0;
		}

		scalar_type* end() noexcept {
			return data + size();
		}

		constexpr const scalar_type& operator[](std::size_t index) const noexcept {
			lassume(index < size());
			return data[index];
		}

		scalar_type& operator[](std::size_t index) noexcept {
			lassume(index < size());
			return data[index];
		}

		basic_quaternion& operator+=(basic_quaternion rhs) noexcept;
		basic_quaternion& operator-=(basic_quaternion rhs) noexcept;

		basic_quaternion& operator*=(basic_quaternion rhs) noexcept;
		basic_quaternion& operator*=(scalar_type rhs) noexcept;
		basic_quaternion& operator/=(scalar_type rhs) noexcept;

		basic_quaternion operator+() const noexcept;
		basic_quaternion operator-() const noexcept;

		bool operator==(const basic_quaternion& rhs) const noexcept;

		union {
			struct {
				scalar x, y, z, w;
			};
			scalar data[4];
		};
	};


	/* !\brief make a Quaternion of which the sign of the scalar element encodes the Reflection
	*/
	template<typename scalar>
	basic_quaternion< scalar> make_qtangent(const vector3<scalar>& normal, scalar signw) {
		return  basic_quaternion<scalar>{ normal.x, normal.y, normal.z, signw };
	}

	template<typename scalar>
	const basic_quaternion<scalar> basic_quaternion<scalar>::identity{ 0,0,0,1 }

	using quaternion = basic_quaternion<float>;
}

#endif