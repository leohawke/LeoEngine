#include <platform.h>
#include "d3dx11.hpp"
#include "PostProcess.hpp"

using namespace leo;

leo::PostProcess::PostProcess(ID3D11Device*) {
	++mCommonThunk.mRefCount;
	if (mCommonThunk.mRefCount && !mCommonThunk.mVertexShader) {

	}
}

leo::PostProcess::~PostProcess() {
	--mCommonThunk.mRefCount;
	if (!mCommonThunk.mRefCount && mCommonThunk.mVertexShader) {

	}
}

leo::PostProcess::PostProcess(PostProcess && rvalue)
:mPixelShader(std::move(rvalue.mPixelShader)),mVertexBuffer(std::move(rvalue.mVertexBuffer)){
	assert(mCommonThunk.mRefCount);
	++mCommonThunk.mRefCount;
}

void leo::PostProcess::operator=(PostProcess&& rvalue) {
	PostProcess(std::move(rvalue));
}

bool leo::PostProcess::BindProcess(ID3D11Device* device, const std::string& psfilename) {
	return BindProcess(device, psfilename.c_str());
}
bool leo::PostProcess::BindProcess(ID3D11Device* device, const char* psfilename) {
	return false;
}

namespace {
	struct Vertex {
		float4 PosH;
		float2 Tex;
	};
}

bool leo::PostProcess::BindRect(ops::Rect& src, ops::Rect& dst) {
	return false;
}

void leo::PostProcess::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) {

}