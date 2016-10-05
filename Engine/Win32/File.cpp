// 这个代码部分是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
#include "File.h"

#include <LBase/Win32/Mingw32.h>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <winternl.h>
#if LB_IMPL_MSCPP
#pragma warning(disable:4005)
#endif
#include <ntstatus.h>
#undef CreateFile

extern "C" {
	typedef struct _FILE_STANDARD_INFORMATION {
		LARGE_INTEGER AllocationSize;
		LARGE_INTEGER EndOfFile;
		ULONG NumberOfLinks;
		BOOLEAN DeletePending;
		BOOLEAN Directory;
	} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

	typedef struct _FILE_END_OF_FILE_INFORMATION {
		LARGE_INTEGER EndOfFile;
	} FILE_END_OF_FILE_INFORMATION,*PFILE_END_OF_FILE_INFORMATION;

	typedef enum MCF_FILE_INFORMATION_CLASS {
		FileFullDirectoryInformation =2,
		FileBothDirectoryInformation,
		FileBasicInformation,
		FileStandardInformation,
		FileInternalInformation,
		FileEaInformation,
		FileAccessInformation,
		FileNameInformation,
		FileRenameInformation,
		FileLinkInformation,
		FileNamesInformation,
		FileDispositionInformation,
		FilePositionInformation,
		FileFullEaInformation,
		FileModeInformation,
		FileAlignmentInformation,
		FileAllInformation,
		FileAllocationInformation,
		FileEndOfFileInformation,
		FileAlternateNameInformation,
		FileStreamInformation,
		FilePipeInformation,
		FilePipeLocalInformation,
		FilePipeRemoteInformation,
		FileMailslotQueryInformation,
		FileMailslotSetInformation,
		FileCompressionInformation,
		FileObjectIdInformation,
		FileCompletionInformation,
		FileMoveClusterInformation,
		FileQuotaInformation,
		FileReparsePointInformation,
		FileNetworkOpenInformation,
		FileAttributeTagInformation,
		FileTrackingInformation,
		FileIdBothDirectoryInformation,
		FileIdFullDirectoryInformation,
		FileValidDataLengthInformation,
		FileShortNameInformation,
		FileIoCompletionNotificationInformation,
		FileIoStatusBlockRangeInformation,
		FileIoPriorityHintInformation,
		FileSfioReserveInformation,
		FileSfioVolumeInformation,
		FileHardLinkInformation,
		FileProcessIdsUsingFileInformation,
		FileNormalizedNameInformation,
		FileNetworkPhysicalNameInformation,
		FileIdGlobalTxDirectoryInformation,
		FileIsRemoteDeviceInformation,
		FileUnusedInformation,
		FileNumaNodeInformation,
		FileStandardLinkInformation,
		FileRemoteProtocolInformation,
		FileRenameInformationBypassAccessCheck,
		FileLinkInformationBypassAccessCheck,
		FileVolumeNameInformation,
		FileIdInformation,
		FileIdExtdDirectoryInformation,
		FileReplaceCompletionInformation,
		FileHardLinkFullIdInformation,
		FileIdExtdBothDirectoryInformation,
		FileMaximumInformation
	} MCF_FILE_INFORMATION_CLASS, *PMCF_FILE_INFORMATION_CLASS;

	NTSTATUS(WINAPI * NTQUERYINFORMATIONFILE)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, MCF_FILE_INFORMATION_CLASS);

	auto NtQueryInformationFile = reinterpret_cast<decltype(NTQUERYINFORMATIONFILE)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationFile"));

	NTSTATUS(WINAPI * NTSETINFORMATIONFILE)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, MCF_FILE_INFORMATION_CLASS);

	auto NtSetInformationFile = reinterpret_cast<decltype(NTSETINFORMATIONFILE)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSetInformationFile"));

	NTSTATUS(WINAPI *NTOPENDIRECTORYOBJECT)(HANDLE *pHandle, ACCESS_MASK dwDesiredAccess, const OBJECT_ATTRIBUTES *pObjectAttributes) lnoexcept;

	auto NtOpenDirectoryObject = reinterpret_cast<decltype(NTOPENDIRECTORYOBJECT)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenDirectoryObject"));

	typedef enum tagRTL_PATH_TYPE {
		RtlPathTypeUnknown,
		RtlPathTypeUncAbsolute,
		RtlPathTypeDriveAbsolute,
		RtlPathTypeDriveRelative,
		RtlPathTypeRooted,
		RtlPathTypeRelative,
		RtlPathTypeLocalDevice,
		RtlPathTypeRootLocalDevice,
	} RTL_PATH_TYPE;

	NTSTATUS(WINAPI *RTLGETFULLPATHNAME_USTREX)(const UNICODE_STRING *pFileName, UNICODE_STRING *pStaticBuffer, UNICODE_STRING *pDynamicBuffer,
		UNICODE_STRING **ppWhichBufferIsUsed, SIZE_T *puPrefixChars, BOOLEAN *pbValid, RTL_PATH_TYPE *pePathType, SIZE_T *puBytesRequired);

	auto RtlGetFullPathName_UstrEx = reinterpret_cast<decltype(RTLGETFULLPATHNAME_USTREX)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetFullPathName_UstrEx"));

	NTSTATUS(WINAPI *NTREADFILE)(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
		void *pBuffer, ULONG ulLength, const LARGE_INTEGER *pliOffset, const ULONG *pulKey) lnoexcept;

	auto NtReadFile = reinterpret_cast<decltype(NTREADFILE)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadFile"));

	NTSTATUS(WINAPI *NTWRITEFILE)(HANDLE hFile, HANDLE hEvent, PIO_APC_ROUTINE pfnApcRoutine, void *pApcContext, IO_STATUS_BLOCK *pIoStatus,
		const void *pBuffer, ULONG ulLength, const LARGE_INTEGER *pliOffset, const ULONG *pulKey) lnoexcept;

	auto NtWriteFile = reinterpret_cast<decltype(NTWRITEFILE)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWriteFile"));

	NTSTATUS(WINAPI *NTFLUSHBUFFERSFILE)(HANDLE hFile, IO_STATUS_BLOCK *pIoStatus) lnoexcept;

	auto NtFlushBuffersFile = reinterpret_cast<decltype(NTFLUSHBUFFERSFILE)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtFlushBuffersFile"));

	NTSTATUS(WINAPI* NTDELAYEXECUTION)(BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

	auto NtDelayExecution = reinterpret_cast<decltype(NTDELAYEXECUTION)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtDelayExecution"));

	auto MCF_NtCreateFile = reinterpret_cast<decltype(NtCreateFile) *>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtCreateFile"));

	auto MCF_RtlFreeUnicodeString = reinterpret_cast<decltype(RtlFreeUnicodeString) *>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlFreeUnicodeString"));

	auto MCF_RtlNtStatusToDosError = reinterpret_cast<decltype(RtlNtStatusToDosError) *>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlNtStatusToDosError"));
}

