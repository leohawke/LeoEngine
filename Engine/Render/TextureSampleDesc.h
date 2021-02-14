#pragma once

#include <LBase/linttype.hpp>
#include <LBase/id.hpp>
#include <LBase/string_utility.hpp>
#include <LBase/exception_type.h>
#include "Color_T.hpp"
#include <compare>

namespace platform::Render
{
	enum class TexAddressingMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border
	};

	namespace details
	{
		enum TexFilterOp {
			Mip_Point = 0x0,
			Mip_Linear = 0x1,
			Mag_Point = 0x0,
			Mag_Linear = 0x2,
			Min_Point = 0x0,
			Min_Linear = 0x4,
			Anisotropic = 0x08,
			Comparison = 0x10,
		};
	}

	enum class TexFilterOp
	{
		Min_Mag_Mip_Point = details::Min_Point | details::Mag_Point | details::Mip_Point,
		Min_Mag_Point_Mip_Linear = details::Min_Point | details::Mag_Point | details::Mip_Linear,
		Min_Point_Mag_Linear_Mip_Point = details::Min_Point | details::Mag_Linear | details::Mip_Point,
		Min_Point_Mag_Mip_Linear = details::Min_Point | details::Mag_Linear | details::Mip_Linear,
		Min_Linear_Mag_Mip_Point = details::Min_Linear | details::Mag_Point | details::Mip_Point,
		Min_Linear_Mag_Point_Mip_Linear = details::Min_Linear | details::Mag_Point | details::Mip_Linear,
		Min_Mag_Linear_Mip_Point = details::Min_Linear | details::Mag_Linear | details::Mip_Point,
		Min_Mag_Mip_Linear = details::Min_Linear | details::Mag_Linear | details::Mip_Linear,
		Anisotropic = details::Anisotropic,

		Cmp_Min_Mag_Mip_Point = details::Comparison | Min_Mag_Mip_Point,
		Cmp_Min_Mag_Point_Mip_Linear = details::Comparison | Min_Mag_Point_Mip_Linear,
		Cmp_Min_Point_Mag_Linear_Mip_Point = details::Comparison | Min_Point_Mag_Linear_Mip_Point,
		Cmp_Min_Point_Mag_Mip_Linear = details::Comparison | Min_Point_Mag_Mip_Linear,
		Cmp_Min_Linear_Mag_Mip_Point = details::Comparison | Min_Linear_Mag_Mip_Point,
		Cmp_Min_Linear_Mag_Point_Mip_Linear = details::Comparison | Min_Linear_Mag_Point_Mip_Linear,
		Cmp_Min_Mag_Linear_Mip_Point = details::Comparison | Min_Mag_Linear_Mip_Point,
		Cmp_Min_Mag_Mip_Linear = details::Comparison | Min_Mag_Mip_Linear,
		Cmp_Anisotropic = details::Comparison | Anisotropic
	};

	enum class CompareOp
	{
		Fail,
		Pass,
		Less,
		Less_Equal,
		Equal,
		NotEqual,
		GreaterEqual,
		Greater
	};

	struct TextureSampleDesc
	{
		M::Color border_clr;

		TexAddressingMode address_mode_u;
		TexAddressingMode address_mode_v;
		TexAddressingMode address_mode_w;

		TexFilterOp filtering;

		leo::uint8 max_anisotropy;
		float min_lod;
		float max_lod;
		float mip_map_lod_bias;

		CompareOp cmp_func;

		TextureSampleDesc();

		friend auto operator<=>(const TextureSampleDesc& lhs, const TextureSampleDesc& rhs) = default;
		friend bool operator==(const TextureSampleDesc& lhs, const TextureSampleDesc& rhs) = default;

		template<typename T>
		static T to_op(const std::string& value);



		static TexAddressingMode to_mode(const std::string& value) {
			auto lower_value = value;
			leo::to_lower(lower_value);
			auto hash = leo::constfn_hash(lower_value.c_str());
			switch (hash) {
			case leo::constfn_hash("wrap"):
				return TexAddressingMode::Wrap;
			case leo::constfn_hash("mirror"):
				return TexAddressingMode::Mirror;
			case leo::constfn_hash("clamp"):
				return TexAddressingMode::Clamp;
			case leo::constfn_hash("border"):
				return TexAddressingMode::Border;
			}
			throw leo::narrowing_error();
		}


	};

	template<>
	inline CompareOp TextureSampleDesc::to_op<CompareOp>(const std::string& value) {
		auto lower_value = value;
		leo::to_lower(lower_value);
		auto hash = leo::constfn_hash(lower_value.c_str());
		switch (hash) {
		case leo::constfn_hash("fail"):
			return CompareOp::Fail;
		case leo::constfn_hash("pass"):
			return CompareOp::Pass;
		case leo::constfn_hash("less"):
			return CompareOp::Less;
		case leo::constfn_hash("less_equal"):
			return CompareOp::Less_Equal;
		case leo::constfn_hash("equal"):
			return CompareOp::Equal;
		case leo::constfn_hash("notequal"):
			return CompareOp::NotEqual;
		case leo::constfn_hash("greaterequal"):
			return CompareOp::GreaterEqual;
		case leo::constfn_hash("greater"):
			return CompareOp::Greater;
		}

		throw leo::narrowing_error();
	}

	template<>
	inline TexFilterOp TextureSampleDesc::to_op<TexFilterOp>(const std::string& value) {
		auto lower_value = value;
		leo::to_lower(lower_value);
		auto hash = leo::constfn_hash(lower_value.c_str());
		switch (hash) {
		case leo::constfn_hash("min_mag_mip_point"):
			return TexFilterOp::Min_Mag_Mip_Point;
		case leo::constfn_hash("min_mag_point_mip_linear"):
			return TexFilterOp::Min_Mag_Point_Mip_Linear;
		case leo::constfn_hash("min_point_mag_linear_mip_point"):
			return TexFilterOp::Min_Point_Mag_Linear_Mip_Point;
		case leo::constfn_hash("min_point_mag_mip_linear"):
			return TexFilterOp::Min_Point_Mag_Mip_Linear;
		case leo::constfn_hash("min_linear_mag_mip_point"):
			return TexFilterOp::Min_Linear_Mag_Mip_Point;
		case leo::constfn_hash("min_linear_mag_point_mip_linear"):
			return TexFilterOp::Min_Linear_Mag_Point_Mip_Linear;
		case leo::constfn_hash("min_mag_linear_mip_point"):
			return TexFilterOp::Min_Mag_Linear_Mip_Point;
		case leo::constfn_hash("min_mag_mip_linear"):
			return TexFilterOp::Min_Mag_Mip_Linear;
		case leo::constfn_hash("anisotropic"):
			return TexFilterOp::Anisotropic;
		case leo::constfn_hash("cmp_min_mag_mip_point"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_mag_point_mip_linear"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_point_mag_linear_mip_point"):
			return TexFilterOp::Cmp_Min_Point_Mag_Linear_Mip_Point;
		case leo::constfn_hash("cmp_min_point_mag_mip_linear"):
			return TexFilterOp::Cmp_Min_Point_Mag_Mip_Linear;
		case leo::constfn_hash("cmp_min_linear_mag_mip_point"):
			return TexFilterOp::Cmp_Min_Linear_Mag_Mip_Point;
		case leo::constfn_hash("cmp_min_linear_mag_point_mip_linear"):
			return TexFilterOp::Cmp_Min_Linear_Mag_Point_Mip_Linear;
		case leo::constfn_hash("cmp_min_mag_linear_mip_point"):
			return TexFilterOp::Cmp_Min_Mag_Linear_Mip_Point;
		case leo::constfn_hash("cmp_min_mag_mip_linear"):
			return TexFilterOp::Cmp_Min_Mag_Mip_Linear;
		case leo::constfn_hash("cmp_anisotropic"):
			return TexFilterOp::Cmp_Anisotropic;
		}
		throw leo::narrowing_error();
	}
}