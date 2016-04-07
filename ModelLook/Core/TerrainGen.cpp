#include "TerrainGen.h"
#include "Core/EffectTerrain.hpp"
#include "Core/EffectTerrainSO.hpp"
#include "Core/COM.hpp"
#include "Core/FileSearch.h"
#include "Core/EngineConfig.h"

#include "RenderSystem/d3dx11.hpp"
#include "RenderSystem/D3D11/D3D11Texture.hpp"
#include "RenderSystem/TextureX.hpp"
#include "RenderSystem/RenderStates.hpp"

namespace leo {
	namespace todo {
		/*
		float GetHeight(const float2& xz) const{
		//初始化一些东西
		static auto mScreenSize = DeviceMgr().GetClientSize();
		static auto mAveChunkNum = static_cast<uint32>((mScreenSize.first*mScreenSize.second) / (MINEDGEPIXEL*3.f*MAXEDGE*MAXEDGE))+3u;
		static std::map < std::pair<uint32, uint32>, std::vector<float>> mMap;
		static auto mSizePerEDGE = 1.f*mChunkSize / MAXEDGE;
		auto base = Offset(0, 0);
		base.x -= (mChunkSize / 2.f);
		base.y += (mChunkSize / 2.f);
		auto slotX = static_cast<uint32>((xz.x - base.x)/mChunkSize);
		auto slotY = static_cast<uint32>((base.y - xz.y) /mChunkSize);
		auto chunk = std::make_pair(slotX,slotY);
		if (mMap.find(chunk) == mMap.cend()) {
		if (mMap.size() > mAveChunkNum)
		mMap.erase(mMap.begin());
		//流输出
		auto context = DeviceMgr().GetDeviceContext();
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		UINT strides[] = { sizeof(Terrain::Vertex) };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, &mCommonVertexBuffer, strides, offsets);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Terrain));

		auto & mEffect = EffectTerrainSO::GetInstance();
		mEffect->HeightMap(mHeightMap);
		mEffect->UVScale(float2(1.f / mHorChunkNum / mChunkSize, 1.f / mVerChunkNum / mChunkSize));
		mEffect->WorldOffset(Offset(slotX, slotY));
		static UINT pOffsets[1] = { 0 };
		mEffect->SetSOTargets(1, &mSOTargetBuffer, pOffsets);


		mEffect->Apply(context);

		context->Draw((MAXEDGE + 1)*(MAXEDGE + 1), 0);

		//拷贝
		context->CopyResource(mReadBuffer, mSOTargetBuffer);
		//映射,读取
		std::vector<SOOutput> mHeights;
		D3D11_MAPPED_SUBRESOURCE mapSubRes;
		try {
		dxcall(context->Map(mReadBuffer, 0, D3D11_MAP_READ, 0, &mapSubRes));
		std::transform(
		(SOOutput*)mapSubRes.pData,
		((SOOutput*)mapSubRes.pData) + (MAXEDGE + 1)*(MAXEDGE + 1),
		std::back_insert_iterator<std::vector<SOOutput>>(mHeights),
		[](const SOOutput& data)
		{
		return data;
		}
		);
		context->Unmap(mReadBuffer, 0);
		}
		Catch_DX_Exception
		std::sort(mHeights.begin(), mHeights.end());
		std::vector<float> mY;
		std::transform(mHeights.begin(),mHeights.end(),std::back_insert_iterator<std::vector<float>>(mY),
		[](const SOOutput& data) {
		return data.H;
		});
		mMap.emplace(chunk,std::move(mY));
		}

		auto offset = Offset(slotX, slotY);
		offset.x -= (mChunkSize / 2.f);
		offset.y += (mChunkSize / 2.f);
		std::pair<uint32, uint32> mSamples[4];
		//暂时忽略有可能出现的数值错误,比如位于一个Chunk的边
		mSamples[0].first = static_cast<uint32>(((xz.x - offset.x) / mChunkSize) * MAXEDGE);
		mSamples[0].second = static_cast<uint32>(((offset.y - xz.y) / mChunkSize)*MAXEDGE);

		mSamples[1].first = mSamples[0].first + 1;
		mSamples[1].second = mSamples[0].second;

		mSamples[2].first = mSamples[0].first;
		mSamples[2].second = mSamples[0].second + 1;

		mSamples[3].first = mSamples[0].first + 1;
		mSamples[3].second = mSamples[0].second + 1;

		//水平和竖直上的插值
		auto xt = ((xz.x - offset.x) - mSamples[0].first*mSizePerEDGE) / mSizePerEDGE;
		auto yt = ((offset.y - xz.y) - mSamples[0].second*mSizePerEDGE) / mSizePerEDGE;

		return Lerp(
		Lerp(
		mMap[chunk][mSamples[0].second*(MAXEDGE + 1) + mSamples[0].first],
		mMap[chunk][mSamples[1].second*(MAXEDGE + 1) + mSamples[1].first],
		xt),
		Lerp(
		mMap[chunk][mSamples[2].second*(MAXEDGE + 1) + mSamples[2].first],
		mMap[chunk][mSamples[3].second*(MAXEDGE + 1) + mSamples[3].first],
		xt),
		yt);
		}
		*/

