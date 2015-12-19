#include "FreeTypeTest.h"
#include <RenderSystem\D3D11\D3D11Texture.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


FT_Library library = nullptr;
FT_Face face;

#define FT_ERROR(msg) if(error) LAssert(false,msg)

void FillTexture(leo::TexturePtr dst, wchar_t c)
{
	auto error = FT_Init_FreeType(&library);
	FT_ERROR("FreeType Init Failure");

	error = FT_New_Face(library, "Fonts/msyh.ttc", 0, &face);
	FT_ERROR("Read Fonts File Failure");

	error = FT_Set_Char_Size(face, 0, 2048 * 64, 96, 96);
	FT_ERROR("Set Char Size Failure");
	
	auto glyph_index = FT_Get_Char_Index(face,c);
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	FT_ERROR("Load Glyph Failure");
	if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_ERROR("Render Glyph Failure");
	}

	face->glyph->bitmap;

	leo::Texture::Mapper mapper{ *dst, 0, 0, leo::Texture::MapAccess::MA_WO, 0, 0, 4096, 4096 };

	auto pByte = mapper.Pointer<unsigned char>();

	auto & bitmap = face->glyph->bitmap;
	//ÃÓ≥‰bitmap ÷¡ texture
	for (auto row_index = 0; row_index != bitmap.rows; ++row_index) {
		for (auto x = 0; x != bitmap.width; ++x) {
			pByte[row_index*mapper.RowPitch + x] = bitmap.buffer[row_index*bitmap.pitch + x];
		}
	}
}
