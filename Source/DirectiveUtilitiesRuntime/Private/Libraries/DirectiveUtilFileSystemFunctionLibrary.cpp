// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilFileSystemFunctionLibrary.h"
#include "DirectiveUtilRuntimeHelpers.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#else
#include <cstdio>
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

	bool ReplaceWithTemporaryFile(const FString& TemporaryPath, const FString& DestinationPath, const bool bAllowOverwrite)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		const FString PhysicalTemporaryPath = PlatformFile.ConvertToAbsolutePathForExternalAppForWrite(*TemporaryPath);
		const FString PhysicalDestinationPath = PlatformFile.ConvertToAbsolutePathForExternalAppForWrite(*DestinationPath);
#if PLATFORM_WINDOWS
		const DWORD Flags = MOVEFILE_WRITE_THROUGH | (bAllowOverwrite ? MOVEFILE_REPLACE_EXISTING : 0);
		return MoveFileExW(*PhysicalTemporaryPath, *PhysicalDestinationPath, Flags) != 0;
#else
		const FTCHARToUTF8 TemporaryUtf8(*PhysicalTemporaryPath);
		const FTCHARToUTF8 DestinationUtf8(*PhysicalDestinationPath);
		if (bAllowOverwrite)
		{
			return rename(TemporaryUtf8.Get(), DestinationUtf8.Get()) == 0;
		}
		if (link(TemporaryUtf8.Get(), DestinationUtf8.Get()) != 0)
		{
			return false;
		}
		if (unlink(TemporaryUtf8.Get()) != 0)
		{
			IFileManager::Get().Delete(*TemporaryPath, false, true, true);
		}
		return true;
#endif
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
	FString ExistingContents;
	if (IFileManager::Get().FileExists(*ResolvedPath)
		&& !UDirectiveUtilFileSystemFunctionLibrary::ReadTextFile(ResolvedPath, ExistingContents))
	{
		return false;
	}
	ExistingContents += Contents;
	return UDirectiveUtilFileSystemFunctionLibrary::WriteTextFileAtomic(
		ResolvedPath, ExistingContents, bCreateDirectories, true);
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
