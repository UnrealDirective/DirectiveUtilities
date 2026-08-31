// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilDataTableFunctionLibrary.h"
#include "Libraries/DirectiveUtilCsvFunctionLibrary.h"
#include "Engine/CompositeDataTable.h"
#include "Engine/DataTable.h"
#include "DataTableUtils.h"
#include "Internationalization/Text.h"
#include "Misc/OutputDeviceNull.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/EnumProperty.h"
#include "UObject/Package.h"
#include "UObject/ScriptInterface.h"
#include "UObject/StructOnScope.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

namespace
{
	bool ValidateRowStruct(const UScriptStruct* RowStruct, FString& OutErrorMessage)
	{
		if (RowStruct == nullptr || RowStruct->GetStructureSize() <= 0)
		{
			OutErrorMessage = TEXT("The row struct is null or has no size.");
			return false;
		}
		if (!RowStruct->IsChildOf(FTableRowBase::StaticStruct()) && !RowStruct->IsA<UUserDefinedStruct>())
		{
			OutErrorMessage = FString::Printf(
				TEXT("%s is neither an FTableRowBase row nor a Blueprint Structure asset."),
				*RowStruct->GetName());
			return false;
		}
		return true;
	}

	bool ParseSignedCell(const TCHAR* Cursor, int64& OutValue)
	{
		while (*Cursor == TEXT(' ') || *Cursor == TEXT('\t'))
		{
			++Cursor;
		}
		const bool bNegative = (*Cursor == TEXT('-'));
		if (*Cursor == TEXT('-') || *Cursor == TEXT('+'))
		{
			++Cursor;
		}
		if (*Cursor == TEXT('\0'))
		{
			return false;
		}

		// Accumulate the magnitude unsigned so MIN_int64, whose magnitude exceeds MAX_int64,
		// still parses.
		const uint64 Limit = bNegative ? static_cast<uint64>(MAX_int64) + 1 : static_cast<uint64>(MAX_int64);
		const TCHAR* const DigitsBegin = Cursor;
		uint64 Magnitude = 0;
		while (*Cursor >= TEXT('0') && *Cursor <= TEXT('9'))
		{
			const uint64 Digit = static_cast<uint64>(*Cursor - TEXT('0'));
			if (Magnitude > (Limit - Digit) / 10)
			{
				return false;
			}
			Magnitude = Magnitude * 10 + Digit;
			++Cursor;
		}
		if (Cursor == DigitsBegin)
		{
			return false;
		}

		while (*Cursor == TEXT(' ') || *Cursor == TEXT('\t'))
		{
			++Cursor;
		}
		if (*Cursor != TEXT('\0'))
		{
			return false;
		}

		if (!bNegative)
		{
			OutValue = static_cast<int64>(Magnitude);
		}
		else
		{
			OutValue = Magnitude == static_cast<uint64>(MAX_int64) + 1
				? MIN_int64
				: -static_cast<int64>(Magnitude);
		}
		return true;
	}

	bool ParseUnsignedCell(const FString& Cell, uint64& OutValue)
	{
		const FString Trimmed = Cell.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return false;
		}

		const TCHAR* Cursor = *Trimmed;
		if (*Cursor == TEXT('+'))
		{
			++Cursor;
		}
		if (*Cursor < TEXT('0') || *Cursor > TEXT('9'))
		{
			return false;
		}

		uint64 Value = 0;
		while (*Cursor >= TEXT('0') && *Cursor <= TEXT('9'))
		{
			const uint64 Digit = static_cast<uint64>(*Cursor - TEXT('0'));
			if (Value > (MAX_uint64 - Digit) / 10)
			{
				return false;
			}
			Value = Value * 10 + Digit;
			++Cursor;
		}
		if (*Cursor != TEXT('\0'))
		{
			return false;
		}

