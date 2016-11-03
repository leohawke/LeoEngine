/*! \file Engine\Asset\CompressionBC.hpp
\ingroup Engine
\brief BC Compression IO ...
*/
#ifndef LE_ASSET_COMPRESSIONBC_HPP
#define LE_ASSET_COMPRESSIONBC_HPP 1

#include <LBase/linttype.hpp>
#include "TexCompression.hpp"
#include "../emacro.h"


namespace bc {
	using namespace leo::inttype;
	using namespace tc;

	struct BC1Block
	{
		uint16 clr_0, clr_1;
		uint16 bitmap[2];
	};

	struct BC2Block
	{
		uint16 alpha[4];
		BC1Block bc1;
	};

	struct BC4Block
	{
		uint8 alpha_0, alpha_1;
		uint8 bitmap[6];
	};

	struct BC3Block
	{
		BC4Block alpha;
		BC1Block bc1;
	};

	struct BC5Block
	{
		BC4Block red;
		BC4Block green;
	};

	class LE_API TexCompressionBC1 : public TexCompression
	{
	public:
		TexCompressionBC1();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void EncodeBC1Internal(BC1Block& bc1, ARGBColor32 const * argb, bool alpha, TexCompressionMethod method) const;

	private:
		void PrepareOptTable(uint8* table, uint8 const * expand, int size) const;
		ARGBColor32 RGB565To888(uint16_t rgb) const;
		uint16_t RGB888To565(ARGBColor32 const & rgb) const;
		uint32 MatchColorsBlock(ARGBColor32 const * argb, ARGBColor32 const & min_clr, ARGBColor32 const & max_clr, bool alpha) const;
		void OptimizeColorsBlock(ARGBColor32 const * argb, ARGBColor32& min_clr, ARGBColor32& max_clr, TexCompressionMethod method) const;
		bool RefineBlock(ARGBColor32 const * argb, ARGBColor32& min_clr, ARGBColor32& max_clr, uint32 mask) const;

	private:
		static uint8 expand5_[32];
		static uint8 expand6_[64];
		static uint8 o_match5_[256][2];
		static uint8 o_match6_[256][2];
		static uint8 quant_rb_tab_[256 + 16];
		static uint8 quant_g_tab_[256 + 16];
		static bool lut_inited_;
	};
	using TexCompressionBC1Ptr = std::shared_ptr<TexCompressionBC1>;

	class LE_API TexCompressionBC2 : public TexCompression
	{
	public:
		TexCompressionBC2();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC1Ptr bc1_codec_;
	};

	class TexCompressionBC4;
	using TexCompressionBC4Ptr = std::shared_ptr<TexCompressionBC4>;
	class LE_API TexCompressionBC3 : public TexCompression
	{
	public:
		TexCompressionBC3();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC1Ptr bc1_codec_;
		TexCompressionBC4Ptr bc4_codec_;
	};

	class LE_API TexCompressionBC4 : public TexCompression
	{
	public:
		TexCompressionBC4();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;
	};

	class LE_API TexCompressionBC5 : public TexCompression
	{
	public:
		TexCompressionBC5();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC4Ptr bc4_codec_;
	};

	class LE_API TexCompressionBC6U : public TexCompression
	{
	public:
		TexCompressionBC6U();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

		void DecodeBC6Internal(void* output, void const * input, bool signed_fmt);

	private:
		int Unquantize(int comp, uint8 bits_per_comp, bool signed_fmt);
		int FinishUnquantize(int comp, bool signed_fmt);

	private:
		static uint32 const BC6_MAX_REGIONS = 2;
		static uint32 const BC6_MAX_INDICES = 16;

		static int32_t const BC6_WEIGHT_MAX = 64;
		static uint32 const BC6_WEIGHT_SHIFT = 6;
		static int32_t const BC6_WEIGHT_ROUND = 32;

		enum ModeField
		{
			NA, // N/A
			M,  // Mode
			D,  // Shape
			RW,
			RX,
			RY,
			RZ,
			GW,
			GX,
			GY,
			GZ,
			BW,
			BX,
			BY,
			BZ,
		};

		struct ModeDescriptor
		{
			ModeField field;
			uint8 bit;
		};

		struct ModeInfo
		{
			uint8 mode;
			uint8 partitions;
			bool transformed;
			uint8 index_prec;
			ARGBColor32 rgba_prec[BC6_MAX_REGIONS][2];
		};

		static ModeDescriptor const mode_desc_[][82];
		static ModeInfo const mode_info_[];
		static int const mode_to_info_[];
	};
	using TexCompressionBC6UPtr = std::shared_ptr<TexCompressionBC6U>;

	class LE_API TexCompressionBC6S : public TexCompression
	{
	public:
		TexCompressionBC6S();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		TexCompressionBC6UPtr bc6u_codec_;
	};

