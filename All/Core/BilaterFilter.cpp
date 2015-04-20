#include "BilateralFilter.hpp"
#include "..\file.hpp"
#include "FileSearch.h"
namespace leo {
	void CompilerBilaterCS(unsigned int radius, const wchar_t* savepath) {
		auto inFile = win::File::Open(FileSearch::Search(L"BilateralFilterCS.hlsl"),win::File::TO_READ|win::File::NO_CREATE);
		auto outFile = win::File::Open(savepath, win::File::TO_WRITE);

		const static double rsigma = 0.051;

		std::string prev = "#define RADIUS ";
		prev += std::to_string(radius);

		prev += "\r\n\r\n";

		prev += "static const float filter[RADIUS][RADIUS] = {";
		{
			auto filter = std::make_unique<float[]>(radius*radius);

			auto weight = 0.f;
			int middle = (radius - 1) / 2;
			for (int x = 0; x != radius; ++x)
				for (int y = 0; y != radius; ++y) {
					auto delta = -((x - middle)*(x - middle) + (y - middle)*(y - middle));
					auto range = exp(delta / (2 * rsigma*rsigma))/(2*3.14159265*rsigma*rsigma);
					filter[x + y*radius] = range;
					weight += range;
				}
			for (unsigned int x = 0; x != radius; ++x)
				for (unsigned int y = 0; y != radius; ++y) 
					filter[x + y*radius] /= weight;

			for (unsigned int x = 0; x != radius; ++x)
				for (unsigned int y = 0; y != radius; ++y) {
					prev += std::to_string(filter[x + y*radius]);
					prev += ",";
				}
		}
		prev += "};\r\n\r\n";
	}
}