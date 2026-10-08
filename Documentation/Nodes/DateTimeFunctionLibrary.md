# Date Time Function Library

> Custom pattern formatting and parsing, local-time to UTC conversion, and human-readable duration formatting.

A pattern is literal text mixed with these tokens:

| Token | Meaning | Example |
|-------|---------|---------|
| yyyy  | Four-digit year | 2026 |
| MM    | Two-digit month | 08 |
| dd    | Two-digit day of month | 22 |
| HH    | Two-digit hour, 00 to 23 | 09 |
| mm    | Two-digit minute | 05 |
| ss    | Two-digit second | 03 |
| fff   | Three-digit millisecond | 412 |

Digits, punctuation, spaces, and non-ASCII characters outside single quotes are emitted verbatim. An unquoted ASCII letter that is not part of a token is an error for both formatting and parsing, so `YYYY-MM-DD` returns false instead of printing `YYYY-08-DD`. A run of token letters with the wrong length, such as `mmm`, is also an error. Wrap letters in single quotes to emit them literally (`yyyy-MM-dd'T'HH:mm`); two consecutive quotes emit one quote character.

The engine covers `FDateTime` construction, arithmetic, component getters, ISO-string conversion, and Unix timestamps; this library fills the remaining gaps without duplicating those nodes.

**Module:** `DirectiveUtilitiesRuntime (Runtime)` &nbsp;|&nbsp; **Header:** `Source/DirectiveUtilitiesRuntime/Public/Libraries/DirectiveUtilDateTimeFunctionLibrary.h`

---

## Format Date Time
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|DateTime`

```cpp
static bool FormatDateTime(const FDateTime& DateTime, const FString& Pattern, FString& OutText);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| DateTime | `const FDateTime&` | The date and time to format. |
| Pattern | `const FString&` | The pattern describing the output. |
| OutText | `FString&` | [out] The formatted text, or an empty string on failure. |

**Returns:** True when the pattern was valid.

## Try Parse Date Time
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|DateTime`

```cpp
static bool TryParseDateTime(const FString& Text, const FString& Pattern, FDateTime& OutDateTime);
```

Parses a date written in a custom pattern. The whole text must match the whole pattern; extra characters and conflicting repeated fields are errors. Literal text must match case exactly, so the pattern `yyyy'T'MM` accepts `2026T08` and rejects `2026t08`. Components omitted from the pattern default to year 1, month 1, day 1, and midnight.

| Parameter | Type | Description |
|-----------|------|-------------|
| Text | `const FString&` | The text to parse. |
| Pattern | `const FString&` | The pattern describing the text. |
| OutDateTime | `FDateTime&` | [out] The parsed value, or a default value on failure. |

**Returns:** True when the text matched the pattern and held a valid calendar date (month 1-12, day within the month, hour 0-23, minute and second 0-59).

## To Local Time
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|DateTime`

```cpp
static bool ToLocalTime(const FDateTime& UtcDateTime, FDateTime& OutLocalDateTime);
```

Converts a UTC timestamp to local wall-clock time using the timezone rules in effect at that instant. Sub-millisecond ticks are preserved.

| Parameter | Type | Description |
|-----------|------|-------------|
| UtcDateTime | `const FDateTime&` | The timestamp treated as UTC. |
| OutLocalDateTime | `FDateTime&` | [out] The local time, or `UtcDateTime` on failure. |

**Returns:** False when the platform's time routines reject the value or the result would fall outside the `FDateTime` range.

**Platform difference:** On Windows, dates before the year 1601 are outside the system time range and fail. On macOS and Linux, those dates convert with the platform's historical zone rules, which often apply local mean time, so the same input can give different results on different platforms.

## To UTC Time
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|DateTime`

```cpp
static bool ToUtcTime(const FDateTime& LocalDateTime, FDateTime& OutUtcDateTime);
```

Converts local wall-clock time to a UTC timestamp using the timezone rules in effect at that instant. Sub-millisecond ticks are preserved. Ambiguous or skipped local times around daylight-saving transitions resolve per the platform's standard rules.

| Parameter | Type | Description |
|-----------|------|-------------|
| LocalDateTime | `const FDateTime&` | The timestamp treated as local time. |
| OutUtcDateTime | `FDateTime&` | [out] The UTC time, or `LocalDateTime` on failure. |

**Returns:** False when the platform's time routines reject the value or the result would fall outside the `FDateTime` range.

**Platform difference:** On Windows, dates before the year 1601 fail. On macOS and Linux, they convert with the platform's historical zone rules.

## Format Time Span
**Type:** Blueprint Pure &nbsp;|&nbsp; **Category:** `Directive Utilities|DateTime`

```cpp
static FString FormatTimeSpan(const FTimespan& TimeSpan, const bool bIncludeMilliseconds = false);
```

Formats a span as clock-like text: `hh:mm:ss`, prefixed with days as `d.` when the span holds one day or more. Negative spans start with `-`. Optional milliseconds are truncated from the remaining ticks rather than rounded.

| Parameter | Type | Description |
|-----------|------|-------------|
| TimeSpan | `const FTimespan&` | The span to format. |
| bIncludeMilliseconds | `const bool` | When true, seconds are followed by `.fff`. |

**Returns:** The formatted span, such as `02:05:09`, `3.14:00:00`, or `00:00:01.500`.
