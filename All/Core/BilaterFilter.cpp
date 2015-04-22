#include "BilateralFilter.hpp"
#include "..\file.hpp"
#include "FileSearch.h"
#include <sstream>
#include <iomanip>
#include <fstream>

#include <d3dcompiler.h>
#pragma warning(disable: 4172)

#include <exception.hpp>
#include <..\debug.hpp>
namespace leo {
	void CompilerBilaterCS(unsigned int radius, const wchar_t* savepath) {

		const static double rsigma = 1.6;

		std::string prev = "#define RADIUS ";
		prev += std::to_string(radius);

		prev += "\r\n\r\n";

		prev += "static const float filter[RADIUS][RADIUS] = {";
		{
			auto filter = std::make_unique<float[]>(radius*radius);

			std::stringstream strstream;

			auto weight = 0.f;
			int middle = (radius - 1) / 2;
			for (int x = 0; x != radius; ++x)
				for (int y = 0; y != radius; ++y) {
					auto delta = -((x - middle)*(x - middle) + (y - middle)*(y - middle));
					auto power = delta*0.25 - 2.507901035590337872276786476796;
					auto range = exp(power);
					filter[x + y*radius] =static_cast<float>(range);
					weight += static_cast<float>(range);
				}
			for (unsigned int x = 0; x != radius; ++x)
				for (unsigned int y = 0; y != radius; ++y) 
					filter[x + y*radius] /= weight;

			strstream << std::setprecision(6);
			for (unsigned int x = 0; x != radius; ++x) {
				for (unsigned int y = 0; y != radius; ++y) {
					strstream<<filter[x + y*radius] << ',';
				}
				strstream << std::endl<<'\t';
			}

			prev += strstream.str();

		}
		prev += "};\r\n\r\n";


		std::ifstream inFile(FileSearch::Search(L"BilateralFilterCS.hlsl"));
		std::string str { std::istreambuf_iterator<char>(inFile),
			std::istreambuf_iterator<char>() };

		prev += str;


		UINT flags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		HRESULT hr = E_FAIL;

		ID3DBlob* pBlobCS = nullptr;
		ID3DBlob* pErrorBlob = nullptr;

		hr = D3DCompile(prev.data(), prev.size(), nullptr, nullptr, nullptr, "main", "cs_5_0", flags, 0, &pBlobCS, &pErrorBlob);
			
		if (pErrorBlob) {
			DebugPrintf((const char*)(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
			pBlobCS? pBlobCS->Release():0;
			Raise_DX_Exception(hr)
		}
		
		auto outFile = win::File::Open(savepath, win::File::TO_WRITE);

		outFile->Write(0, pBlobCS->GetBufferPointer(), pBlobCS->GetBufferSize());
		pBlobCS->Release();
	}
}