		OutValue = Value;
		return true;
	}

	bool ParseDoubleCell(const FString& Cell, double& OutValue)
	{
		const FString Trimmed = Cell.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return false;
		}

		// Charset validation alone accepts shapes Atod silently truncates, such as "1.2.3" or "--5".
		const TCHAR* Cursor = *Trimmed;
		auto ConsumeDigits = [&Cursor]() -> int32
		{
			int32 Count = 0;
			while (*Cursor >= TEXT('0') && *Cursor <= TEXT('9'))
			{
				++Cursor;
				++Count;
			}
			return Count;
		};

		if (*Cursor == TEXT('-') || *Cursor == TEXT('+'))
		{
			++Cursor;
		}
		int32 MantissaDigits = ConsumeDigits();
		if (*Cursor == TEXT('.'))
		{
			++Cursor;
			MantissaDigits += ConsumeDigits();
		}
		if (MantissaDigits == 0)
		{
			return false;
		}
		if (*Cursor == TEXT('e') || *Cursor == TEXT('E'))
		{
			++Cursor;
			if (*Cursor == TEXT('-') || *Cursor == TEXT('+'))
			{
				++Cursor;
			}
			if (ConsumeDigits() == 0)
			{
				return false;
			}
		}
		if (*Cursor != TEXT('\0'))
		{
			return false;
		}

		const double Value = FCString::Atod(*Trimmed);
		if (!FMath::IsFinite(Value))
		{
			return false;
		}
		OutValue = Value;
		return true;
	}

	bool ImportBooleanCell(const FString& Cell, bool& OutValue)
	{
		const FString Trimmed = Cell.TrimStartAndEnd();
		if (Trimmed.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Trimmed == TEXT("1"))
		{
			OutValue = true;
			return true;
		}
		if (Trimmed.Equals(TEXT("false"), ESearchCase::IgnoreCase) || Trimmed == TEXT("0"))
		{
			OutValue = false;
			return true;
		}
		return false;
	}

	bool ImportIntegerCell(const FString& Cell, const int64 Minimum, const int64 Maximum, int64& OutValue)
	{
		int64 Value = 0;
		if (!ParseSignedCell(*Cell, Value))
		{
			return false;
		}
		if (Value < Minimum || Value > Maximum)
		{
			return false;
		}
		OutValue = Value;
		return true;
	}

	bool FindEnumValueByString(const UEnum* Enum, const FString& SearchString, int64& OutValue)
	{
		for (int32 Index = 0; Index < Enum->NumEnums(); ++Index)
		{
			const FString FullName = Enum->GetNameByIndex(Index).ToString();
			const FString PlainName = Enum->GetNameStringByIndex(Index);
			const FString AuthoredName = Enum->GetAuthoredNameStringByIndex(Index);
			if (SearchString.Equals(FullName, ESearchCase::IgnoreCase)
				|| SearchString.Equals(PlainName, ESearchCase::IgnoreCase)
				|| SearchString.Equals(AuthoredName, ESearchCase::IgnoreCase))
			{
				OutValue = Enum->GetValueByIndex(Index);
				return true;
			}
		}
		return false;
	}

	bool ImportEnumCell(const FString& Cell, UEnum* Enum, const FNumericProperty* UnderlyingProperty, void* ValuePtr, const FString& PropertyName, FString& OutErrorMessage)
	{
		FString Trimmed = Cell.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			OutErrorMessage = FString::Printf(
				TEXT("Row value for enumeration '%s' is empty (property %s)."),
				*Enum->GetName(), *PropertyName);
			return false;
		}

		int64 Value = 0;
		if (!FindEnumValueByString(Enum, Trimmed, Value))
		{
			if (!ParseSignedCell(*Trimmed, Value))
			{
				OutErrorMessage = FString::Printf(
					TEXT("Row value '%s' is neither a name nor a number accepted by enumeration '%s' (property %s)."),
					*Trimmed, *Enum->GetName(), *PropertyName);
				return false;
			}
			if (Enum->GetIndexByValue(Value) == INDEX_NONE)
			{
				OutErrorMessage = FString::Printf(
					TEXT("Row value %lld is outside the declared values of enumeration '%s' (property %s)."),
					Value, *Enum->GetName(), *PropertyName);
				return false;
			}
		}

		if (UnderlyingProperty->IsA<FByteProperty>() && (Value < 0 || Value > MAX_uint8))
		{
			OutErrorMessage = FString::Printf(
				TEXT("Row value %lld does not fit the underlying byte of enumeration '%s' (property %s)."),
				Value, *Enum->GetName(), *PropertyName);
			return false;
		}

		UnderlyingProperty->SetIntPropertyValue(ValuePtr, Value);
		return true;
	}

	bool ImportCellIntoProperty(const FString& Cell, FProperty* Property, void* RowData, const FString& PropertyName, FString& OutErrorMessage)
	{
		if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			bool Value = false;
			if (!ImportBooleanCell(Cell, Value))
			{
				OutErrorMessage = FString::Printf(
					TEXT("Row value '%s' is not a Boolean the import accepts (true, false, 1, 0) for property %s."),
					*Cell, *PropertyName);
				return false;
			}
			BoolProperty->SetPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			return ImportEnumCell(Cell, EnumProperty->GetEnum(), EnumProperty->GetUnderlyingProperty(), ValuePtr, PropertyName, OutErrorMessage);
		}

		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			if (UEnum* ByteEnum = ByteProperty->Enum)
			{
				return ImportEnumCell(Cell, ByteEnum, ByteProperty, ValuePtr, PropertyName, OutErrorMessage);
			}

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, 0, MAX_uint8, Value))
			{
				OutErrorMessage = FString::Printf(
					TEXT("Row value '%s' is not an unsigned byte for property %s."),
					*Cell, *PropertyName);
				return false;
			}
			ByteProperty->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FInt8Property* Int8Property = CastField<const FInt8Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, MIN_int8, MAX_int8, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not an int8 for property %s."), *Cell, *PropertyName);
				return false;
			}
			Int8Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FInt16Property* Int16Property = CastField<const FInt16Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, MIN_int16, MAX_int16, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not an int16 for property %s."), *Cell, *PropertyName);
				return false;
			}
			Int16Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FIntProperty* Int32Property = CastField<const FIntProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, MIN_int32, MAX_int32, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not an int32 for property %s."), *Cell, *PropertyName);
				return false;
			}
			Int32Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FInt64Property* Int64Property = CastField<const FInt64Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, MIN_int64, MAX_int64, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not an int64 for property %s."), *Cell, *PropertyName);
				return false;
			}
			Int64Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FUInt16Property* UInt16Property = CastField<const FUInt16Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, 0, MAX_uint16, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not a uint16 for property %s."), *Cell, *PropertyName);
				return false;
			}
			UInt16Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FUInt32Property* UInt32Property = CastField<const FUInt32Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			int64 Value = 0;
			if (!ImportIntegerCell(Cell, 0, MAX_uint32, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not a uint32 for property %s."), *Cell, *PropertyName);
				return false;
			}
			UInt32Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FUInt64Property* UInt64Property = CastField<const FUInt64Property>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			uint64 Value = 0;
			if (!ParseUnsignedCell(Cell, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not a uint64 for property %s."), *Cell, *PropertyName);
				return false;
			}
			UInt64Property->SetIntPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FFloatProperty* FloatProperty = CastField<const FFloatProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			double Value = 0.0;
			const double MaximumFloat = static_cast<double>(TNumericLimits<float>::Max());
			if (!ParseDoubleCell(Cell, Value) || Value < -MaximumFloat || Value > MaximumFloat)
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not a finite float for property %s."), *Cell, *PropertyName);
				return false;
			}
			FloatProperty->SetPropertyValue(ValuePtr, static_cast<float>(Value));
			return true;
		}

		if (const FDoubleProperty* DoubleProperty = CastField<const FDoubleProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			double Value = 0.0;
			if (!ParseDoubleCell(Cell, Value))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value '%s' is not a finite double for property %s."), *Cell, *PropertyName);
				return false;
			}
			DoubleProperty->SetPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FStrProperty* StringProperty = CastField<const FStrProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			StringProperty->SetPropertyValue(ValuePtr, Cell);
			return true;
		}

		if (const FNameProperty* NameProperty = CastField<const FNameProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
			if (Cell.Len() >= NAME_SIZE || Cell.Len() != FCString::Strlen(*Cell))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value for name property %s is too long or contains a null character."), *PropertyName);
				return false;
			}
			NameProperty->SetPropertyValue(ValuePtr, Cell.IsEmpty() ? NAME_None : FName(*Cell));
			return true;
		}

		if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property))
		{
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);

			FText Value;
			const TCHAR* EndCursor = FTextStringHelper::ReadFromBuffer(*Cell, Value);
			while (EndCursor != nullptr && (*EndCursor == TEXT(' ') || *EndCursor == TEXT('\t')))
			{
				++EndCursor;
			}
			if (EndCursor == nullptr || *EndCursor != TEXT('\0'))
			{
				OutErrorMessage = FString::Printf(TEXT("Row value for text property %s is malformed."), *PropertyName);
				return false;
			}
			TextProperty->SetPropertyValue(ValuePtr, Value);
			return true;
		}

		if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property))
		{
			const UScriptStruct* Struct = StructProperty->Struct;
			if (Struct == nullptr)
			{
				OutErrorMessage = FString::Printf(TEXT("Property %s has no struct definition."), *PropertyName);
				return false;
			}

			const FString Trimmed = Cell.TrimStartAndEnd();

			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
			auto TryImport = [&](const FString& Source)
			{
				FStructOnScope ImportedValue(Struct);
				FOutputDeviceNull ErrorCollector;
				const TCHAR* EndCursor = Struct->ImportText(
					*Source, ImportedValue.GetStructMemory(), nullptr, PPF_ExternalEditor, &ErrorCollector, Struct->GetName());
				while (EndCursor != nullptr && (*EndCursor == TEXT(' ') || *EndCursor == TEXT('\t')))
				{
					++EndCursor;
				}
				if (EndCursor == nullptr || *EndCursor != TEXT('\0'))
				{
					return false;
				}
				Struct->CopyScriptStruct(ValuePtr, ImportedValue.GetStructMemory());
				return true;
			};

			const bool bImported = TryImport(Trimmed)
				|| (!Trimmed.StartsWith(TEXT("(")) && TryImport(FString(TEXT("(")) + Trimmed + TEXT(")")));
			if (!bImported)
			{
				OutErrorMessage = FString::Printf(
					TEXT("Row value '%s' is not valid engine text format for struct property %s (%s), such as (X=1.0,Y=2.0,Z=3.0)."),
					*Cell, *PropertyName, *Struct->GetName());
				return false;
			}
			return true;
		}

		OutErrorMessage = FString::Printf(
			TEXT("Property %s has unsupported type %s. Runtime CSV import supports Boolean, numeric, enumeration, string, name, text, and struct properties."),
			*PropertyName, *Property->GetClass()->GetName());
		return false;
	}

	bool ExportEnumToCell(const UEnum* Enum, const int64 Value, const FString& PropertyName, FString& OutCell, FString& OutErrorMessage)
	{
		const int32 EnumIndex = Enum->GetIndexByValue(Value);
		if (EnumIndex == INDEX_NONE)
		{
			OutErrorMessage = FString::Printf(
				TEXT("Value %lld of property %s is not a declared value of enumeration '%s'."),
				Value, *PropertyName, *Enum->GetName());
			return false;
		}
		FString Name = Enum->GetAuthoredNameStringByIndex(EnumIndex);
		if (Name.IsEmpty())
		{
			Name = Enum->GetNameStringByIndex(EnumIndex);
		}
		OutCell = MoveTemp(Name);
		return true;
	}

	bool IsSupportedValueProperty(const FProperty* Property)
	{
		return CastField<const FBoolProperty>(Property)
			|| CastField<const FEnumProperty>(Property)
			|| CastField<const FNumericProperty>(Property)
			|| CastField<const FStrProperty>(Property)
			|| CastField<const FNameProperty>(Property)
			|| CastField<const FTextProperty>(Property);
	}

	bool ValidateSupportedProperty(
		const FProperty* Property,
		const FString& PropertyPath,
		TSet<const UScriptStruct*>& VisitingStructs,
		FString& OutErrorMessage)
	{
		if (Property->ArrayDim != 1)
		{
			OutErrorMessage = FString::Printf(TEXT("Property %s is a fixed-size array, which runtime CSV does not support."), *PropertyPath);
			return false;
		}
		if (IsSupportedValueProperty(Property))
		{
			return true;
		}

		const FStructProperty* StructProperty = CastField<const FStructProperty>(Property);
		if (StructProperty == nullptr || StructProperty->Struct == nullptr)
		{
			OutErrorMessage = FString::Printf(
				TEXT("Property %s has unsupported type %s. Runtime CSV supports Boolean, numeric, enumeration, string, name, text, and value-only struct properties."),
				*PropertyPath, *Property->GetClass()->GetName());
			return false;
		}

		if (VisitingStructs.Contains(StructProperty->Struct))
		{
			OutErrorMessage = FString::Printf(TEXT("Property %s contains a recursive struct definition."), *PropertyPath);
			return false;
		}

		VisitingStructs.Add(StructProperty->Struct);
		for (TFieldIterator<FProperty> PropertyIterator(StructProperty->Struct); PropertyIterator; ++PropertyIterator)
		{
			const FProperty* NestedProperty = *PropertyIterator;
			const FString NestedPath = PropertyPath + TEXT(".") + NestedProperty->GetAuthoredName();
			if (!ValidateSupportedProperty(NestedProperty, NestedPath, VisitingStructs, OutErrorMessage))
			{
				VisitingStructs.Remove(StructProperty->Struct);
				return false;
			}
		}
		VisitingStructs.Remove(StructProperty->Struct);
		return true;
	}

	bool ValidateSupportedProperty(const FProperty* Property, FString& OutErrorMessage)
	{
		TSet<const UScriptStruct*> VisitingStructs;
		return ValidateSupportedProperty(Property, Property->GetAuthoredName(), VisitingStructs, OutErrorMessage);
	}

	bool ExportPropertyToCell(const FProperty* Property, void* RowData, const FString& PropertyName, FString& OutCell, FString& OutErrorMessage)
	{
		if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Property))
		{
			OutCell = BoolProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData)) ? TEXT("true") : TEXT("false");
			return true;
		}

		if (const FEnumProperty* EnumProperty = CastField<const FEnumProperty>(Property))
		{
			const int64 Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData));
			return ExportEnumToCell(EnumProperty->GetEnum(), Value, PropertyName, OutCell, OutErrorMessage);
		}

		if (const FByteProperty* ByteProperty = CastField<const FByteProperty>(Property))
		{
			const uint8 ByteValue = ByteProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData));
			if (ByteProperty->Enum)
			{
				return ExportEnumToCell(ByteProperty->Enum, ByteValue, PropertyName, OutCell, OutErrorMessage);
			}
			OutCell = LexToString(ByteValue);
			return true;
		}

		if (const FNumericProperty* NumericProperty = CastField<const FNumericProperty>(Property))
		{
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
			if (NumericProperty->IsFloatingPoint())
			{
				const double Value = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
				if (!FMath::IsFinite(Value))
				{
					OutErrorMessage = FString::Printf(TEXT("Property %s holds a non-finite number."), *PropertyName);
					return false;
				}
				OutCell = LexToString(Value);
			}
			else if (CastField<const FUInt64Property>(Property))
			{
				// The signed accessor would render the upper half of the range as negative.
				OutCell = LexToString(NumericProperty->GetUnsignedIntPropertyValue(ValuePtr));
			}
			else
			{
				OutCell = LexToString(NumericProperty->GetSignedIntPropertyValue(ValuePtr));
			}
			return true;
		}

		if (const FStrProperty* StringProperty = CastField<const FStrProperty>(Property))
		{
			OutCell = StringProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData));
			return true;
		}

		if (const FNameProperty* NameProperty = CastField<const FNameProperty>(Property))
		{
			OutCell = NameProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData)).ToString();
			return true;
		}

		if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property))
		{
			FTextStringHelper::WriteToBuffer(
				OutCell, TextProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(RowData)));
			return true;
		}

		if (const FStructProperty* StructProperty = CastField<const FStructProperty>(Property))
		{
			const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(RowData);
			FString Formatted;
			// Passing the value as its own defaults forces every member out; a null Defaults
			// makes ExportText skip members that compare equal to a zeroed struct.
			StructProperty->Struct->ExportText(Formatted, ValuePtr, /*Defaults*/ ValuePtr, /*OwnerObject*/ nullptr, PPF_ExternalEditor, /*ExportRootScope*/ nullptr);
			OutCell = MoveTemp(Formatted);
			return true;
		}

		OutErrorMessage = FString::Printf(
			TEXT("Property %s has unsupported type %s. Runtime CSV export supports Boolean, numeric, enumeration, string, name, text, and struct properties."),
			*PropertyName, *Property->GetClass()->GetName());
		return false;
	}
	FProperty* FindHeaderProperty(const TArray<FProperty*>& Properties, const FString& HeaderName)
	{
		for (FProperty* Property : Properties)
		{
			// Blueprint Structure assets mangle GetName() to Field_2_<GUID>; the authored
			// name is what an exported header row carries.
			if (Property->GetName().Equals(HeaderName, ESearchCase::IgnoreCase)
				|| Property->GetAuthoredName().Equals(HeaderName, ESearchCase::IgnoreCase))
			{
				return Property;
			}
		}
		return nullptr;
	}

	bool BuildRowFromCells(
		const TArray<FString>& Cells,
		const TArray<TPair<int32, FProperty*>>& ColumnBindings,
		void* RowData,
		const FString& RowDescription,
		FString& OutErrorMessage)
	{
		for (const TPair<int32, FProperty*>& Binding : ColumnBindings)
		{
			const int32 ColumnIndex = Binding.Key;
			if (!Cells.IsValidIndex(ColumnIndex))
			{
				continue;
			}

			FProperty* Property = Binding.Value;
			if (!ImportCellIntoProperty(Cells[ColumnIndex], Property, RowData, Property->GetAuthoredName(), OutErrorMessage))
			{
				OutErrorMessage = FString::Printf(TEXT("%s: %s"), *RowDescription, *OutErrorMessage);
				return false;
			}
		}
		return true;
	}

	bool MakeRowKey(const FString& Cell, const FString& RowDescription, FName& OutRowKey, FString& OutErrorMessage)
	{
		FString KeyText = Cell.TrimStartAndEnd();
		if (KeyText.IsEmpty())
		{
			OutErrorMessage = FString::Printf(TEXT("%s has an empty Name cell."), *RowDescription);
			return false;
		}
		if (KeyText.Len() >= NAME_SIZE || KeyText.Len() != FCString::Strlen(*KeyText))
		{
			OutErrorMessage = FString::Printf(TEXT("%s has a row name that is too long or contains a null character."), *RowDescription);
			return false;
		}

		OutRowKey = DataTableUtils::MakeValidName(KeyText);
		if (OutRowKey.IsNone())
		{
			OutErrorMessage = FString::Printf(TEXT("%s has a row name that resolves to None."), *RowDescription);
			return false;
		}
		return true;
	}

	bool BuildCanonicalCsv(
		UScriptStruct* RowStruct,
		const FString& CsvText,
		const EDirectiveUtilCsvDelimiter Delimiter,
		const bool bHasHeaderRow,
		FString& OutCanonicalCsv,
		FString& OutKeyHeader,
		FString& OutErrorMessage)
	{
		if (CsvText.Len() != FCString::Strlen(*CsvText))
		{
			OutErrorMessage = TEXT("DataTable CSV text cannot contain null characters.");
			return false;
		}

		FDirectiveUtilCsvDocument Document;
		if (!UDirectiveUtilCsvFunctionLibrary::ParseCsv(CsvText, Delimiter, Document, OutErrorMessage))
		{
			return false;
		}
		if (bHasHeaderRow && Document.Rows.IsEmpty())
		{
			OutErrorMessage = TEXT("The text has no rows to act as the header.");
			return false;
		}

		TArray<FProperty*> Properties;
		for (TFieldIterator<FProperty> PropertyIterator(RowStruct); PropertyIterator; ++PropertyIterator)
		{
			Properties.Add(*PropertyIterator);
		}
		for (const FProperty* Property : Properties)
		{
			if (!ValidateSupportedProperty(Property, OutErrorMessage))
			{
				return false;
			}
		}

		int32 RowNameColumn = INDEX_NONE;
		TArray<TPair<int32, FProperty*>> ColumnBindings;
		if (bHasHeaderRow)
		{
			const FDirectiveUtilCsvRow& HeaderRow = Document.Rows[0];
			TSet<FProperty*> BoundProperties;
			for (int32 ColumnIndex = 0; ColumnIndex < HeaderRow.Cells.Num(); ++ColumnIndex)
			{
				const FString HeaderName = HeaderRow.Cells[ColumnIndex].TrimStartAndEnd();
				if (HeaderName.Equals(TEXT("Name"), ESearchCase::IgnoreCase) && RowNameColumn == INDEX_NONE)
				{
					if (FindHeaderProperty(Properties, HeaderName) != nullptr)
					{
						OutErrorMessage = FString::Printf(
							TEXT("%s declares a property named Name, which collides with the row-key column. Rename the property or import without a header row."),
							*RowStruct->GetName());
						return false;
					}
					RowNameColumn = ColumnIndex;
					continue;
				}

				FProperty* BoundProperty = FindHeaderProperty(Properties, HeaderName);
				if (BoundProperty == nullptr)
				{
					OutErrorMessage = FString::Printf(
						TEXT("Column '%s' matches no property of %s."), *HeaderName, *RowStruct->GetName());
					return false;
				}
				if (BoundProperties.Contains(BoundProperty))
				{
					OutErrorMessage = FString::Printf(
						TEXT("Column '%s' binds property %s more than once."),
						*HeaderName, *BoundProperty->GetAuthoredName());
					return false;
				}
				BoundProperties.Add(BoundProperty);
				ColumnBindings.Add(TPair<int32, FProperty*>(ColumnIndex, BoundProperty));
			}
		}
		else
		{
			for (int32 PropertyIndex = 0; PropertyIndex < Properties.Num(); ++PropertyIndex)
			{
				ColumnBindings.Add(TPair<int32, FProperty*>(PropertyIndex, Properties[PropertyIndex]));
			}
		}

		OutKeyHeader = TEXT("__DirectiveRowKey");
		while (Properties.ContainsByPredicate([&](const FProperty* Property)
		{
			return Property->GetName().Equals(OutKeyHeader, ESearchCase::IgnoreCase);
		}))
		{
			OutKeyHeader += TEXT('_');
		}

		FDirectiveUtilCsvDocument CanonicalDocument;
		CanonicalDocument.Delimiter = EDirectiveUtilCsvDelimiter::Comma;
		FDirectiveUtilCsvRow CanonicalHeader;
		CanonicalHeader.Cells.Reserve(Properties.Num() + 1);
		CanonicalHeader.Cells.Add(OutKeyHeader);
		for (const FProperty* Property : Properties)
		{
			CanonicalHeader.Cells.Add(Property->GetName());
		}
		CanonicalDocument.Rows.Add(MoveTemp(CanonicalHeader));

		const int32 FirstDataRow = bHasHeaderRow ? 1 : 0;
		const int32 HeaderWidth = bHasHeaderRow ? Document.Rows[0].Cells.Num() : 0;
		TSet<FName> UsedRowNames;
		for (int32 RowIndex = FirstDataRow; RowIndex < Document.Rows.Num(); ++RowIndex)
		{
			const TArray<FString>& Cells = Document.Rows[RowIndex].Cells;
			const FString RowDescription = FString::Printf(TEXT("Row %d"), RowIndex + 1);
			if (bHasHeaderRow && Cells.Num() != HeaderWidth)
			{
				OutErrorMessage = FString::Printf(
					TEXT("%s has %d cells; the header has %d."), *RowDescription, Cells.Num(), HeaderWidth);
				return false;
			}
			if (!bHasHeaderRow && Cells.Num() > Properties.Num()
				&& !(Properties.IsEmpty() && Cells.Num() == 1 && Cells[0].IsEmpty()))
			{
				OutErrorMessage = FString::Printf(
					TEXT("%s has %d cells; %s has %d properties."),
					*RowDescription, Cells.Num(), *RowStruct->GetName(), Properties.Num());
				return false;
			}

			FName RowKey;
			if (RowNameColumn != INDEX_NONE)
			{
				if (!MakeRowKey(Cells[RowNameColumn], RowDescription, RowKey, OutErrorMessage))
				{
					return false;
				}
			}
			else
			{
				RowKey = FName(*FString::Printf(TEXT("Row_%d"), RowIndex - FirstDataRow + 1));
			}
			if (UsedRowNames.Contains(RowKey))
			{
				OutErrorMessage = FString::Printf(
					TEXT("%s repeats the row name '%s'."), *RowDescription, *RowKey.ToString());
				return false;
			}
			UsedRowNames.Add(RowKey);

			FStructOnScope RowData(RowStruct);
			if (!BuildRowFromCells(Cells, ColumnBindings, RowData.GetStructMemory(), RowDescription, OutErrorMessage))
			{
				return false;
			}

			FDirectiveUtilCsvRow CanonicalRow;
			CanonicalRow.Cells.Reserve(Properties.Num() + 1);
			CanonicalRow.Cells.Add(RowKey.ToString());
			for (FProperty* Property : Properties)
			{
				FString Cell;
				if (!ExportPropertyToCell(
					Property, RowData.GetStructMemory(), Property->GetAuthoredName(), Cell, OutErrorMessage))
				{
					OutErrorMessage = FString::Printf(TEXT("%s: %s"), *RowDescription, *OutErrorMessage);
					return false;
				}
				CanonicalRow.Cells.Add(MoveTemp(Cell));
			}
			CanonicalDocument.Rows.Add(MoveTemp(CanonicalRow));
		}

		UDirectiveUtilCsvFunctionLibrary::WriteCsv(CanonicalDocument, OutCanonicalCsv);
		return true;
	}

	bool ImportCanonicalCsv(
		UDataTable* Table,
		const FString& CanonicalCsv,
		const FString& KeyHeader,
		FString& OutErrorMessage)
	{
		const bool bOriginalIgnoreExtraFields = Table->bIgnoreExtraFields;
		const bool bOriginalIgnoreMissingFields = Table->bIgnoreMissingFields;
		const bool bOriginalPreserveExistingValues = Table->bPreserveExistingValues;
		const FString OriginalImportKeyField = Table->ImportKeyField;

		Table->bIgnoreExtraFields = false;
		Table->bIgnoreMissingFields = false;
		Table->bPreserveExistingValues = false;
		Table->ImportKeyField = KeyHeader;
		const TArray<FString> Problems = Table->CreateTableFromCSVString(CanonicalCsv);
		Table->bIgnoreExtraFields = bOriginalIgnoreExtraFields;
		Table->bIgnoreMissingFields = bOriginalIgnoreMissingFields;
		Table->bPreserveExistingValues = bOriginalPreserveExistingValues;
		Table->ImportKeyField = OriginalImportKeyField;

		if (!Problems.IsEmpty())
		{
			OutErrorMessage = FString::Join(Problems, TEXT("\n"));
			return false;
		}
		OutErrorMessage.Reset();
		return true;
	}
}