namespace platform_ex {
	namespace Windows {
		namespace {
			void WINAPI IoApcCallback(void *pContext, ::IO_STATUS_BLOCK *pIoStatus, ULONG ulReserved) lnoexcept {
				(void)pIoStatus;
				(void)ulReserved;

				const auto pbIoPending = static_cast<bool *>(pContext);
				*pbIoPending = false;
			}
		}

		UniqueNtHandle File::CreateFileWA(const std::experimental::wstring_view& path, uint32 flags) {
			constexpr wchar_t kDosDevicePath[] = { L'\\', L'?', L'?' };

			const auto uSize = path.size() * sizeof(wchar_t);
			if (uSize > USHRT_MAX) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_BUFFER_OVERFLOW, "File: 文件名太长。");
			}

			::UNICODE_STRING ustrRawPath;
			ustrRawPath.Length = (USHORT)uSize;
			ustrRawPath.MaximumLength = (USHORT)uSize;
			ustrRawPath.Buffer = (PWSTR)path.data();

			wchar_t awcStaticStr[MAX_PATH];
			::UNICODE_STRING ustrStaticBuffer;
			ustrStaticBuffer.Length = 0;
			ustrStaticBuffer.MaximumLength = sizeof(awcStaticStr);
			ustrStaticBuffer.Buffer = awcStaticStr;

			::UNICODE_STRING ustrDynamicBuffer;
			ustrDynamicBuffer.Length = 0;
			ustrDynamicBuffer.MaximumLength = 0;
			ustrDynamicBuffer.Buffer = nullptr;
			auto vFreeDynamicBuffer = leo::finally([&]() noexcept { ::MCF_RtlFreeUnicodeString(&ustrDynamicBuffer); });

