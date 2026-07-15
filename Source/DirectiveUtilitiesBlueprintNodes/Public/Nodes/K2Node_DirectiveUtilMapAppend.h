// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "K2Node_CallFunction.h"
#include "K2Node_DirectiveUtilMapAppend.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FCompilerResultsLog;
class UEdGraphPin;

UCLASS()
class DIRECTIVEUTILITIESBLUEPRINTNODES_API UK2Node_DirectiveUtilMapAppend : public UK2Node_CallFunction
{
	GENERATED_BODY()

public:
	UK2Node_DirectiveUtilMapAppend();

	virtual void AllocateDefaultPins() override;
	virtual void PostReconstructNode() override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;

private:
	bool ConformMapPins();
	static bool IsAppendMapPin(const UEdGraphPin* Pin);
	static bool IsConcreteMap(const FEdGraphPinType& PinType);
	static bool HaveMatchingMapTypes(const FEdGraphPinType& First, const FEdGraphPinType& Second);
	static void ApplyMapType(UEdGraphPin& Pin, const FEdGraphPinType& MapType);
	static void ResetMapType(UEdGraphPin& Pin);
};
