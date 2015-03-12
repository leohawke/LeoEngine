#include "IndePlatform\platform.h"
#include "IndePlatform\raii.hpp"
#include "IndePlatform\memory.hpp"
#include <algorithm>
#include <type_traits>
#include <exception>
#include <assert.h>

#include "File.hpp"
#include "exception.hpp"
#include "Debug.hpp"

#pragma warning(push)
#pragma warning(disable : 4800)
namespace leo
{
	namespace win
	{
		class FileDelegate : CONCRETE(File) {
		private:
			struct xFileCloser {
				using value_type = HANDLE;
				HANDLE operator()() const lnothrow{
					return INVALID_HANDLE_VALUE;
				}
					void operator()(HANDLE hFile) const lnothrow{
					::CloseHandle(hFile);
				}
			};

			struct xApcResult {
				DWORD dwBytesTransferred;
				DWORD dwErrorCode;
			};

		private:
			static void __stdcall xAioCallback(DWORD dwErrorCode, DWORD dwBytesTransferred, LPOVERLAPPED pOverlapped) lnothrow{
				const auto pApcResult = (xApcResult *)pOverlapped->hEvent;
				pApcResult->dwErrorCode = dwErrorCode;
				pApcResult->dwBytesTransferred = dwBytesTransferred;
			}

		private:
			UniqueHandle<xFileCloser> xm_hFile;

		public:
			std::pair<unsigned long, std::string> Open(const wchar_t *pwszPath, std::uint32_t u32Flags){
				DWORD dwCreateDisposition;
				if (u32Flags & TO_WRITE){
					if (u32Flags & NO_CREATE){
						dwCreateDisposition = OPEN_EXISTING;
					}
					else if (u32Flags & FAIL_IF_EXISTS){
						dwCreateDisposition = CREATE_NEW;
					}
					else {
						dwCreateDisposition = OPEN_ALWAYS;
					}
				}
				else {
					dwCreateDisposition = OPEN_EXISTING;
				}

				DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
				if (u32Flags & NO_BUFFERING){
					dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
				}
				if (u32Flags & WRITE_THROUGH){
					dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
				}
				if (u32Flags & DEL_ON_CLOSE){
					dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
				}

				xm_hFile.Reset(::CreateFileW(
					pwszPath,
					((u32Flags & TO_READ) ? GENERIC_READ : 0) | ((u32Flags & TO_WRITE) ? GENERIC_WRITE : 0),
					(u32Flags & TO_WRITE) ? 0 : FILE_SHARE_READ,
					nullptr, dwCreateDisposition, dwFlagsAndAttributes, NULL
					));
				if (!xm_hFile){
					return std::make_pair(::GetLastError(),"::CreateFileW() 失败。");
				}
				if ((u32Flags & TO_WRITE) && !(u32Flags & NO_TRUNC)){
					if (!::SetEndOfFile(xm_hFile.Get())){
						Raise_Win32_Exception("创建文件时试图截断文件，::SetEndOfFile() 失败。");
					}
					if (!::FlushFileBuffers(xm_hFile.Get())){
						Raise_Win32_Exception("创建文件时试图截断文件，::FlushFileBuffers() 失败。");
					}
				}
				return std::make_pair((unsigned long)ERROR_SUCCESS, std::string());
			}

			std::uint64_t GetSize() const {
				lassume(xm_hFile);

				LARGE_INTEGER liFileSize;
				if (!::GetFileSizeEx(xm_hFile.Get(), &liFileSize)){
					Raise_Win32_Exception("::GetFileSizeEx() 失败。");
				}
				return (std::uint64_t)liFileSize.QuadPart;
			}
			void Resize(std::uint64_t u64NewSize){
				lassume(xm_hFile);

				if (u64NewSize > (std::uint64_t)LLONG_MAX){
					Raise_Error_Exception(ERROR_INVALID_PARAMETER,"调整文件大小时指定的大小无效。");
				}
				LARGE_INTEGER liNewSize;
				liNewSize.QuadPart = (long long)u64NewSize;
				if (!::SetFilePointerEx(xm_hFile.Get(), liNewSize, nullptr, FILE_BEGIN)){
					Raise_Win32_Exception("::SetFilePointerEx() 失败。");
				}
				if (!::SetEndOfFile(xm_hFile.Get())){
					Raise_Win32_Exception("::SetEndOfFile() 失败。");
				}
			}

