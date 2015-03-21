#pragma once
#include "debug.hpp"
namespace leo
{
	namespace win
	{
		namespace details {
			template<typename COM,typename = decltype(&COM::SetPrivateData)>
			void Print(COM* &com, decltype(&COM::SetPrivateData)) {
				UINT DataSize;
				com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, nullptr);
				if (DataSize != 0) {
					std::string name;
					name.resize(DataSize);
					com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, &name[0]);
					if (DataSize > 1 && (name[1] == char())) {
						std::wstring wname;
						wname.resize(DataSize / 2);
						com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, &wname[0]);
						DebugPrintf(L"%s address: %p,refcount: ", wname.c_str(), com);
					}
					else
						DebugPrintf("%s address: %p,refcount: ", name.c_str(), com);
					return;
				}
				DebugPrintf("UNKNOWN address: %p,refcount: ", com);
			}

			template<typename COM>
			void Print(COM* &com,...) {
				DebugPrintf("UNKNOWN address: %p,refcount: ", com);
			}

			template<typename COM>
			void PrintAndRelease(COM* &com) {
				Print(com,nullptr);
				DebugPrintf("%u\n", com->Release());
			}
		}

		template<typename COM>
		void ReleaseCOM(COM* &com)
		{
			if (com) {
				details::PrintAndRelease(com);
			}
			com = nullptr;
		}
	}
}