// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Nodes/DirectiveUtilMapNodeMigration.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Libraries/DirectiveUtilMapFunctionLibrary.h"
#include "Nodes/K2Node_DirectiveUtilMapAppend.h"

namespace
{
	bool IsLegacyAppendNode(const UK2Node_CallFunction& Node)
	{
		return !Node.IsA<UK2Node_DirectiveUtilMapAppend>()
			&& Node.FunctionReference.GetMemberParentClass() == UDirectiveUtilMapFunctionLibrary::StaticClass()
			&& Node.FunctionReference.GetMemberName() == GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_Append);
	}

	void CopyMapPinTypes(const UK2Node_CallFunction& LegacyNode, UK2Node_DirectiveUtilMapAppend& NewNode)
	{
		const FEdGraphPinType* MapType = nullptr;
		for (const FName PinName : {FName(TEXT("TargetMap")), FName(TEXT("SourceMap"))})
		{
			const UEdGraphPin* LegacyPin = LegacyNode.FindPin(PinName);
			if (!LegacyPin)
			{
				continue;
			}

			if (!LegacyPin->LinkedTo.IsEmpty() && LegacyPin->LinkedTo[0]->PinType.IsMap())
			{
				MapType = &LegacyPin->LinkedTo[0]->PinType;
				break;
			}
			if (LegacyPin->PinType.IsMap()
				&& LegacyPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard
				&& LegacyPin->PinType.PinValueType.TerminalCategory != UEdGraphSchema_K2::PC_Wildcard)
			{
				MapType = &LegacyPin->PinType;
				break;
			}
		}

		if (!MapType)
		{
			return;
		}

		for (const FName PinName : {FName(TEXT("TargetMap")), FName(TEXT("SourceMap"))})
		{
			if (UEdGraphPin* NewPin = NewNode.FindPin(PinName))
			{
				NewPin->PinType.PinCategory = MapType->PinCategory;
				NewPin->PinType.PinSubCategory = MapType->PinSubCategory;
				NewPin->PinType.PinSubCategoryObject = MapType->PinSubCategoryObject;
				NewPin->PinType.PinSubCategoryMemberReference = MapType->PinSubCategoryMemberReference;
				NewPin->PinType.PinValueType = MapType->PinValueType;
				NewPin->PinType.bIsWeakPointer = MapType->bIsWeakPointer;
				NewPin->PinType.bIsUObjectWrapper = MapType->bIsUObjectWrapper;
				NewPin->PinType.bSerializeAsSinglePrecisionFloat = MapType->bSerializeAsSinglePrecisionFloat;
			}
		}
	}

	bool ReplaceLegacyAppendNode(UK2Node_CallFunction& LegacyNode)
	{
		UEdGraph* Graph = LegacyNode.GetGraph();
		const UEdGraphSchema_K2* Schema = Graph ? Cast<UEdGraphSchema_K2>(Graph->GetSchema()) : nullptr;
		if (!Graph || !Schema)
		{
			return false;
		}

		UK2Node_DirectiveUtilMapAppend* NewNode = NewObject<UK2Node_DirectiveUtilMapAppend>(Graph, NAME_None, RF_Transactional);
		Graph->AddNode(NewNode, false, false);
		NewNode->CreateNewGuid();
		NewNode->PostPlacedNewNode();
		NewNode->AllocateDefaultPins();
		NewNode->AdvancedPinDisplay = LegacyNode.AdvancedPinDisplay;
		NewNode->SetEnabledState(LegacyNode.GetDesiredEnabledState(), LegacyNode.HasUserSetTheEnabledState());
		CopyMapPinTypes(LegacyNode, *NewNode);

		if (Schema->ReplaceOldNodeWithNew(&LegacyNode, NewNode, {}))
		{
			return true;
		}

		NewNode->DestroyNode();
		return false;
	}
}

bool DirectiveUtilMapNodeMigration::UpgradeLegacyAppendNodes(UBlueprint& Blueprint)
{
	TArray<UEdGraph*> Graphs;
	Blueprint.GetAllGraphs(Graphs);

	bool bModified = false;
	for (UEdGraph* Graph : Graphs)
	{
		if (!Graph)
		{
			continue;
		}

		TArray<UK2Node_CallFunction*> FunctionNodes;
		Graph->GetNodesOfClass(FunctionNodes);
		for (UK2Node_CallFunction* FunctionNode : FunctionNodes)
		{
			if (FunctionNode && IsLegacyAppendNode(*FunctionNode))
			{
				bModified |= ReplaceLegacyAppendNode(*FunctionNode);
			}
		}
	}

	if (bModified)
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(&Blueprint);
	}

	return bModified;
}
