#include "TerrainGen.h"
#include "Core/EffectTerrain.hpp"
#include "Core/EffectTerrainSO.hpp"
#include "Core/COM.hpp"
#include "Core/FileSearch.h"
#include "Core/EngineConfig.h"

#include "RenderSystem/d3dx11.hpp"
#include "RenderSystem/ShaderMgr.h"
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

		struct SOOutput {
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

		ID3D11ComputeShader* mCHMCS = nullptr;
		ID3D11ShaderResourceView* mNoiseMap = nullptr;
		uint16 mNormaMapHorSize;
		uint16 mNormaMapVerSize;

		std::uint32_t mHorChunkNum;
		std::uint32_t mVerChunkNum;


		ID3D11Buffer* mSOTargetBuffer = nullptr;
		ID3D11Buffer* mReadBuffer = nullptr;
		leo::win::unique_com<ID3D11Buffer> mCHMCSCB = nullptr;
		leo::win::unique_com<ID3D11UnorderedAccessView> mHeightMapUAV = nullptr;
		leo::win::unique_com<ID3D11ShaderResourceView> mHeightMapSRV = nullptr;
		//leo::win::ReleaseCOM(mSOTargetBuffer);
		//		
		//leo::win::ReleaseCOM(mReadBuffer);
		ID3D11SamplerState* mSS = nullptr;
		void ComputerHeightMap(ID3D11DeviceContext* context) {
			ID3D11ShaderResourceView* nullSRV = nullptr;
			context->VSSetShaderResources(0, 1, &nullSRV);
			context->PSSetShaderResources(3, 1, &nullSRV);

			context->CSSetShader(mCHMCS, nullptr, 0);
			context->CSSetUnorderedAccessViews(0, 1, &mHeightMapUAV, nullptr);
			context->CSSetConstantBuffers(0, 1, &mCHMCSCB);
			context->CSSetShaderResources(0, 1, &mNoiseMap);
			context->CSSetSamplers(0, 1, &mSS);

			context->Dispatch(mNormaMapHorSize / 32, mNormaMapVerSize / 32, 1);

			ID3D11UnorderedAccessView* mNullptrUAV = nullptr;
			context->CSSetUnorderedAccessViews(0, 1, &mNullptrUAV, nullptr);
			context->CSSetShader(nullptr, nullptr, 0);
		}

#define MAXEDGE 128
		void CreateSOBuffer(ID3D11Device* device) {
			CD3D11_BUFFER_DESC sovbDesc(8 * (MAXEDGE + 1)*(MAXEDGE + 1), D3D11_BIND_STREAM_OUTPUT, D3D11_USAGE_DEFAULT);
			dxcall(device->CreateBuffer(&sovbDesc, nullptr, &mSOTargetBuffer));
			DebugDXCOM(mSOTargetBuffer);

			CD3D11_BUFFER_DESC readvbDesc(8 * (MAXEDGE + 1)*(MAXEDGE + 1), 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);
			dxcall(device->CreateBuffer(&readvbDesc, nullptr, &mReadBuffer));
			DebugDXCOM(mReadBuffer);

			{
				const uint16 size[] = { 256,512,1024,2048,4096 };

				auto clamp_size = [&](uint16 ord_size) {
					size_t i = 0;
					for (; i != arrlen(size);) {
						if (ord_size > size[i])
							++i;
						else
							break;
					}
					i = std::min<size_t>(i, arrlen(size) - 1);
					return size[i];
				};

				mNormaMapHorSize = clamp_size(uint16(mHorChunkNum*MAXEDGE));
				mNormaMapVerSize = clamp_size(uint16(mVerChunkNum* MAXEDGE));

				float4 Params{ 1.f / mNormaMapHorSize,1.f / mNormaMapVerSize,5.f,5.f };

				D3D11_BUFFER_DESC Desc;
				Desc.Usage = D3D11_USAGE_DEFAULT;
				Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				Desc.CPUAccessFlags = 0;
				Desc.MiscFlags = 0;
				Desc.StructureByteStride = 0;
				Desc.ByteWidth = sizeof(Params);

				D3D11_SUBRESOURCE_DATA Data;
				Data.pSysMem = &Params;

				dxcall(device->CreateBuffer(&Desc, &Data, &mCHMCSCB));


				D3D11_TEXTURE2D_DESC NormalMapTexDesc;

				NormalMapTexDesc.Format = DXGI_FORMAT_R32_FLOAT;

				NormalMapTexDesc.ArraySize = 1;
				NormalMapTexDesc.MipLevels = 1;

				NormalMapTexDesc.SampleDesc.Count = 1;
				NormalMapTexDesc.SampleDesc.Quality = 0;

				NormalMapTexDesc.Width = mNormaMapHorSize;
				NormalMapTexDesc.Height = mNormaMapVerSize;

				NormalMapTexDesc.Usage = D3D11_USAGE_DEFAULT;
				NormalMapTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
				NormalMapTexDesc.CPUAccessFlags = 0;
				NormalMapTexDesc.MiscFlags = 0;

				auto mTex = leo::win::make_scope_com<ID3D11Texture2D>();

				leo::dxcall(device->CreateTexture2D(&NormalMapTexDesc, nullptr, &mTex));
				leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mHeightMapSRV));
				leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mHeightMapUAV));

				leo::ShaderMgr SM;
				mCHMCS = SM.CreateComputeShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrain", D3D11_COMPUTE_SHADER)));

				leo::RenderStates SS;
				mSS = SS.GetSamplerState(L"NearestRepeat");
			}
		}
	}
}