			::UNICODE_STRING *pustrFullPath;
			UniqueNtHandle hRootDirectory;
			if ((ustrRawPath.Length >= sizeof(kDosDevicePath)) && (std::memcmp(ustrRawPath.Buffer, kDosDevicePath, sizeof(kDosDevicePath)) == 0)) {
				pustrFullPath = &ustrRawPath;
			}
			else {
				::RTL_PATH_TYPE ePathType;
				const auto lPathStatus = ::RtlGetFullPathName_UstrEx(&ustrRawPath, &ustrStaticBuffer, &ustrDynamicBuffer, &pustrFullPath, nullptr, nullptr, &ePathType, nullptr);
				if (!NT_SUCCESS(lPathStatus)) {
					LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lPathStatus), "File: RtlGetFullPathName_UstrEx() 失败。");
				}
				if ((RtlPathTypeDriveAbsolute <= ePathType) && (ePathType <= RtlPathTypeRelative)) {
					::UNICODE_STRING ustrName;
					ustrName.Length = sizeof(kDosDevicePath);
					ustrName.MaximumLength = sizeof(kDosDevicePath);
					ustrName.Buffer = (PWSTR)kDosDevicePath;

					::OBJECT_ATTRIBUTES vObjectAttributes;
					InitializeObjectAttributes(&vObjectAttributes, &ustrName, 0, nullptr, nullptr);

					HANDLE hTemp;
					const auto lStatus = ::NtOpenDirectoryObject(&hTemp, 0x0F, &vObjectAttributes);
					if (!NT_SUCCESS(lStatus)) {
						LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtOpenDirectoryObject() 失败。");
					}
					hRootDirectory.Reset(hTemp);
				}
			}

			::ACCESS_MASK dwDesiredAccess = 0;
			if (flags & kToRead) {
				dwDesiredAccess |= FILE_GENERIC_READ;
			}
			if (flags & kToWrite) {
				dwDesiredAccess |= FILE_GENERIC_WRITE;
			}

			::OBJECT_ATTRIBUTES vObjectAttributes;
			InitializeObjectAttributes(&vObjectAttributes, pustrFullPath, OBJ_CASE_INSENSITIVE, hRootDirectory.Get(), nullptr);

			::IO_STATUS_BLOCK vIoStatus;

			DWORD dwSharedAccess;
			if (flags & kToWrite) {
				dwSharedAccess = 0;
			}
			else {
				dwSharedAccess = FILE_SHARE_READ;
			}
			if (flags & kSharedRead) {
				dwSharedAccess |= FILE_SHARE_READ;
			}
			if (flags & kSharedWrite) {
				dwSharedAccess |= FILE_SHARE_WRITE;
			}
			if (flags & kSharedDelete) {
				dwSharedAccess |= FILE_SHARE_DELETE;
			}

			DWORD dwCreateDisposition;
			if (flags & kToWrite) {
				if (flags & kDontCreate) {
					dwCreateDisposition = FILE_OPEN;
				}
				else if (flags & kFailIfExists) {
					dwCreateDisposition = FILE_CREATE;
				}
				else {
					dwCreateDisposition = FILE_OPEN_IF;
				}
			}
			else {
				dwCreateDisposition = FILE_OPEN;
			}

			DWORD dwCreateOptions = FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS;
			if (flags & kNoBuffering) {
				dwCreateOptions |= FILE_NO_INTERMEDIATE_BUFFERING;
			}
			if (flags & kWriteThrough) {
				dwCreateOptions |= FILE_WRITE_THROUGH;
			}
			if (flags & kDeleteOnClose) {
				dwDesiredAccess |= DELETE;
				dwCreateOptions |= FILE_DELETE_ON_CLOSE;
			}

			HANDLE hTemp;
			const auto lStatus = ::MCF_NtCreateFile(&hTemp, dwDesiredAccess, &vObjectAttributes, &vIoStatus, nullptr, FILE_ATTRIBUTE_NORMAL, dwSharedAccess, dwCreateDisposition, dwCreateOptions, nullptr, 0);
			if (!NT_SUCCESS(lStatus)) {
				LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtCreateFile() 失败。");
			}
			UniqueNtHandle hFile(hTemp);

			return hFile;
		}

		bool File::IsOpen() const lnoexcept {
			return !!file;
		}
		void File::Open(const std::experimental::wstring_view& path, uint32 flags) {
			File(path, flags).Swap(*this);
		}
		bool File::OpenNothrow(const std::experimental::wstring_view& path, uint32 flags) {
			try {
				Open(path, flags);
				return true;
			}
			catch (Win32Exception &e) {
				::SetLastError(e.GetErrorCode());
				return false;
			}
		}
		void File::Close() lnoexcept {
			File().Swap(*this);
		}

		uint64 File::GetSize() const {
			if (!file) {
				return 0;
			}

			::IO_STATUS_BLOCK vIoStatus;
			::FILE_STANDARD_INFORMATION vStandardInfo;
			const auto lStatus = ::NtQueryInformationFile(file.Get(), &vIoStatus, &vStandardInfo, sizeof(vStandardInfo), FileStandardInformation);
			if (!NT_SUCCESS(lStatus)) {
				LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtQueryInformationFile() 失败。");
			}
			return static_cast<uint64>(vStandardInfo.EndOfFile.QuadPart);
		}
		void File::Resize(uint64 u64NewSize) {
			if (!file) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_INVALID_HANDLE, "File: 尚未打开任何文件。");
			}

			if (u64NewSize >= static_cast<uint64>(INT64_MAX)) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_INVALID_PARAMETER, "File: 文件大小过大。");
			}

			::IO_STATUS_BLOCK vIoStatus;
			::FILE_END_OF_FILE_INFORMATION vEofInfo;
			vEofInfo.EndOfFile.QuadPart = static_cast<std::int64_t>(u64NewSize);
			const auto lStatus = ::NtSetInformationFile(file.Get(), &vIoStatus, &vEofInfo, sizeof(vEofInfo), FileEndOfFileInformation);
			if (!NT_SUCCESS(lStatus)) {
				LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtSetInformationFile() 失败。");
			}
		}
		void File::Clear() {
			Resize(0);
		}

		std::size_t File::Read(void *pBuffer, std::size_t uBytesToRead, uint64 u64Offset) const
		{
			if (!file) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_INVALID_HANDLE, "File: 尚未打开任何文件。");
			}

			if (u64Offset >= static_cast<uint64>(INT64_MAX)) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_SEEK, "File: 文件偏移量太大。");
			}

			bool bIoPending = true;
			::IO_STATUS_BLOCK vIoStatus;
			vIoStatus.Information = 0;
			if (uBytesToRead > ULONG_MAX) {
				uBytesToRead = ULONG_MAX;
			}
			::LARGE_INTEGER liOffset;
			liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
			auto lStatus = ::NtReadFile(file.Get(), nullptr, &IoApcCallback, &bIoPending, &vIoStatus, pBuffer, static_cast<ULONG>(uBytesToRead), &liOffset, nullptr);
			
			if (lStatus != STATUS_END_OF_FILE) {
				if (!NT_SUCCESS(lStatus)) {
					LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtReadFile() 失败。");
				}
				do {
					LARGE_INTEGER liTimeout;
					liTimeout.QuadPart = -1;
					lStatus = ::NtDelayExecution(true, &liTimeout);
					lassume(NT_SUCCESS(lStatus));
				} while (bIoPending);
			}

			return vIoStatus.Information;
		}
		std::size_t File::Write(uint64 u64Offset, const void *pBuffer, std::size_t uBytesToWrite)
		{
			if (!file) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_INVALID_HANDLE, "File: 尚未打开任何文件。");
			}

			if (u64Offset >= static_cast<uint64>(INT64_MAX)) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_SEEK, "File: 文件偏移量太大。");
			}

			bool bIoPending = true;
			::IO_STATUS_BLOCK vIoStatus;
			vIoStatus.Information = 0;
			if (uBytesToWrite > ULONG_MAX) {
				uBytesToWrite = ULONG_MAX;
			}
			::LARGE_INTEGER liOffset;
			liOffset.QuadPart = static_cast<std::int64_t>(u64Offset);
			auto lStatus = ::NtWriteFile(file.Get(), nullptr, &IoApcCallback, &bIoPending, &vIoStatus, pBuffer, static_cast<ULONG>(uBytesToWrite), &liOffset, nullptr);

			{
				if (!NT_SUCCESS(lStatus)) {
					LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtWriteFile() 失败。");
				}
				do {
					lStatus = ::NtDelayExecution(true, nullptr);
					lassume(NT_SUCCESS(lStatus));
				} while (bIoPending);
			}

			return vIoStatus.Information;
		}
		void File::HardFlush() {
			if (!file) {
				LFL_Raise_Win32Exception_On_Failure(ERROR_INVALID_HANDLE, "File: 尚未打开任何文件。");
			}

			::IO_STATUS_BLOCK vIoStatus;
			const auto lStatus = ::NtFlushBuffersFile(file.Get(), &vIoStatus);
			if (!NT_SUCCESS(lStatus)) {
				LFL_Raise_Win32Exception_On_Failure(::MCF_RtlNtStatusToDosError(lStatus), "File: NtFlushBuffersFile() 失败。");
			}
		}
	}
}