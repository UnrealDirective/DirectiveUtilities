// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "DirectiveUtilRuntimeHelpers.h"

#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#else
#include <cerrno>
#include <ctime>
#endif

namespace DirectiveUtil
{
	namespace
	{
		bool MakeValidatedDateTime(
			const int32 Year,
			const int32 Month,
			const int32 Day,
			const int32 Hour,
			const int32 Minute,
			const int32 Second,
			const int32 Millisecond,
			const int64 AdditionalTicks,
			FDateTime& OutDateTime)
		{
			if (!FDateTime::Validate(Year, Month, Day, Hour, Minute, Second, Millisecond)
				|| AdditionalTicks < 0 || AdditionalTicks >= ETimespan::TicksPerSecond)
			{
				return false;
			}

			const FDateTime Base(Year, Month, Day, Hour, Minute, Second, Millisecond);
			if (AdditionalTicks > FDateTime::MaxValue().GetTicks() - Base.GetTicks())
			{
				return false;
			}
			OutDateTime = FDateTime(Base.GetTicks() + AdditionalTicks);
			return true;
		}

#if PLATFORM_WINDOWS
		SYSTEMTIME ToSystemTime(const FDateTime& DateTime)
		{
			SYSTEMTIME Result = {};
			Result.wYear = static_cast<WORD>(DateTime.GetYear());
			Result.wMonth = static_cast<WORD>(DateTime.GetMonth());
			Result.wDay = static_cast<WORD>(DateTime.GetDay());
			Result.wHour = static_cast<WORD>(DateTime.GetHour());
			Result.wMinute = static_cast<WORD>(DateTime.GetMinute());
			Result.wSecond = static_cast<WORD>(DateTime.GetSecond());
			Result.wMilliseconds = static_cast<WORD>(DateTime.GetMillisecond());
			return Result;
		}

		bool FromSystemTime(const SYSTEMTIME& SystemTime, const int64 SubMillisecondTicks, FDateTime& OutDateTime)
		{
			return MakeValidatedDateTime(
				SystemTime.wYear,
				SystemTime.wMonth,
				SystemTime.wDay,
				SystemTime.wHour,
				SystemTime.wMinute,
				SystemTime.wSecond,
				SystemTime.wMilliseconds,
				SubMillisecondTicks,
				OutDateTime);
		}

		bool GetTimeZoneForYear(const int32 Year, TIME_ZONE_INFORMATION& OutTimeZone)
		{
			DYNAMIC_TIME_ZONE_INFORMATION DynamicTimeZone = {};
			if (GetDynamicTimeZoneInformation(&DynamicTimeZone) == TIME_ZONE_ID_INVALID)
			{
				return false;
			}

			return GetTimeZoneInformationForYear(
				static_cast<USHORT>(Year),
				&DynamicTimeZone,
				&OutTimeZone) != 0;
		}
#else
		bool FromTm(const tm& Components, const int64 FractionTicks, FDateTime& OutDateTime)
		{
			return MakeValidatedDateTime(
				Components.tm_year + 1900,
				Components.tm_mon + 1,
				Components.tm_mday,
				Components.tm_hour,
				Components.tm_min,
				Components.tm_sec,
				0,
				FractionTicks,
				OutDateTime);
		}
#endif
	}

