// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Nodes/K2Node_DirectiveUtilMapAppend.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Libraries/DirectiveUtilMapFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node_DirectiveUtilMapAppend"

namespace DirectiveUtilMapAppendPins
{
	const FName TargetMap(TEXT("TargetMap"));
	const FName SourceMap(TEXT("SourceMap"));
}

UK2Node_DirectiveUtilMapAppend::UK2Node_DirectiveUtilMapAppend()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_Append),
		UDirectiveUtilMapFunctionLibrary::StaticClass());
}

void UK2Node_DirectiveUtilMapAppend::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	ConformMapPins();
}

void UK2Node_DirectiveUtilMapAppend::PostReconstructNode()
{
	Super::PostReconstructNode();
	ConformMapPins();
}

void UK2Node_DirectiveUtilMapAppend::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	if (IsAppendMapPin(Pin) && ConformMapPins())
	{
		GetGraph()->NotifyGraphChanged();
	}
}

void UK2Node_DirectiveUtilMapAppend::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	const UEdGraphPin* TargetMap = FindPin(DirectiveUtilMapAppendPins::TargetMap);
	const UEdGraphPin* SourceMap = FindPin(DirectiveUtilMapAppendPins::SourceMap);
	if (!TargetMap || !SourceMap || TargetMap->LinkedTo.IsEmpty() || SourceMap->LinkedTo.IsEmpty())
	{
		return;
	}

	if (!HaveMatchingMapTypes(TargetMap->LinkedTo[0]->PinType, SourceMap->LinkedTo[0]->PinType))
	{
		MessageLog.Error(*LOCTEXT("MismatchedMapTypes", "Append requires matching map types on @@.").ToString(), this);
	}
}

void UK2Node_DirectiveUtilMapAppend::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

bool UK2Node_DirectiveUtilMapAppend::IsConnectionDisallowed(
	const UEdGraphPin* MyPin,
	const UEdGraphPin* OtherPin,
	FString& OutReason) const
{
	if (Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason))
	{
		return true;
	}

	if (!IsAppendMapPin(MyPin) || !OtherPin || !IsConcreteMap(OtherPin->PinType))
	{
		return false;
	}

	const FName SiblingName = MyPin->PinName == DirectiveUtilMapAppendPins::TargetMap
		? DirectiveUtilMapAppendPins::SourceMap
		: DirectiveUtilMapAppendPins::TargetMap;
	const UEdGraphPin* SiblingPin = FindPin(SiblingName);
	if (!SiblingPin)
	{
		return false;
	}

	const FEdGraphPinType* SiblingType = &SiblingPin->PinType;
	if (!SiblingPin->LinkedTo.IsEmpty())
	{
		SiblingType = &SiblingPin->LinkedTo[0]->PinType;
	}

	if (IsConcreteMap(*SiblingType) && !HaveMatchingMapTypes(*SiblingType, OtherPin->PinType))
	{
		OutReason = LOCTEXT("MapTypeMismatch", "Both maps must have the same key and value types.").ToString();
		return true;
	}

	return false;
}

bool UK2Node_DirectiveUtilMapAppend::ConformMapPins()
{
	UEdGraphPin* TargetMap = FindPin(DirectiveUtilMapAppendPins::TargetMap);
	UEdGraphPin* SourceMap = FindPin(DirectiveUtilMapAppendPins::SourceMap);
	if (!TargetMap || !SourceMap)
	{
		return false;
	}

	const FEdGraphPinType* ResolvedType = nullptr;
	for (const UEdGraphPin* MapPin : {TargetMap, SourceMap})
	{
		if (!MapPin->LinkedTo.IsEmpty() && IsConcreteMap(MapPin->LinkedTo[0]->PinType))
		{
			ResolvedType = &MapPin->LinkedTo[0]->PinType;
			break;
		}
	}

	const FEdGraphPinType PreviousTargetType = TargetMap->PinType;
	const FEdGraphPinType PreviousSourceType = SourceMap->PinType;
	if (ResolvedType)
	{
		ApplyMapType(*TargetMap, *ResolvedType);
		ApplyMapType(*SourceMap, *ResolvedType);
	}
	else
	{
		ResetMapType(*TargetMap);
		ResetMapType(*SourceMap);
	}

	return PreviousTargetType != TargetMap->PinType || PreviousSourceType != SourceMap->PinType;
}

bool UK2Node_DirectiveUtilMapAppend::IsAppendMapPin(const UEdGraphPin* Pin)
{
	return Pin && (Pin->PinName == DirectiveUtilMapAppendPins::TargetMap || Pin->PinName == DirectiveUtilMapAppendPins::SourceMap);
}

bool UK2Node_DirectiveUtilMapAppend::IsConcreteMap(const FEdGraphPinType& PinType)
{
	return PinType.IsMap()
		&& PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard
		&& PinType.PinValueType.TerminalCategory != UEdGraphSchema_K2::PC_Wildcard;
}

bool UK2Node_DirectiveUtilMapAppend::HaveMatchingMapTypes(
	const FEdGraphPinType& First,
	const FEdGraphPinType& Second)
{
	return First.IsMap()
		&& Second.IsMap()
		&& First.PinCategory == Second.PinCategory
		&& First.PinSubCategory == Second.PinSubCategory
		&& First.PinSubCategoryObject == Second.PinSubCategoryObject
		&& First.bIsWeakPointer == Second.bIsWeakPointer
		&& First.bIsUObjectWrapper == Second.bIsUObjectWrapper
		&& First.PinValueType.TerminalCategory == Second.PinValueType.TerminalCategory
		&& First.PinValueType.TerminalSubCategory == Second.PinValueType.TerminalSubCategory
		&& First.PinValueType.TerminalSubCategoryObject == Second.PinValueType.TerminalSubCategoryObject
		&& First.PinValueType.bTerminalIsWeakPointer == Second.PinValueType.bTerminalIsWeakPointer
		&& First.PinValueType.bTerminalIsUObjectWrapper == Second.PinValueType.bTerminalIsUObjectWrapper;
}

void UK2Node_DirectiveUtilMapAppend::ApplyMapType(UEdGraphPin& Pin, const FEdGraphPinType& MapType)
{
	Pin.PinType.PinCategory = MapType.PinCategory;
	Pin.PinType.PinSubCategory = MapType.PinSubCategory;
	Pin.PinType.PinSubCategoryObject = MapType.PinSubCategoryObject;
	Pin.PinType.PinSubCategoryMemberReference = MapType.PinSubCategoryMemberReference;
	Pin.PinType.PinValueType = MapType.PinValueType;
	Pin.PinType.bIsWeakPointer = MapType.bIsWeakPointer;
	Pin.PinType.bIsUObjectWrapper = MapType.bIsUObjectWrapper;
	Pin.PinType.bSerializeAsSinglePrecisionFloat = MapType.bSerializeAsSinglePrecisionFloat;
}

void UK2Node_DirectiveUtilMapAppend::ResetMapType(UEdGraphPin& Pin)
{
	Pin.PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	Pin.PinType.PinSubCategory = NAME_None;
	Pin.PinType.PinSubCategoryObject = nullptr;
	Pin.PinType.PinSubCategoryMemberReference = FSimpleMemberReference();
	Pin.PinType.PinValueType = FEdGraphTerminalType();
	Pin.PinType.PinValueType.TerminalCategory = UEdGraphSchema_K2::PC_Wildcard;
	Pin.PinType.bIsWeakPointer = false;
	Pin.PinType.bIsUObjectWrapper = false;
	Pin.PinType.bSerializeAsSinglePrecisionFloat = false;
}

#undef LOCTEXT_NAMESPACE
