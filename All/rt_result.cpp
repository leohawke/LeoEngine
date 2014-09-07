#include "File.hpp"
#include "Debug.hpp"
#include <windows.h>
#include <algorithm>
#include <type_traits>
#include <exception>

namespace leo
{
	namespace win
	{
		template<typename _Ty>
		bool kernel_check(_Ty *pointer)
		{
			return pointer != nullptr && pointer != INVALID_HANDLE_VALUE;
		}
		template<typename _Ty>
		void kernel_delete(_Ty *pointer)
		{
			CloseHandle(pointer);
		}
		/*
		void force_compiler_kernel_check_and_delete()
		{
			kernel_check(INVALID_HANDLE_VALUE);
			kernel_delete(INVALID_HANDLE_VALUE);
		}
		*/
		namespace file
		{
			
#pragma region TempFile
			wchar_t TempFile::mPath[max_path] = L".\0";
			wchar_t TempFile::mPrefix[TempFile::max_prefix] = L"temp\0";
			TempFile::TempFile()
			{
				::GetTempFileName(mPath,mPrefix, 0, mTempFileName);
			}
			TempFile::TempFile(const std::wstring & path, const std::wstring & prefix)
			{
				::GetTempFileName(path.c_str(), prefix.c_str(), 0, mTempFileName);
			}
			TempFile::TempFile(const wchar_t * path, const wchar_t * prefix)
			{
				::GetTempFileName(path,prefix, 0, mTempFileName);
			}
			TempFile::~TempFile()
			{
				::DeleteFile(mTempFileName);
			}

			const wchar_t * TempFile::c_str() const
			{
				return mTempFileName;
			}