	FString ResolveRuntimePath(const FString& Path)
	{
		if (Path.IsEmpty())
		{
			return Path;
		}
		// ProjectSavedDir() is itself relative to the binaries directory, so it has to be resolved
		// before it can serve as a base, or the result stays relative.
		return FPaths::ConvertRelativePathToFull(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()), Path);
	}

	bool TryUtcToLocal(const FDateTime& UtcTimestamp, FDateTime& OutLocalDateTime)
	{
#if PLATFORM_WINDOWS
		TIME_ZONE_INFORMATION TimeZone = {};
		if (!GetTimeZoneForYear(UtcTimestamp.GetYear(), TimeZone))
		{
			return false;
		}

		const SYSTEMTIME UtcSystemTime = ToSystemTime(UtcTimestamp);
		SYSTEMTIME LocalSystemTime = {};
		if (!SystemTimeToTzSpecificLocalTime(&TimeZone, &UtcSystemTime, &LocalSystemTime))
		{
			return false;
		}

		const int64 SubMillisecondTicks = UtcTimestamp.GetTicks() % ETimespan::TicksPerMillisecond;
		return FromSystemTime(LocalSystemTime, SubMillisecondTicks, OutLocalDateTime);
#else
		// ToUnixTimestamp truncates toward zero, which rounds pre-1970 instants up a second and
		// leaves the millisecond component describing a different second than the one converted.
		// FMath::DivideAndRoundDown truncates too despite its name, so floor explicitly.
		const int64 TicksSinceEpoch = UtcTimestamp.GetTicks() - FDateTime(1970, 1, 1).GetTicks();
		int64 UnixSeconds = TicksSinceEpoch / ETimespan::TicksPerSecond;
		int64 RemainderTicks = TicksSinceEpoch - UnixSeconds * ETimespan::TicksPerSecond;
		if (RemainderTicks < 0)
		{
			--UnixSeconds;
			RemainderTicks += ETimespan::TicksPerSecond;
		}
		const time_t Time = static_cast<time_t>(UnixSeconds);
		tm LocalTm;
		if (localtime_r(&Time, &LocalTm) == nullptr)
		{
			return false;
		}

		return FromTm(LocalTm, RemainderTicks, OutLocalDateTime);
#endif
	}

	bool TryLocalToUtc(const FDateTime& LocalDateTime, FDateTime& OutUtcDateTime)
	{
#if PLATFORM_WINDOWS
		TIME_ZONE_INFORMATION TimeZone = {};
		if (!GetTimeZoneForYear(LocalDateTime.GetYear(), TimeZone))
		{
			return false;
		}

		const SYSTEMTIME LocalSystemTime = ToSystemTime(LocalDateTime);
		SYSTEMTIME UtcSystemTime = {};
		if (!TzSpecificLocalTimeToSystemTime(&TimeZone, &LocalSystemTime, &UtcSystemTime))
		{
			return false;
		}

		const int64 SubMillisecondTicks = LocalDateTime.GetTicks() % ETimespan::TicksPerMillisecond;
		return FromSystemTime(UtcSystemTime, SubMillisecondTicks, OutUtcDateTime);
#else
		tm LocalTm = {};
		LocalTm.tm_year = LocalDateTime.GetYear() - 1900;
		LocalTm.tm_mon = LocalDateTime.GetMonth() - 1;
		LocalTm.tm_mday = LocalDateTime.GetDay();
		LocalTm.tm_hour = LocalDateTime.GetHour();
		LocalTm.tm_min = LocalDateTime.GetMinute();
		LocalTm.tm_sec = LocalDateTime.GetSecond();
		LocalTm.tm_isdst = -1;

		errno = 0;
		const time_t Time = mktime(&LocalTm);
		if (Time == static_cast<time_t>(-1) && errno != 0)
		{
			return false;
		}

		const int64 FractionTicks = LocalDateTime.GetTicks() % ETimespan::TicksPerSecond;
		const FDateTime UtcDateTime = FDateTime::FromUnixTimestamp(static_cast<int64>(Time)) + FTimespan(FractionTicks);
		if (UtcDateTime < FDateTime::MinValue() || UtcDateTime > FDateTime::MaxValue())
		{
			return false;
		}
		OutUtcDateTime = UtcDateTime;
		return true;
#endif
	}

	FDateTime UtcToLocal(const FDateTime& UtcTimestamp)
	{
		FDateTime LocalDateTime;
		return TryUtcToLocal(UtcTimestamp, LocalDateTime) ? LocalDateTime : UtcTimestamp;
	}

	FDateTime LocalToUtc(const FDateTime& LocalDateTime)
	{
		FDateTime UtcDateTime;
		return TryLocalToUtc(LocalDateTime, UtcDateTime) ? UtcDateTime : LocalDateTime;
	}
}