			std::size_t Read(
				void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
				const std::function<void()> *pfnAsyncProc,
				const std::function<void()> *pfnCompleteCallback
				) const {
				lassume(xm_hFile);

				auto dwBytesToReadThisTime = (DWORD)std::min(0xFFFFF000u, uBytesToRead);

				xApcResult vApcResult;
				vApcResult.dwBytesTransferred = 0;
				vApcResult.dwErrorCode = ERROR_SUCCESS;

				OVERLAPPED vOverlapped;
				BZero(vOverlapped);
				vOverlapped.Offset = static_cast<win::dword>(u64Offset);
				vOverlapped.OffsetHigh = (u64Offset >> 32);
				vOverlapped.hEvent = (HANDLE)&vApcResult;
				const bool bSucceeds = ::ReadFileEx(xm_hFile.Get(), pBuffer, dwBytesToReadThisTime, &vOverlapped, &xAioCallback);
				if (!bSucceeds){
					vApcResult.dwErrorCode = ::GetLastError();
				}
				std::exception_ptr ep;
				if (pfnAsyncProc && *pfnAsyncProc){
					try {
						(*pfnAsyncProc)();
					}
					catch (...){
						ep = std::current_exception();
					}
				}
				if (bSucceeds){
					::SleepEx(INFINITE, TRUE);
				}
				if (ep){
					std::rethrow_exception(ep);
				}
				if (!bSucceeds){
					Raise_Error_Exception(vApcResult.dwErrorCode,"::ReadFileEx() 失败。");
				}
				if (pfnCompleteCallback){
					(*pfnCompleteCallback)();
				}

				std::size_t uBytesRead = vApcResult.dwBytesTransferred;
				while ((uBytesRead < uBytesToRead) && (vApcResult.dwBytesTransferred == dwBytesToReadThisTime)){
					dwBytesToReadThisTime = std::min(0xFFFFF000u, uBytesToRead - uBytesRead);

					BZero(vOverlapped);
					const auto u64NewOffset = u64Offset + uBytesRead;
					vOverlapped.Offset = static_cast<win::dword>(u64NewOffset);
					vOverlapped.OffsetHigh = (u64NewOffset >> 32);
					vOverlapped.hEvent = (HANDLE)&vApcResult;
					if (!::ReadFileEx(
						xm_hFile.Get(), (unsigned char *)pBuffer + uBytesRead, dwBytesToReadThisTime,
						&vOverlapped, &xAioCallback))
					{
						Raise_Error_Exception(vApcResult.dwErrorCode,"::ReadFileEx() 失败。");
					}
					::SleepEx(INFINITE, TRUE);

					if (vApcResult.dwErrorCode != ERROR_SUCCESS){
						if (vApcResult.dwErrorCode == ERROR_HANDLE_EOF){
							break;
						}
						Raise_Error_Exception(vApcResult.dwErrorCode,"::ReadFileEx() 失败。");
					}
					uBytesRead += vApcResult.dwBytesTransferred;
				}
				return uBytesRead;
			}
			void Write(
				std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
				const std::function<void()> *pfnAsyncProc,
				const std::function<void()> *pfnCompleteCallback
				){
				lassume(xm_hFile);

				auto dwBytesToWriteThisTime = (DWORD)std::min(0xFFFFF000u, uBytesToWrite);

				xApcResult vApcResult;
				vApcResult.dwBytesTransferred = 0;
				vApcResult.dwErrorCode = ERROR_SUCCESS;

				OVERLAPPED vOverlapped;
				BZero(vOverlapped);
				vOverlapped.Offset = (DWORD)u64Offset;
				vOverlapped.OffsetHigh = (DWORD)(u64Offset >> 32);
				vOverlapped.hEvent = (HANDLE)&vApcResult;
				const bool bSucceeds = ::WriteFileEx(xm_hFile.Get(), pBuffer, dwBytesToWriteThisTime, &vOverlapped, &xAioCallback);
				if (!bSucceeds){
					vApcResult.dwErrorCode = ::GetLastError();
				}
				std::exception_ptr ep;
				if (pfnAsyncProc && *pfnAsyncProc){
					try {
						(*pfnAsyncProc)();
					}
					catch (...){
						ep = std::current_exception();
					}
				}
				if (bSucceeds){
					::SleepEx(INFINITE, TRUE);
				}
				if (ep){
					std::rethrow_exception(ep);
				}
				if (!bSucceeds){
					Raise_Error_Exception(vApcResult.dwErrorCode,"::WriteFileEx() 失败。");
				}
				if (pfnCompleteCallback){
					(*pfnCompleteCallback)();
				}

				std::size_t uBytesWritten = vApcResult.dwBytesTransferred;
				while ((uBytesWritten < uBytesToWrite) && (vApcResult.dwBytesTransferred == dwBytesToWriteThisTime)){
					dwBytesToWriteThisTime = std::min(0xFFFFF000u, uBytesToWrite - uBytesWritten);

					BZero(vOverlapped);
					const auto u64NewOffset = u64Offset + uBytesWritten;
					vOverlapped.Offset = (DWORD)u64NewOffset;
					vOverlapped.OffsetHigh = (DWORD)(u64NewOffset >> 32);
					vOverlapped.hEvent = (HANDLE)&vApcResult;
					if (!::WriteFileEx(
						xm_hFile.Get(), (const unsigned char *)pBuffer + uBytesWritten, dwBytesToWriteThisTime,
						&vOverlapped, &xAioCallback))
					{
						Raise_Error_Exception(vApcResult.dwErrorCode,"::WriteFileEx() 失败。");
					}
					::SleepEx(INFINITE, TRUE);

					if (vApcResult.dwErrorCode != ERROR_SUCCESS){
						Raise_Error_Exception(vApcResult.dwErrorCode,"::WriteFileEx() 失败。");
					}
					uBytesWritten += vApcResult.dwBytesTransferred;
				}
			}