	class LE_API TexCompressionBC7 : public TexCompression
	{
		static uint32 const BC7_MAX_REGIONS = 3;
		static uint32 const BC7_NUM_CHANNELS = 4;
		static uint32 const BC7_MAX_SHAPES = 64;

		static uint32 const BC7_WEIGHT_MAX = 64;
		static uint32 const BC7_WEIGHT_SHIFT = 6;
		static uint32 const BC7_WEIGHT_ROUND = 32;

		enum PBitType
		{
			PBT_None,
			PBT_Shared,
			PBT_Unique
		};

		struct ModeInfo
		{
			uint8 partitions;
			uint8 partition_bits;
			uint8 p_bits;
			uint8 rotation_bits;
			uint8 index_mode_bits;
			uint8 index_prec_1;
			uint8 index_prec_2;
			ARGBColor32 rgba_prec;
			ARGBColor32 rgba_prec_with_p;
			PBitType p_bit_type;
		};

		struct CompressParams
		{
			float4 p1[BC7_MAX_REGIONS], p2[BC7_MAX_REGIONS];
			uint8 indices[BC7_MAX_REGIONS][16];
			uint8 alpha_indices[16];
			uint8 pbit_combo[BC7_MAX_REGIONS];
			int8 rotation_mode;
			int8 index_mode;
			uint32 shape_index;

			CompressParams()
			{
			}
			explicit CompressParams(uint32 shape)
				: rotation_mode(-1), index_mode(-1), shape_index(shape)
			{
				memset(indices, 0xFF, sizeof(indices));
				memset(alpha_indices, 0xFF, sizeof(alpha_indices));
				memset(pbit_combo, 0xFF, sizeof(pbit_combo));
			}
		};

	public:
		TexCompressionBC7();

		virtual void EncodeBlock(void* output, void const * input, TexCompressionMethod method) override;
		virtual void DecodeBlock(void* output, void const * input) override;

	private:
		void PrepareOptTable(uint8* table, uint8 const * expand, int size) const;
		void PrepareOptTable2(uint8* table, uint8 const * expand, int size) const;
		void PackBC7UniformBlock(void* output, ARGBColor32 const & pixel);
		void PackBC7Block(int mode, CompressParams& params, void* output);
		int RotationMode(ModeInfo const & mode_info) const;
		int NumBitsPerIndex(ModeInfo const & mode_info, int8 index_mode = -1) const;
		int NumBitsPerAlpha(ModeInfo const & mode_info, int8 index_mode = -1) const;
		uint4 ErrorMetric(ModeInfo const & mode_info) const;
		ARGBColor32 QuantizationMask(ModeInfo const & mode_info) const;
		int NumPbitCombos(ModeInfo const & mode_info) const;
		int const * PBitCombo(ModeInfo const & mode_info, int idx) const;
		uint64_t OptimizeEndpointsForCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8* best_indices, uint8& best_pbit_combo) const;
		void PickBestNeighboringEndpoints(int mode, float4 const & p1, float4 const & p2,
			int cur_pbit_combo, float4& np1, float4& np2, int& pbit_combo, float step_sz = 1) const;
		bool AcceptNewEndpointError(uint64_t new_err, uint64_t old_err, float temp) const;
		uint64_t CompressSingleColor(ModeInfo const & mode_info,
			ARGBColor32 const & pixel, float4& p1, float4& p2, uint8& best_pbit_combo) const;
		uint64_t CompressCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8* best_indices, uint8& best_pbit_combo) const;
		uint64_t CompressCluster(int mode, RGBACluster const & cluster,
			float4& p1, float4& p2, uint8 *best_indices, uint8* alpha_indices) const;
		void ClampEndpoints(float4& p1, float4& p2) const;
		void ClampEndpointsToGrid(ModeInfo const & mode_info,
			float4& p1, float4& p2, uint8& best_pbit_combo) const;
		uint64_t TryCompress(int mode, int simulated_annealing_steps, TexCompressionErrorMetric metric,
			CompressParams& params, uint32 shape_index, RGBACluster& cluster);

		uint8 Unquantize(uint8 comp, size_t prec) const;
		ARGBColor32 Unquantize(ARGBColor32 const & c, ARGBColor32 const & rgba_prec) const;
		ARGBColor32 Interpolate(ARGBColor32 const & c0, ARGBColor32 const & c1,
			size_t wc, size_t wa, size_t wc_prec, size_t wa_prec) const;

	private:
		int sa_steps_;
		TexCompressionErrorMetric error_metric_;
		int rotate_mode_;
		int index_mode_;

		static ModeInfo const mode_info_[];

		static uint8 expand6_[64];
		static uint8 expand7_[128];
		static uint8 o_match6_[256][2];
		static uint8 o_match7_[256][2];
		static bool lut_inited_;
	};

	void BC4ToBC1G(BC1Block& bc1, BC4Block const & bc4);
}


#endif