		/*struct SOOutput {
			union
			{
				struct {
					float H;
					uint32 Id;
				};
				uint64 data;
			};

			bool operator<(const SOOutput& rhs) {
				return Id < rhs.Id;
			}
		};

		ID3D11Buffer* mSOTargetBuffer = nullptr;
		ID3D11Buffer* mReadBuffer = nullptr;
		
		
		void CreateSOBuffer(ID3D11Device* device) {
			CD3D11_BUFFER_DESC sovbDesc(8 * (MAXEDGE + 1)*(MAXEDGE + 1), D3D11_BIND_STREAM_OUTPUT, D3D11_USAGE_DEFAULT);
			dxcall(device->CreateBuffer(&sovbDesc, nullptr, &mSOTargetBuffer));
			DebugDXCOM(mSOTargetBuffer);

			CD3D11_BUFFER_DESC readvbDesc(8 * (MAXEDGE + 1)*(MAXEDGE + 1), 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);
			dxcall(device->CreateBuffer(&readvbDesc, nullptr, &mReadBuffer));
			DebugDXCOM(mReadBuffer);

			{
				
			
			}
		}*/
	}
	
	bool ReBuildTerrain(ID3D11Device * device, ID3D11DeviceContext * context, const std::wstring & src_terrain, const uint8& maxedage, const std::wstring & dst_terrain, const std::wstring & height_map_path)
	{
		TerrainFileHeader header;
		auto pFile = leo::win::File::Open(src_terrain, win::File::TO_READ | win::File::NO_CREATE);
		pFile->Read(&header, sizeof(TerrainFileHeader), 0);

		return GenTerrainEx(device,context,header,maxedage,dst_terrain,height_map_path);
	}

	template<typename T>
	using com_ptr = leo::win::unique_com<T>;

	bool GenTerrainEx(ID3D11Device * device, ID3D11DeviceContext * context, const TerrainFileHeader & header, const uint8& maxedage, const std::wstring & dst_terrain, const std::wstring & height_map_path)
	{
		auto HorChunkNum = header.mHorChunkNum;
		auto VerChunkNum = header.mVerChunkNum;
		auto select_tex_size = [&](uint16 ord_size) {
			const uint16 tex_opt_size[] = { 256,512,1024,2048,4096 };
			size_t i = 0;
			for (; i != arrlen(tex_opt_size);) {
				if (ord_size > tex_opt_size[i])
					++i;
				else
					break;
			}
			i = std::min<size_t>(i, arrlen(tex_opt_size) - 1);
			return tex_opt_size[i];
		};

		auto HeightMapHorSize = select_tex_size(uint16(HorChunkNum*maxedage));
		auto HeightMapVerSize = select_tex_size(uint16(VerChunkNum* maxedage));

		float4 Params{ 1.f / HeightMapHorSize,1.f / HeightMapVerSize,5.f,5.f };
		com_ptr<ID3D11Buffer> mCHMCSCB = nullptr;
		dx::CreateGPUCBuffer(device,Params,mCHMCSCB, "TerrainCHMCSParams");
		
		auto mHeightMapTex =X::MakeTexture2D(HeightMapHorSize, HeightMapVerSize, 1, 1, EF_R32F, {}, EA_G_R | EA_G_U,nullptr);
		auto mHeightMapD3DTex = static_cast<D3D11Texture2D*>(mHeightMapTex.get());

		auto mNoiseMapTex = X::SyncLoadTexture(header.mHeightMap, EA_G_R);
		auto mNoiseMapD3DTex = static_cast<D3D11Texture2D*>(mNoiseMapTex.get());

		auto computer_shader = ShaderMgr().CreateComputeShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrain", D3D11_COMPUTE_SHADER)));

		auto point_sampler = RenderStates().GetSamplerState(L"NearestRepeat");

		context->CSSetShader(computer_shader, nullptr, 0);
		auto mHeightMapUAV = mHeightMapD3DTex->AccessView();
		context->CSSetUnorderedAccessViews(0, 1, &mHeightMapUAV, nullptr);
		context->CSSetConstantBuffers(0, 1, &mCHMCSCB);
		auto mNoiseMapSRV = mNoiseMapD3DTex->ResourceView();
		context->CSSetShaderResources(0, 1, &mNoiseMapSRV);
		context->CSSetSamplers(0, 1, &point_sampler);

		context->Dispatch(HeightMapHorSize / 32, HeightMapVerSize / 32, 1);

		ID3D11UnorderedAccessView* mNullptrUAV = nullptr;
		context->CSSetUnorderedAccessViews(0, 1, &mNullptrUAV, nullptr);
		context->CSSetShader(nullptr, nullptr, 0);

		TerrainFileHeaderEx headerEx;
		memcpy(headerEx, header);
		memset(headerEx.mHeightMap, 0);

		return false;
	}
}