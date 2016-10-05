#include "TextureX.h"

namespace platform {
	void X::GetImageInfo(File const & file, Render::TextureType & type, uint16 & width, uint16 & height, uint16 & depth, uint8 & num_mipmaps, uint8 & array_size, Render::EFormat & format, uint32 & row_pitch, uint32 & slice_pitch)
	{
		FileRead tex_res{ file };

		uint32 magic;
		tex_res.Read(&magic, sizeof(magic));
	}
}