UDataTable* UDirectiveUtilDataTableFunctionLibrary::CreateDataTableFromCsv(
	UScriptStruct* RowStruct,
	const FString& CsvText,
	const EDirectiveUtilCsvDelimiter Delimiter,
	const bool bHasHeaderRow,
	FString& OutErrorMessage)
{
	const FString CsvTextCopy = CsvText;
	OutErrorMessage.Reset();
	if (!ValidateRowStruct(RowStruct, OutErrorMessage))
	{
		return nullptr;
	}

	FString CanonicalCsv;
	FString KeyHeader;
	if (!BuildCanonicalCsv(
		RowStruct, CsvTextCopy, Delimiter, bHasHeaderRow, CanonicalCsv, KeyHeader, OutErrorMessage))
	{
		return nullptr;
	}

	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage(), NAME_None, RF_Transient);
	Table->RowStruct = RowStruct;
	return ImportCanonicalCsv(Table, CanonicalCsv, KeyHeader, OutErrorMessage) ? Table : nullptr;
}

bool UDirectiveUtilDataTableFunctionLibrary::ExportDataTableToCsv(
	const UDataTable* Table,
	const EDirectiveUtilCsvDelimiter Delimiter,
	const bool bIncludeHeaderRow,
	FString& OutCsvText,
	FString& OutErrorMessage)
{
	if (&OutCsvText == &OutErrorMessage)
	{
		OutCsvText = TEXT("The CSV and error outputs must be different strings.");
		return false;
	}
	OutCsvText.Reset();
	OutErrorMessage.Reset();

	if (Table == nullptr)
	{
		OutErrorMessage = TEXT("The table is null.");
		return false;
	}
	if (!ValidateRowStruct(Table->GetRowStruct(), OutErrorMessage))
	{
		return false;
	}
	if (UDirectiveUtilCsvFunctionLibrary::GetCsvDelimiterCharacter(Delimiter).Len() != 1)
	{
		OutErrorMessage = TEXT("The delimiter value is invalid.");
		return false;
	}

	const UScriptStruct* RowStruct = Table->GetRowStruct();

	TArray<FProperty*> Properties;
	for (TFieldIterator<FProperty> PropertyIterator(RowStruct); PropertyIterator; ++PropertyIterator)
	{
		Properties.Add(*PropertyIterator);
	}

	for (const FProperty* Property : Properties)
	{
		if (!ValidateSupportedProperty(Property, OutErrorMessage))
		{
			return false;
		}
	}
	if (bIncludeHeaderRow && FindHeaderProperty(Properties, TEXT("Name")) != nullptr)
	{
		OutErrorMessage = FString::Printf(
			TEXT("%s declares a property named Name, which collides with the row-key column. Export without a header row or rename the property."),
			*RowStruct->GetName());
		return false;
	}

	TArray<FName> RowNames;
	for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
	{
		RowNames.Add(Row.Key);
	}
	RowNames.Sort(FNameLexicalLess());

	FDirectiveUtilCsvDocument Document;
	Document.Delimiter = Delimiter;
	if (bIncludeHeaderRow)
	{
		FDirectiveUtilCsvRow Header;
		Header.Cells.Reserve(Properties.Num() + 1);
		Header.Cells.Add(TEXT("Name"));
		for (const FProperty* Property : Properties)
		{
			Header.Cells.Add(Property->GetAuthoredName());
		}
		Document.Rows.Add(MoveTemp(Header));
	}

	for (const FName& RowName : RowNames)
	{
		const uint8* const* RowDataPointer = Table->GetRowMap().Find(RowName);
		if (RowDataPointer == nullptr || *RowDataPointer == nullptr)
		{
			OutErrorMessage = FString::Printf(TEXT("Row '%s' has no row data."), *RowName.ToString());
			return false;
		}
		const uint8* RowData = *RowDataPointer;

		FDirectiveUtilCsvRow Row;
		Row.Cells.Reserve(Properties.Num() + (bIncludeHeaderRow ? 1 : 0));
		if (bIncludeHeaderRow)
		{
			Row.Cells.Add(RowName.ToString());
		}
		for (int32 PropertyIndex = 0; PropertyIndex < Properties.Num(); ++PropertyIndex)
		{
			FString Cell;
			if (!ExportPropertyToCell(Properties[PropertyIndex], const_cast<uint8*>(RowData), Properties[PropertyIndex]->GetAuthoredName(), Cell, OutErrorMessage))
			{
				OutErrorMessage = FString::Printf(TEXT("Row '%s': %s"), *RowName.ToString(), *OutErrorMessage);
				return false;
			}
			Row.Cells.Add(MoveTemp(Cell));
		}
		Document.Rows.Add(MoveTemp(Row));
	}

	FString CsvOutput;
	UDirectiveUtilCsvFunctionLibrary::WriteCsv(Document, CsvOutput);
	OutCsvText = MoveTemp(CsvOutput);
	return true;
}

