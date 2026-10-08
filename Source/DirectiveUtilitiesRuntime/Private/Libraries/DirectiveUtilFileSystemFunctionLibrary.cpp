// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"

#include "DirectiveUtilRuntimeHelpers.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Serialization/Archive.h"
#include "Templates/UniquePtr.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#else
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace
{
	bool IsUsablePath(const FString& Path)
	{
		return !Path.IsEmpty() && Path.Len() == FCString::Strlen(*Path);
	}

	bool EnsureParentDirectory(const FString& ResolvedPath, const bool bCreateDirectories)
	{
		const FString Directory = FPaths::GetPath(ResolvedPath);
		if (Directory.IsEmpty() || IFileManager::Get().DirectoryExists(*Directory))
		{
			return true;
		}
		// The writer creates missing directories on its own retry path, so an inert flag
		// would silently ignore the caller's request not to.
		return bCreateDirectories && IFileManager::Get().MakeDirectory(*Directory, /*Tree*/ true);
	}

#if PLATFORM_WINDOWS
	FString ToWindowsApiPath(const FString& Path)
	{
		if (Path.StartsWith(TEXT("\\\\?\\")))
		{
			return Path;
		}

		FString WindowsPath = Path;
		FPaths::NormalizeFilename(WindowsPath);
		const bool bIsUncPath = WindowsPath.StartsWith(TEXT("//"));
		FPaths::RemoveDuplicateSlashes(WindowsPath);
		if (bIsUncPath)
		{
			WindowsPath.InsertAt(0, TEXT('/'));
		}
		WindowsPath.ReplaceCharInline(TEXT('/'), TEXT('\\'));

		// Matches FWindowsPlatformFile::NormalizeWindowsPath so MoveFileExW accepts the paths the engine wrote.
		if (WindowsPath.Len() >= MAX_PATH)
		{
			return bIsUncPath
				? FString(TEXT("\\\\?\\UNC")) + WindowsPath.RightChop(1)
				: FString(TEXT("\\\\?\\")) + WindowsPath;
		}
		return WindowsPath;
	}

	bool MoveFileWithRetry(const FString& From, const FString& To, const DWORD Flags)
	{
		constexpr int32 MaximumAttempts = 5;
		for (int32 Attempt = 1; Attempt <= MaximumAttempts; ++Attempt)
		{
			if (MoveFileExW(*From, *To, Flags) != 0)
			{
				return true;
			}
			// Antivirus scanners and search indexers can hold a newly written file open briefly.
			const DWORD Error = GetLastError();
			if (Error != ERROR_ACCESS_DENIED && Error != ERROR_SHARING_VIOLATION)
			{
				return false;
			}
			if (Attempt < MaximumAttempts)
			{
				FPlatformProcess::Sleep(0.01f * static_cast<float>(Attempt));
			}
		}
		return false;
	}
#else
	bool IsHardLinkUnsupported(const int Error)
	{
		if (Error == EPERM || Error == ENOTSUP || Error == ENOSYS || Error == EXDEV)
		{
			return true;
		}
#if EOPNOTSUPP != ENOTSUP
		return Error == EOPNOTSUPP;
#else
		return false;
#endif
	}

	bool InstallOverExclusivePlaceholder(const char* TemporaryPath, const char* DestinationPath)
	{
		const int Placeholder = open(DestinationPath, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR);
		if (Placeholder < 0)
		{
			return false;
		}
		struct stat PlaceholderInfo;
		const bool bHasPlaceholderInfo = fstat(Placeholder, &PlaceholderInfo) == 0;
		close(Placeholder);
		if (rename(TemporaryPath, DestinationPath) == 0)
		{
			return true;
		}

		struct stat DestinationInfo;
		if (bHasPlaceholderInfo && lstat(DestinationPath, &DestinationInfo) == 0
			&& DestinationInfo.st_dev == PlaceholderInfo.st_dev && DestinationInfo.st_ino == PlaceholderInfo.st_ino)
		{
			unlink(DestinationPath);
		}
		return false;
	}
#endif

	bool ReplaceWithTemporaryFile(const FString& TemporaryPath, const FString& DestinationPath, const bool bAllowOverwrite)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const FString PhysicalTemporaryPath = PlatformFile.ConvertToAbsolutePathForExternalAppForWrite(*TemporaryPath);
		const FString PhysicalDestinationPath = PlatformFile.ConvertToAbsolutePathForExternalAppForWrite(*DestinationPath);
#if PLATFORM_WINDOWS
		const DWORD Flags = MOVEFILE_WRITE_THROUGH | (bAllowOverwrite ? MOVEFILE_REPLACE_EXISTING : 0);
		return MoveFileWithRetry(ToWindowsApiPath(PhysicalTemporaryPath), ToWindowsApiPath(PhysicalDestinationPath), Flags);
#else
		const FTCHARToUTF8 TemporaryUtf8(*PhysicalTemporaryPath);
		const FTCHARToUTF8 DestinationUtf8(*PhysicalDestinationPath);
		if (bAllowOverwrite)
		{
			return rename(TemporaryUtf8.Get(), DestinationUtf8.Get()) == 0;
		}
		if (link(TemporaryUtf8.Get(), DestinationUtf8.Get()) != 0)
		{
			// FAT, exFAT, and some network shares have no hard links.
			return IsHardLinkUnsupported(errno)
				&& InstallOverExclusivePlaceholder(TemporaryUtf8.Get(), DestinationUtf8.Get());
		}
		if (unlink(TemporaryUtf8.Get()) != 0)
		{
			IFileManager::Get().Delete(*TemporaryPath, false, true, true);
		}
		return true;
#endif
	}

	enum class EAppendedTextEncoding : uint8
	{
		Utf8,
		Utf16LittleEndian,
		Utf16BigEndian
	};

	bool TryGetAppendedTextEncoding(const FString& ResolvedPath, EAppendedTextEncoding& OutEncoding)
	{
		OutEncoding = EAppendedTextEncoding::Utf8;
		const TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*ResolvedPath, FILEREAD_Silent));
		if (!Reader)
		{
			return !IFileManager::Get().FileExists(*ResolvedPath);
		}
		if (Reader->TotalSize() < 2)
		{
			return true;
		}
		const int64 FileSize = Reader->TotalSize();
		uint8 Header[4] = {};
		Reader->Serialize(Header, FMath::Min<int64>(FileSize, sizeof(Header)));
		if (Reader->IsError())
		{
			return false;
		}

		const bool bUtf32LittleEndian = FileSize >= 4 && Header[0] == 0xFF && Header[1] == 0xFE && Header[2] == 0x00 && Header[3] == 0x00;
		const bool bUtf32BigEndian = FileSize >= 4 && Header[0] == 0x00 && Header[1] == 0x00 && Header[2] == 0xFE && Header[3] == 0xFF;
		if (bUtf32LittleEndian || bUtf32BigEndian)
		{
			return false;
		}
		if (Header[0] == 0xFF && Header[1] == 0xFE)
		{
			OutEncoding = EAppendedTextEncoding::Utf16LittleEndian;
		}
		else if (Header[0] == 0xFE && Header[1] == 0xFF)
		{
			OutEncoding = EAppendedTextEncoding::Utf16BigEndian;
		}
		// An odd-sized UTF-16 file ends in half a code unit, so appended text would be misaligned.
		return OutEncoding == EAppendedTextEncoding::Utf8 || FileSize % 2 == 0;
	}

	template <typename CharType>
	bool ConvertText(const FString& Text, TArray<CharType>& OutCharacters)
	{
		const int32 RequiredLength = FPlatformString::ConvertedLength<CharType>(*Text, Text.Len());
		OutCharacters.SetNumUninitialized(RequiredLength);
		return RequiredLength == 0
			|| FPlatformString::Convert(OutCharacters.GetData(), RequiredLength, *Text, Text.Len()) != nullptr;
	}

	bool EncodeAppendedText(const FString& Text, const EAppendedTextEncoding Encoding, TArray<uint8>& OutBytes)
	{
		OutBytes.Reset();
		if (Encoding == EAppendedTextEncoding::Utf8)
		{
			TArray<UTF8CHAR> Utf8;
			if (!ConvertText(Text, Utf8))
			{
				return false;
			}
			OutBytes.Append(reinterpret_cast<const uint8*>(Utf8.GetData()), Utf8.Num());
			return true;
		}

		TArray<UTF16CHAR> Utf16;
		if (!ConvertText(Text, Utf16))
		{
			return false;
		}
		const bool bBigEndian = Encoding == EAppendedTextEncoding::Utf16BigEndian;
		OutBytes.Reserve(Utf16.Num() * 2);
		for (const UTF16CHAR CodeUnit : Utf16)
		{
			const uint8 LowByte = static_cast<uint8>(CodeUnit & 0xFF);
			const uint8 HighByte = static_cast<uint8>((CodeUnit >> 8) & 0xFF);
			OutBytes.Add(bBigEndian ? HighByte : LowByte);
			OutBytes.Add(bBigEndian ? LowByte : HighByte);
		}
		return true;
	}

	template <typename WriteFunction>
	bool WriteFileAtomic(const FString& Path, const bool bCreateDirectories, const bool bAllowOverwrite,
		WriteFunction&& WriteTemporaryFile)
	{
		if (!IsUsablePath(Path))
		{
			return false;
		}

		IFileManager& FileManager = IFileManager::Get();
		const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
		if (!bAllowOverwrite && FileManager.FileExists(*ResolvedPath))
		{
			return false;
		}
		if (!EnsureParentDirectory(ResolvedPath, bCreateDirectories))
		{
			return false;
		}

		const FString TemporaryPath = ResolvedPath + TEXT(".tmp-") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
		if (!WriteTemporaryFile(TemporaryPath))
		{
			FileManager.Delete(*TemporaryPath, false, true, true);
			return false;
		}
		if (!ReplaceWithTemporaryFile(TemporaryPath, ResolvedPath, bAllowOverwrite))
		{
			FileManager.Delete(*TemporaryPath, false, true, true);
			return false;
		}
		return true;
	}
}