			void Flush() const {
				lassume(xm_hFile);

				if (!::FlushFileBuffers(xm_hFile.Get())){
					Raise_Win32_Exception("::FlushFileBuffers() 失败。");
				}
			}
		};

		// 静态成员函数。
		std::unique_ptr<File> File::Open(const std::wstring &wsoPath, std::uint32_t u32Flags){
			auto pFile = std::make_unique<FileDelegate>();
			const auto vResult = pFile->Open(wsoPath.c_str(), u32Flags);
			if (vResult.first != ERROR_SUCCESS){
				Raise_Error_Exception(vResult.first, vResult.second);
			}
			return std::move(pFile);
		}

		std::unique_ptr<File> File::OpenNoThrow(const std::wstring &wcsPath, std::uint32_t u32Flags){
			auto pFile = std::make_unique<FileDelegate>();
			const auto vResult = pFile->Open(wcsPath.c_str(), u32Flags);
			if (vResult.first != ERROR_SUCCESS){
				::SetLastError(vResult.first);
				return nullptr;
			}
			return std::move(pFile);
		}

		// 其他非静态成员函数。
		std::uint64_t File::GetSize() const {
			lassume(dynamic_cast<const FileDelegate *>(this));

			return ((const FileDelegate *)this)->GetSize();
		}
		void File::Resize(std::uint64_t u64NewSize){
			lassume(dynamic_cast<FileDelegate *>(this));

			static_cast<FileDelegate *>(this)->Resize(u64NewSize);
		}
		void File::Clear(){
			Resize(0);
			Flush();
		}

		std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const {
			lassume(dynamic_cast<const FileDelegate *>(this));

			return ((const FileDelegate *)this)->Read(
				pBuffer, uBytesToRead, u64Offset, nullptr, nullptr
				);
		}
		void File::Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite){
			lassume(dynamic_cast<FileDelegate *>(this));

			static_cast<FileDelegate *>(this)->Write(
				u64Offset, pBuffer, uBytesToWrite, nullptr, nullptr
				);
		}