bool UDirectiveUtilDataTableFunctionLibrary::ReplaceDataTableFromCsv(
	UDataTable* Table,
	const FString& CsvText,
	const EDirectiveUtilCsvDelimiter Delimiter,
	const bool bHasHeaderRow,
	FString& OutErrorMessage)
{
	const FString CsvTextCopy = CsvText;
	OutErrorMessage.Reset();
	if (Table == nullptr)
	{
		OutErrorMessage = TEXT("The table is null.");
		return false;
	}
	if (!ValidateRowStruct(Table->GetRowStruct(), OutErrorMessage))
	{
		return false;
	}
	if (Table->IsA<UCompositeDataTable>())
	{
		OutErrorMessage = TEXT("Composite Data Tables cannot be replaced directly.");
		return false;
	}

	FString CanonicalCsv;
	FString KeyHeader;
	if (!BuildCanonicalCsv(
		Table->RowStruct, CsvTextCopy, Delimiter, bHasHeaderRow, CanonicalCsv, KeyHeader, OutErrorMessage))
	{
		return false;
	}

	UDataTable* Snapshot = NewObject<UDataTable>(GetTransientPackage(), NAME_None, RF_Transient);
	const TArray<FString> SnapshotProblems = Snapshot->CreateTableFromOtherTable(Table);
	if (!SnapshotProblems.IsEmpty())
	{
		OutErrorMessage = FString::Join(SnapshotProblems, TEXT("\n"));
		return false;
	}

	FString ImportError;
	if (ImportCanonicalCsv(Table, CanonicalCsv, KeyHeader, ImportError))
	{
		return true;
	}

	const TArray<FString> RestoreProblems = Table->CreateTableFromOtherTable(Snapshot);
	OutErrorMessage = ImportError;
	if (!RestoreProblems.IsEmpty())
	{
		OutErrorMessage += TEXT("\nThe original table could not be restored: ");
		OutErrorMessage += FString::Join(RestoreProblems, TEXT("\n"));
	}
	return false;
}