bool UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(const FString& Path, FString& OutContents)
{
	if (!IsUsablePath(Path))
	{
		OutContents.Reset();
		return false;
	}

	const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	// FString is int32-indexed, so a file at or past 2 GB would decode into a truncated buffer.
	if (IFileManager::Get().FileSize(*ResolvedPath) >= MAX_int32)
	{
		OutContents.Reset();
		return false;
	}
	FString Contents;
	if (!FFileHelper::LoadFileToString(Contents, *ResolvedPath))
	{
		OutContents.Reset();
		return false;
	}
	OutContents = MoveTemp(Contents);
	return true;
}

bool UDirectiveUtilFileSystemFunctionLibrary::ReadTextFileLines(const FString& Path, TArray<FString>& OutLines, const bool bIncludeEmptyLines)
{
	OutLines.Reset();
	FString Contents;
	if (!ReadTextFile(Path, Contents))
	{
		return false;
	}

	TArray<FString> RawLines;
	Contents.ParseIntoArrayLines(RawLines, false);
	if (RawLines.Num() > 0 && RawLines.Last().IsEmpty())
	{
		RawLines.Pop(EAllowShrinking::No);
	}
	for (const FString& Line : RawLines)
	{
		if (!Line.IsEmpty() || bIncludeEmptyLines)
		{
			OutLines.Add(Line);
		}
	}
	return true;
}

