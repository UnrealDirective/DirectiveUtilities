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

	void CopyPinDefaults(const UK2Node_CallFunction& LegacyNode, UK2Node_DirectiveUtilMapAppend& NewNode)
	{
		for (const UEdGraphPin* LegacyPin : LegacyNode.Pins)
		{
			UEdGraphPin* NewPin = NewNode.FindPin(LegacyPin->PinName);
			if (!NewPin)
			{
				continue;
			}

			NewPin->DefaultValue = LegacyPin->DefaultValue;
			NewPin->DefaultObject = LegacyPin->DefaultObject;
			NewPin->DefaultTextValue = LegacyPin->DefaultTextValue;
		}
	}

	bool HaveMatchingPins(const UK2Node_CallFunction& LegacyNode, const UK2Node_DirectiveUtilMapAppend& NewNode)
	{
		for (const UEdGraphPin* LegacyPin : LegacyNode.Pins)
		{
			if (!LegacyPin->ParentPin && !NewNode.FindPin(LegacyPin->PinName))
			{
				return false;
			}
		}

		return true;
	}

	void TransferPinLinks(UK2Node_CallFunction& LegacyNode, UK2Node_DirectiveUtilMapAppend& NewNode)
	{
		for (UEdGraphPin* LegacyPin : LegacyNode.Pins)
		{
			UEdGraphPin* NewPin = NewNode.FindPin(LegacyPin->PinName);
			if (!NewPin)
			{
				continue;
			}

			const TArray<UEdGraphPin*> LinkedPins = LegacyPin->LinkedTo;
			for (UEdGraphPin* LinkedPin : LinkedPins)
			{
				LegacyPin->BreakLinkTo(LinkedPin);
				NewPin->MakeLinkTo(LinkedPin);
				if (UK2Node* LinkedNode = Cast<UK2Node>(LinkedPin->GetOwningNode()))
				{
					LinkedNode->PinConnectionListChanged(LinkedPin);
				}
			}

			NewNode.PinConnectionListChanged(NewPin);
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
		CopyPinDefaults(LegacyNode, *NewNode);
		if (!HaveMatchingPins(LegacyNode, *NewNode))
		{
			NewNode->DestroyNode();
			return false;
		}

		TransferPinLinks(LegacyNode, *NewNode);

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