		std::size_t File::Read(
			void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
			const std::function<void()> &fnAsyncProc,
			const std::function<void()> &fnCompleteCallback
			) const {
			lassume(dynamic_cast<const FileDelegate *>(this));

			return ((const FileDelegate *)this)->Read(
				pBuffer, uBytesToRead, u64Offset, &fnAsyncProc, &fnCompleteCallback
				);
		}
		void File::Write(
			std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
			const std::function<void()> &fnAsyncProc,
			const std::function<void()> &fnCompleteCallback
			){
			lassume(dynamic_cast<FileDelegate *>(this));

			static_cast<FileDelegate *>(this)->Write(
				u64Offset, pBuffer, uBytesToWrite, &fnAsyncProc, &fnCompleteCallback
				);
		}

		void File::Flush() const {
			lassume(dynamic_cast<const FileDelegate *>(this));

			static_cast<const FileDelegate *>(this)->Flush();
		}

	}


	namespace win
	{
		namespace file
		{
			bool FileExist(const wchar_t * filename)
			{
				assert(filename);
				DWORD fileAttr;
				fileAttr = GetFileAttributes(filename);
				if (fileAttr == INVALID_FILE_ATTRIBUTES)
					return false;
				return true;
			}
			std::wstring GetDirectoryFromFileName(const wchar_t * filename)
			{
				assert(filename);
				std::wstring filepath(filename);
				size_t idx = filepath.rfind(L'\\');
				if (idx != std::wstring::npos)
					return filepath.substr(0, idx + 1);
				return std::wstring();
			}
			std::wstring GetFileNameWithoutExt(const wchar_t * filename)
			{
				assert(filename);
				std::wstring filepath(filename);
				size_t idx = filepath.rfind(L'.');
				if (idx != std::wstring::npos)
					return filepath.substr(0, idx);
				return std::wstring();
			}
			FILE_TYPE GetFileExt(const wchar_t* filename)
			{
				wchar_t ext[_MAX_EXT];
				_wsplitpath_s(filename, nullptr, 0, nullptr, 0, nullptr, 0, ext, _MAX_EXT);
				FILE_TYPE type = FILE_TYPE::OTHER_TEX_END;
				if (_wcsicmp(ext, L".dds") == 0)
				{
					return FILE_TYPE::DDS;
				}
				if (_wcsicmp(ext, L".tga") == 0)
				{
					return FILE_TYPE::DDS;
				}
				return type;
			}

			bool FileExist(const std::wstring& filename)
			{
				return FileExist(filename.c_str());
			}
			std::wstring GetDirectoryFromFileName(const std::wstring& filename)
			{
				size_t idx = filename.rfind(L'\\');
				if (idx != std::wstring::npos)
					return filename.substr(0, idx + 1);
				return std::wstring();
			}
			std::wstring GetFileNameWithoutExt(const std::wstring& filename)
			{
				size_t idx = filename.rfind(L'.');
				if (idx != std::wstring::npos)
					return filename.substr(0, idx);
				return std::wstring();
			}
			FILE_TYPE GetFileExt(const std::wstring& filename)
			{
				return GetFileExt(filename.c_str());
			}
#pragma region TempFile
			wchar_t TempFile::mPath[max_path] = L".\0";
			wchar_t TempFile::mPrefix[TempFile::max_prefix] = L"temp\0";
			TempFile::TempFile()
			{
				::GetTempFileName(mPath, mPrefix, 0, mTempFileName);
			}
			TempFile::TempFile(const std::wstring & path, const std::wstring & prefix)
			{
				::GetTempFileName(path.c_str(), prefix.c_str(), 0, mTempFileName);
			}
			TempFile::TempFile(const wchar_t * path, const wchar_t * prefix)
			{
				::GetTempFileName(path, prefix, 0, mTempFileName);
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
				::lstrcpyW(mPrefix, prefix);
			}
#pragma endregion
#pragma region HuffManCode
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
			class unique_handle : public unique_res<void*>
			{
			public:
				unique_handle(pointer_type poi, _CX che = kernel_check<value_type>, _DX del = kernel_delete<value_type>)
					:unique_res(poi, che, del)
				{}
				operator void*()
				{
					return get();
				}
			};
			struct HuffManCodeHeader
			{
				const static BYTE arraysize = 16;
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
			size_t EncodeHeader(size_t weight[HuffManCodeHeader::arraysize], HuffManCodeHeader& header)
			{
				auto heap_check = [](HANDLE hHeap){return hHeap != nullptr; };
				auto heap_delete = [](HANDLE hHeap){HeapDestroy(hHeap); };
				const static SIZE_T HEAP_SIZE = sizeof(HuffManTreeNode)* HuffManCodeHeader::arraysize * 2;
				unique_handle hHeap(HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, HEAP_SIZE, 0), heap_check, heap_delete);
				if (!hHeap.is_good())
				{
					DebugLastError(); throw;
				}
				assert(true);
				int z = 0;
				HuffManTreeNode * huffnodes = (HuffManTreeNode *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, HEAP_SIZE);
				size_t n = 0;
				for (size_t i = 0; i != HuffManCodeHeader::arraysize; ++i){
					huffnodes[i].weight = weight[i];
					if (weight[i])
						++n;
				}
				huffnodes[HuffManCodeHeader::arraysize * 2 - 1].weight = -1;
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
				wpl = (wpl + 7) / 8;
				return wpl;
			}
			void HuffManEncode(wchar_t * src, wchar_t * dst)
			{
				unique_handle hIn(CreateFile(src, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
				if (!hIn.is_good())
					Raise_Win32_Exception();
				LARGE_INTEGER fileSize;
				GetFileSizeEx(hIn, &fileSize);
				if (fileSize.HighPart > 0 && sizeof(SIZE_T) == 4)
					return;//this means File is too large fo win32(because sizeof(SIZE_T) == 4)
				unique_handle hInMap(CreateFileMapping(hIn, nullptr, PAGE_READONLY, 0, 0, nullptr));
				if (!hInMap.is_good())
					Raise_Win32_Exception();
				BYTE* pInFile = (BYTE*)MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);
				size_t weight[HuffManCodeHeader::arraysize] = { 0 };
				const BYTE* pIn = nullptr;
				try{
					pIn = pInFile;
					size_t Low, High;
					while (pIn < pInFile + fileSize.QuadPart){
						Low = (*pIn) & 0x0F;
						High = ((*pIn) & 0xF0) >> 4;
						++weight[Low];
						++weight[High];
						++pIn;
					}
				}
				catch (std::exception& e)
				{
					//warning C4101
					e;
					DebugPrintf(L"%s", e.what());
				}
				UnmapViewOfFile(pInFile);
				HuffManCodeHeader huffheader = { 0 };
				huffheader.filesize = (size_t)fileSize.QuadPart;
				size_t WPl = EncodeHeader(weight, huffheader);
				pInFile = (BYTE*)MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);

				LARGE_INTEGER outfileSize;
				outfileSize.QuadPart = WPl + sizeof(huffheader);
				unique_handle hOut(CreateFile(dst, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
				if (!hOut.is_good())
					Raise_Win32_Exception();
				unique_handle hOutMap(CreateFileMapping(hOut, nullptr, PAGE_READWRITE, outfileSize.HighPart, outfileSize.LowPart, nullptr));
				if (!hOutMap.is_good())
					Raise_Win32_Exception();
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
					//warning C4101
					e;
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
					const static SIZE_T HEAP_SIZE = sizeof(HuffManTreeNode)* HuffManCodeHeader::arraysize * 2;
					HANDLE hNode = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, HEAP_SIZE, 0);
					HuffManTreeNode * hufftree = (HuffManTreeNode *)HeapAlloc(hNode, HEAP_ZERO_MEMORY, sizeof(HuffManTreeNode));

					auto DecodeHeader = [&]()
					{
						for (BYTE i = 0; i != HuffManCodeHeader::arraysize; ++i){
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
#pragma warning(pop)