bool UDirectiveUtilFileSystemFunctionLibrary::WriteTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories, const bool bAllowOverwrite)
{
	if (!IsUsablePath(Path))
	{
		return false;
	}
	if (!bAllowOverwrite)
	{
		return WriteTextFileAtomic(Path, Contents, bCreateDirectories, false);
	}

	const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	if (!EnsureParentDirectory(ResolvedPath, bCreateDirectories))
	{
		return false;
	}
	return FFileHelper::SaveStringToFile(
		Contents, *ResolvedPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool UDirectiveUtilFileSystemFunctionLibrary::AppendTextFile(const FString& Path, const FString& Contents, const bool bCreateDirectories)
{
	if (!IsUsablePath(Path))
	{
		return false;
	}

	const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	if (!EnsureParentDirectory(ResolvedPath, bCreateDirectories))
	{
		return false;
	}
	EAppendedTextEncoding Encoding = EAppendedTextEncoding::Utf8;
	TArray<uint8> Bytes;
	if (!TryGetAppendedTextEncoding(ResolvedPath, Encoding) || !EncodeAppendedText(Contents, Encoding, Bytes))
	{
		return false;
	}
	const TUniquePtr<FArchive> Writer(IFileManager::Get().CreateFileWriter(*ResolvedPath, FILEWRITE_Append));
	if (!Writer)
	{
		return false;
	}
	if (Bytes.Num() > 0)
	{
		Writer->Serialize(Bytes.GetData(), Bytes.Num());
	}
	return Writer->Close();
}

bool UDirectiveUtilFileSystemFunctionLibrary::ReadBinaryFile(const FString& Path, TArray<uint8>& OutBytes)
{
	if (!IsUsablePath(Path))
	{
		OutBytes.Reset();
		return false;
	}
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *DirectiveUtil::ResolveRuntimePath(Path)))
	{
		OutBytes.Reset();
		return false;
	}
	OutBytes = MoveTemp(Bytes);
	return true;
}