bool UDirectiveUtilDataTableFunctionLibrary::DiffDataTables(
	const UDataTable* Before,
	const UDataTable* After,
	TArray<FName>& OutAddedRows,
	TArray<FName>& OutRemovedRows,
	TArray<FName>& OutChangedRows,
	FString& OutErrorMessage)
{
	if (&OutAddedRows == &OutRemovedRows || &OutAddedRows == &OutChangedRows || &OutRemovedRows == &OutChangedRows)
	{
		OutAddedRows.Reset();
		OutRemovedRows.Reset();
		OutChangedRows.Reset();
		OutErrorMessage = TEXT("The diff outputs must be different arrays.");
		return false;
	}
	OutAddedRows.Reset();
	OutRemovedRows.Reset();
	OutChangedRows.Reset();
	OutErrorMessage.Reset();

	if (Before == nullptr || After == nullptr || Before->GetRowStruct() == nullptr || After->GetRowStruct() == nullptr)
	{
		OutErrorMessage = TEXT("Both tables must have a row struct.");
		return false;
	}
	if (!ValidateRowStruct(Before->GetRowStruct(), OutErrorMessage)
		|| !ValidateRowStruct(After->GetRowStruct(), OutErrorMessage))
	{
		return false;
	}
	if (Before->GetRowStruct() != After->GetRowStruct())
	{
		OutErrorMessage = TEXT("The tables use different row structs.");
		return false;
	}

	const TMap<FName, uint8*>& BeforeRows = Before->GetRowMap();
	const TMap<FName, uint8*>& AfterRows = After->GetRowMap();
	for (const TPair<FName, uint8*>& Row : BeforeRows)
	{
		if (Row.Value == nullptr)
		{
			OutErrorMessage = FString::Printf(TEXT("The earlier table row '%s' has no row data."), *Row.Key.ToString());
			return false;
		}
	}
	for (const TPair<FName, uint8*>& Row : AfterRows)
	{
		if (Row.Value == nullptr)
		{
			OutErrorMessage = FString::Printf(TEXT("The later table row '%s' has no row data."), *Row.Key.ToString());
			return false;
		}
	}

	TArray<FName> AddedRows;
	TArray<FName> RemovedRows;
	TArray<FName> ChangedRows;
	for (const TPair<FName, uint8*>& Row : BeforeRows)
	{
		const uint8* const* AfterRow = AfterRows.Find(Row.Key);
		if (AfterRow == nullptr)
		{
			RemovedRows.Add(Row.Key);
		}
		else if (!Before->GetRowStruct()->CompareScriptStruct(Row.Value, *AfterRow, PPF_None))
		{
			ChangedRows.Add(Row.Key);
		}
	}
	for (const TPair<FName, uint8*>& Row : AfterRows)
	{
		if (!BeforeRows.Contains(Row.Key))
		{
			AddedRows.Add(Row.Key);
		}
	}
	AddedRows.Sort(FNameLexicalLess());
	RemovedRows.Sort(FNameLexicalLess());
	ChangedRows.Sort(FNameLexicalLess());
	OutAddedRows = MoveTemp(AddedRows);
	OutRemovedRows = MoveTemp(RemovedRows);
	OutChangedRows = MoveTemp(ChangedRows);
	return true;
}