			void TempFile::Path(wchar_t * path)
			{
				if (path)
				{
					::lstrcpyW(mPath, path);
				}
				else
				{
					GetTempPath(max_path, mPath);
				}
			}
			void TempFile::Prefix(wchar_t * prefix)
			{
				::lstrcpyW(mPrefix,prefix);
			}
#pragma endregion
#pragma region HuffManCode
			struct HuffManCodeHeader
			{
				const static size_t arraysize = 16;
				BYTE CODE[arraysize];
				BYTE length[arraysize];
				size_t filesize;
			};
			struct HuffManTreeNode
			{
				size_t weight;
				HuffManTreeNode* parent;
				HuffManTreeNode* left;
				HuffManTreeNode* right;
			};
#define DebugErrorAndThrow {DebugLastError();throw;}
			size_t EncodeHeader(size_t weight[HuffManCodeHeader::arraysize], HuffManCodeHeader& header)
			{
				auto heap_check = [](HANDLE hHeap){return hHeap != nullptr; };
				auto heap_delete = [](HANDLE hHeap){HeapDestroy(hHeap); };
				const static SIZE_T HEAP_SIZE = sizeof(HuffManTreeNode)* HuffManCodeHeader::arraysize*2;
				unique_handle hHeap(HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, HEAP_SIZE, 0), heap_check, heap_delete);
				if (!hHeap.is_good())
					DebugErrorAndThrow;
				HuffManTreeNode * huffnodes = (HuffManTreeNode *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, HEAP_SIZE);
				size_t n = 0;
				for (size_t i = 0; i != HuffManCodeHeader::arraysize; ++i){
					huffnodes[i].weight = weight[i];
					if (weight[i])
						++n;
				}
				huffnodes[HuffManCodeHeader::arraysize*2-1].weight = -1;
				auto select = [&huffnodes](size_t end, size_t &left, size_t &right)
				{
					size_t min = HuffManCodeHeader::arraysize * 2 - 1;
					for (size_t i = 0; i != end; ++i){
						if (!huffnodes[i].parent && huffnodes[i].weight && huffnodes[i].weight < huffnodes[min].weight){
							min = i;
						}
					}
					left = min;
					huffnodes[min].parent = (HuffManTreeNode*)-1;
					min = HuffManCodeHeader::arraysize * 2 - 1;
					for (size_t i = 0; i != end; ++i){
						if (!huffnodes[i].parent && huffnodes[i].weight && huffnodes[i].weight < huffnodes[min].weight){
							min = i;
						}
					}
					right = min;
					huffnodes[left].parent = nullptr;
				};
				size_t left, right;
				if (n == 1){
					select(HuffManCodeHeader::arraysize, left, right);
					huffnodes[left].parent = huffnodes + HuffManCodeHeader::arraysize;
					huffnodes[HuffManCodeHeader::arraysize].left = huffnodes + left;
					huffnodes[HuffManCodeHeader::arraysize].weight = huffnodes[left].weight;
				}
				for (size_t i = HuffManCodeHeader::arraysize; i != HuffManCodeHeader::arraysize + n - 1; ++i){
					select(i, left, right);
					huffnodes[left].parent = huffnodes + i; huffnodes[right].parent = huffnodes + i;
					huffnodes[i].left = huffnodes + left; huffnodes[i].right = huffnodes + right;
					huffnodes[i].weight = huffnodes[left].weight + huffnodes[right].weight;
				}
				huffnodes[HuffManCodeHeader::arraysize * 2 - 1].weight = 0;
				BYTE code = 0;
				BYTE length = 0;
				size_t wpl = 0;
				for (size_t i = 0; i != HuffManCodeHeader::arraysize; ++i)
				{
					code = 0;
					length = 0;
					if (weight[i]){
						HuffManTreeNode* pNode = huffnodes + i;
						HuffManTreeNode* pParent = pNode->parent;
						for (; pParent; pNode = pParent, pParent = pNode->parent){
							code <<= 1;
							length++;
							if (pParent->left == pNode)
								code |= 0;
							else
								code |= 1;
						}
					}
					header.CODE[i] = code;
					header.length[i] = length;
					wpl += weight[i] * length;
				}
				wpl =(wpl+7)/8;
				return wpl;
			}
			void HuffManEncode(wchar_t * src, wchar_t * dst)
			{
				unique_handle hIn(CreateFile(src, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
				if (!hIn.is_good())
					DebugErrorAndThrow;
				LARGE_INTEGER fileSize;
				GetFileSizeEx(hIn, &fileSize);
				if (fileSize.HighPart > 0 && sizeof(SIZE_T) == 4)
					return;//this means File is too large fo win32(because sizeof(SIZE_T) == 4)
				unique_handle hInMap(CreateFileMapping(hIn, nullptr, PAGE_READONLY, 0, 0, nullptr));
				if (!hInMap.is_good())
					DebugErrorAndThrow;
				BYTE* pInFile = (BYTE*)MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);
				size_t weight[HuffManCodeHeader::arraysize] = { 0 };
				const BYTE* pIn = nullptr;
				try{
					pIn = pInFile;
					size_t Low, High;
					while (pIn < pInFile + fileSize.QuadPart){
						Low = (*pIn) &0x0F;
						High = ((*pIn) & 0xF0) >> 4;
						++weight[Low];
						++weight[High];
						++pIn;
					}
				}
				catch (std::exception& e)
				{
					DebugPrintf(L"%s", e.what());
				}
				UnmapViewOfFile(pInFile);
				HuffManCodeHeader huffheader = {0};
				huffheader.filesize = (size_t)fileSize.QuadPart;
				size_t WPl = EncodeHeader(weight, huffheader);
				pInFile = (BYTE*)MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);

				LARGE_INTEGER outfileSize;
				outfileSize.QuadPart = WPl + sizeof(huffheader);
				unique_handle hOut(CreateFile(dst, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
				if (!hOut.is_good())
					DebugErrorAndThrow;
				unique_handle hOutMap(CreateFileMapping(hOut, nullptr, PAGE_READWRITE, outfileSize.HighPart, outfileSize.LowPart, nullptr));
				if (!hOutMap.is_good())
					DebugErrorAndThrow;
				BYTE* pOutFile = (BYTE*)MapViewOfFile(hOutMap, FILE_MAP_WRITE, 0, 0, (SIZE_T)outfileSize.QuadPart);
				BYTE* pOut = nullptr;
				std::remove_reference<decltype(huffheader.CODE[0])>::type code = 0;
				BYTE length = 0;
				try{
					pIn = (BYTE*)&huffheader;
					pOut = pOutFile;
					while (pIn < ((BYTE*)&huffheader) + sizeof(huffheader)){
						*pOut = *pIn;
						++pIn; ++pOut;
					}
					pIn = pInFile;
					size_t Low, High;
					auto write4bit = [&length, &pOut, &code, &huffheader](size_t index)
					{
						for (BYTE i = 0; i != huffheader.length[index]; ++i)
						{
							if (length == 8){
								*pOut = code;
								pOut++;
								length = 0;
								code = 0;
							}
							code <<= 1;
							code |= (huffheader.CODE[index] & (1 << i)) >> i;
							length++;
						}
					};
					while (pIn < pInFile + fileSize.QuadPart){
						Low = (*pIn) & 0x0F;
						High = ((*pIn) & 0xF0) >> 4;
						write4bit(Low);
						write4bit(High);
						pIn++;
					}
					if (length != 8){
						code <<= (8 - length);
					}
					*pOut = code;
				}
				catch (std::exception& e)
				{
					DebugPrintf(L"%s", e.what());
				}
				UnmapViewOfFile(pOutFile);
				UnmapViewOfFile(pInFile);
			}
			void HuffManDecode(wchar_t * src, wchar_t * dst)
			{
				HANDLE hIn = CreateFile(src, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				LARGE_INTEGER fileSize;
				GetFileSizeEx(hIn, &fileSize);
				if (fileSize.HighPart > 0 && sizeof(SIZE_T) == 4)
					return;//this means File is too large fo win32(because sizeof(SIZE_T) == 4)
				HANDLE hInMap = CreateFileMapping(hIn, nullptr, PAGE_READONLY, 0, 0, nullptr);
				BYTE* pInFile = (BYTE*)MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);
				HuffManCodeHeader huffheader = { 0 };
				const BYTE* pIn = nullptr;
				BYTE* pOut = nullptr;
				BYTE code[256] = { 0 };
				__try{
					pIn = pInFile;
					pOut = (BYTE*)&huffheader;
					while (pIn < pInFile + sizeof(huffheader)){
						*pOut = *pIn;
						++pIn; ++pOut;
					}
					const static SIZE_T HEAP_SIZE = sizeof(HuffManTreeNode)* HuffManCodeHeader::arraysize*2;
					HANDLE hNode = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, HEAP_SIZE, 0);
					HuffManTreeNode * hufftree = (HuffManTreeNode *)HeapAlloc(hNode, HEAP_ZERO_MEMORY, sizeof(HuffManTreeNode));
					
					auto DecodeHeader = [&]()
					{
						for (size_t i = 0; i != HuffManCodeHeader::arraysize; ++i){
							if (huffheader.length[i] == 0)
								continue;
							BYTE bit;
							HuffManTreeNode * pNode = hufftree;
							for (BYTE j = 0; j != huffheader.length[i]; ++j){
								bit = (huffheader.CODE[i] >> j) & 1;
								if (bit){
									if (pNode->right == nullptr)
										pNode->right = (HuffManTreeNode *)HeapAlloc(hNode, HEAP_ZERO_MEMORY, sizeof(HuffManTreeNode));
									pNode = pNode->right;
								}
								else{
									if (pNode->left == nullptr)
										pNode->left = (HuffManTreeNode *)HeapAlloc(hNode, HEAP_ZERO_MEMORY, sizeof(HuffManTreeNode));
									pNode = pNode->left;
								}
							}
							code[huffheader.CODE[i]] = i;
						}
					};
					DecodeHeader();
					LARGE_INTEGER outfileSize;
					outfileSize.QuadPart = huffheader.filesize;
					HANDLE hOut = CreateFile(dst, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					HANDLE hOutMap = CreateFileMapping(hOut, nullptr, PAGE_READWRITE, outfileSize.HighPart, outfileSize.LowPart, nullptr);
					BYTE* pOutFile = (BYTE*)MapViewOfFile(hOutMap, FILE_MAP_WRITE, 0, 0, (SIZE_T)outfileSize.QuadPart);
					pOut = pOutFile;
					int length = 7;
					std::remove_reference<decltype(huffheader.CODE[0])>::type index = 0;
					BYTE bit = 0;
					BYTE deep = 0;
					HuffManTreeNode * pNode = hufftree;
					bool add = false;
					auto read4bits = [&]()
					{
						index = index >> (sizeof(index)* 8 - deep);
						pNode = hufftree;
						if (add)
						{
							*pOut |= (code[index] << 4);//high 4 bit
							++pOut;
							add = false;
						}
						else
						{
							*pOut |= code[index];//low 4 bit
							add = true;
						}
						index = 0;
						deep = 0;
						++length;
					};
					while (pOut < pOutFile + outfileSize.QuadPart){
						if (length == -1){
							++pIn; length = 7;
						}
						bit = (*pIn) >> length & 1;
						--length;
						if (bit){
							if (!pNode->right){
								read4bits();
							}
							else{
								pNode = pNode->right;
								index = index >> 1 | (1 << (sizeof(index)* 8 - 1));
								deep++;
							}
						}
						else{
							if (!pNode->left){
								read4bits();
							}
							else{
								pNode = pNode->left;
								index = index >> 1;
								deep++;
							}
						}
					}
					HeapDestroy(hNode);
					UnmapViewOfFile(pOutFile);
					CloseHandle(hOutMap);
					CloseHandle(hOut);
				}
				__except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH){
					return;//this means fatal error accessing mapped file
				}
				UnmapViewOfFile(pInFile);
				CloseHandle(hInMap);
				CloseHandle(hIn);
			}
#pragma endregion
		}
	}
}