bool UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFile(const FString& Path, const TArray<uint8>& Bytes, const bool bCreateDirectories, const bool bAllowOverwrite)
{
	if (!IsUsablePath(Path))
	{
		return false;
	}
	if (!bAllowOverwrite)
	{
		return WriteBinaryFileAtomic(Path, Bytes, bCreateDirectories, false);
	}

	const FString ResolvedPath = DirectiveUtil::ResolveRuntimePath(Path);
	if (!EnsureParentDirectory(ResolvedPath, bCreateDirectories))
	{
		return false;
	}
	return FFileHelper::SaveArrayToFile(Bytes, *ResolvedPath);
}

bool UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(const FString& Path, const FString& Contents,
	const bool bCreateDirectories, const bool bAllowOverwrite)
{
	return WriteFileAtomic(Path, bCreateDirectories, bAllowOverwrite, [&](const FString& TemporaryPath)
	{
		return FFileHelper::SaveStringToFile(
			Contents, *TemporaryPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
			&IFileManager::Get(), FILEWRITE_NoReplaceExisting);
	});
}

bool UDirectiveUtilFileSystemFunctionLibrary::WriteBinaryFileAtomic(const FString& Path,
	const TArray<uint8>& Bytes, const bool bCreateDirectories, const bool bAllowOverwrite)
{
	return WriteFileAtomic(Path, bCreateDirectories, bAllowOverwrite, [&](const FString& TemporaryPath)
	{
		return FFileHelper::SaveArrayToFile(
			Bytes, *TemporaryPath, &IFileManager::Get(), FILEWRITE_NoReplaceExisting);
	});
}

int64 UDirectiveUtilFileSystemFunctionLibrary::GetFileSize(const FString& Path)
{
	if (!IsUsablePath(Path))
	{
		return -1;
	}
	return IFileManager::Get().FileSize(*DirectiveUtil::ResolveRuntimePath(Path));
}

bool UDirectiveUtilFileSystemFunctionLibrary::GetFileTimeStamp(const FString& Path, FDateTime& OutTimestamp)
{
	OutTimestamp = FDateTime();
	if (!IsUsablePath(Path))
	{
		return false;
	}

	const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*DirectiveUtil::ResolveRuntimePath(Path));
	if (Timestamp == FDateTime::MinValue())
	{
		return false;
	}

	OutTimestamp = DirectiveUtil::UtcToLocal(Timestamp);
	return true;
}
