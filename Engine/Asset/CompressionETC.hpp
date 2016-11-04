/*! \file Engine\Asset\CompressionETC.hpp
\ingroup Engine
\brief ETC Compression IO ...
*/
#ifndef LE_ASSET_COMPRESSIONETC_HPP
#define LE_ASSET_COMPRESSIONETC_HPP 1

#include <LBase/linttype.hpp>
#include "TexCompression.hpp"
#include "../emacro.h"


namespace etc {
	using namespace leo::inttype;
	using namespace tc;

	struct ETC1Block
	{
		uint8 r;
		uint8 g;
		uint8 b;
		uint8 cw_diff_flip;
		uint16 msb;
		uint16 lsb;
	};

	struct ETC2TModeBlock
	{
		uint8 r1;
		uint8 g1_b1;
		uint8 r2_g2;
		uint8 b2_d;
		uint16 msb;
		uint16 lsb;
	};

	struct ETC2HModeBlock
	{
		uint8 r1_g1;
		uint8 g1_b1;
		uint8 b1_r2_g2;
		uint8 g2_b2_d;
		uint16 msb;
		uint16 lsb;
	};

	struct ETC2PlanarModeBlock
	{
		uint8 ro_go;
		uint8 go_bo;
		uint8 bo;
		uint8 bo_rh;
		uint8 gh_bh;
		uint8 bh_rv;
		uint8 rv_gv;
		uint8 gv_bv;
	};

	union ETC2Block
	{
		ETC1Block etc1;
		ETC2TModeBlock etc2_t_mode;
		ETC2HModeBlock etc2_h_mode;
		ETC2PlanarModeBlock etc2_planar_mode;
	};

	class LE_API TexCompressionETC1 : public TexCompression
	{
	public:
		struct Params
		{
			Params();

			TexCompressionMethod quality_;

			uint32 num_src_pixels_;
			ARGBColor32 const * src_pixels_;

			bool use_color4_;
			int const * scan_deltas_;
			uint32 scan_delta_size_;

			ARGBColor32 base_color5_;
			bool constrain_against_base_color5_;
		};

		struct Results
		{
			Results();

			Results& operator=(Results const & rhs);

			uint64 error_;
			ARGBColor32 block_color_unscaled_;
			uint32 block_inten_table_;
			std::vector<uint8> selectors_;
			bool block_color4_;
		};

	public:
		TexCompressionETC1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		uint64 EncodeETC1BlockInternal(ETC1Block& output, ARGBColor32 const * argb, TexCompressionMethod method);
		void DecodeETCIndividualModeInternal(ARGBColor32* argb, ETC1Block const & etc1) const;
		void DecodeETCDifferentialModeInternal(ARGBColor32* argb, ETC1Block const & etc1, bool alpha) const;

		static int GetModifier(int cw, int selector);

	private:
		struct ETC1SolutionCoordinates
		{
			ETC1SolutionCoordinates();
			ETC1SolutionCoordinates(int r, int g, int b, uint32 inten_table, bool color4);
			ETC1SolutionCoordinates(ARGBColor32 const & c, uint32 inten_table, bool color4);
			ETC1SolutionCoordinates(ETC1SolutionCoordinates const & rhs);

			ETC1SolutionCoordinates& operator=(ETC1SolutionCoordinates const & rhs);

			void Clear();

			void ScaledColor(uint8& br, uint8& bg, uint8& bb) const;
			ARGBColor32 ScaledColor() const;
			void BlockColors(ARGBColor32* block_colors) const;

			ARGBColor32 unscaled_color_;
			uint32 inten_table_;
			bool color4_;
		};

		struct PotentialSolution
		{
			PotentialSolution();

			void Clear();

			ETC1SolutionCoordinates coords_;
			uint8 selectors_[8];
			uint64 error_;
			bool valid_;
		};

	private:
		uint32 ETC1DecodeValue(uint32 diff, uint32 inten, uint32 selector, uint32 packed_c) const;

		uint64 PackETC1UniformBlock(ETC1Block& block, ARGBColor32 const * argb) const;
		uint32 PackETC1UniformPartition(Results& results, uint32 num_colors, ARGBColor32 const * argb,
			bool use_diff, ARGBColor32 const * base_color5_unscaled) const;

		void InitSolver(Params const & params, Results& result);
		bool Solve();
		bool EvaluateSolution(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution);
		bool EvaluateSolutionFast(ETC1SolutionCoordinates const & coords, PotentialSolution& trial_solution, PotentialSolution& best_solution);

	private:
		static uint8 quant_5_tab_[256 + 16];
		static uint16 etc1_inverse_lookup_[2 * 8 * 4][256];
		static bool lut_inited_;

		Params const * params_;
		Results* result_;

		int limit_;

		float3 avg_color_;
		int br_, bg_, bb_;
		uint16 luma_[8];
		uint32 sorted_luma_[2][8];
		uint32 const * sorted_luma_indices_;
		uint32* sorted_luma_ptr_;

		PotentialSolution best_solution_;
		PotentialSolution trial_solution_;
		uint8 temp_selectors_[8];
	};
	using TexCompressionETC1Ptr = std::shared_ptr<TexCompressionETC1>;

	class LE_API TexCompressionETC2RGB8 : public TexCompression
	{
	public:
		TexCompressionETC2RGB8();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void DecodeETCTModeInternal(ARGBColor32* argb, ETC2TModeBlock const & etc2, bool alpha);
		void DecodeETCHModeInternal(ARGBColor32* argb, ETC2HModeBlock const & etc2, bool alpha);
		void DecodeETCPlanarModeInternal(ARGBColor32* argb, ETC2PlanarModeBlock const & etc2);

	private:
		TexCompressionETC1Ptr etc1_codec_;
	};
	using TexCompressionETC2RGB8Ptr = std::shared_ptr<TexCompressionETC2RGB8>;

	class LE_API TexCompressionETC2RGB8A1 : public TexCompression
	{
	public:
		TexCompressionETC2RGB8A1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionETC1Ptr etc1_codec_;
		TexCompressionETC2RGB8Ptr etc2_rgb8_codec_;
	};

	template <typename T>
	inline T
		sqr(T const & x) lnoexcept
	{
		return x * x;
	}

}

#endif