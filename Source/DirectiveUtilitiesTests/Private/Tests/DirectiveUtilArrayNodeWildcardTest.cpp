// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Misc/AutomationTest.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallArrayFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

namespace DirectiveUtilArrayNodeTest
{
	struct FDependentPin
	{
		FName Name;
		EPinContainerType ContainerType;
	};

	struct FArrayNodeCase
	{
		FName FunctionName;
		TArray<FDependentPin> DependentPins;
	};

	FEdGraphPinType MakeArrayType(
		const FName Category,
		const FName SubCategory = NAME_None,
		UObject* SubCategoryObject = nullptr)
	{
		FEdGraphPinType PinType;
		PinType.PinCategory = Category;
		PinType.PinSubCategory = SubCategory;
		PinType.PinSubCategoryObject = SubCategoryObject;
		PinType.ContainerType = EPinContainerType::Array;
		return PinType;
	}

	UEdGraph* MakeGraph()
	{
		UBlueprint* Blueprint = NewObject<UBlueprint>();
		Blueprint->GeneratedClass = UObject::StaticClass();
		Blueprint->SkeletonGeneratedClass = UObject::StaticClass();
		UEdGraph* Graph = NewObject<UEdGraph>(Blueprint);
		Graph->Schema = UEdGraphSchema_K2::StaticClass();
		Blueprint->UbergraphPages.Add(Graph);
		return Graph;
	}

	UEdGraphPin* AddArrayOutput(UEdGraph& Graph, const FName Name, const FEdGraphPinType& PinType)
	{
		UEdGraphNode* SourceNode = NewObject<UEdGraphNode>(&Graph);
		Graph.AddNode(SourceNode);
		return SourceNode->CreatePin(EGPD_Output, PinType, Name);
	}

	UK2Node_CallArrayFunction* AddFunctionNode(UEdGraph& Graph, const FName FunctionName)
	{
		UK2Node_CallArrayFunction* Node = NewObject<UK2Node_CallArrayFunction>(&Graph);
		Graph.AddNode(Node);
		const UFunction* Function = UDirectiveUtilArrayFunctionLibrary::StaticClass()->FindFunctionByName(FunctionName);
		Node->FunctionReference.SetExternalMember(FunctionName, UDirectiveUtilArrayFunctionLibrary::StaticClass());
		Node->bDefaultsToPureFunc = Function && Function->HasAnyFunctionFlags(FUNC_BlueprintPure);
		Node->AllocateDefaultPins();
		return Node;
	}

	void Connect(UEdGraphPin& OutputPin, UK2Node& Node, UEdGraphPin& InputPin)
	{
		OutputPin.MakeLinkTo(&InputPin);
		Node.PinConnectionListChanged(&InputPin);
	}

	void Disconnect(UEdGraphPin& OutputPin, UK2Node& Node, UEdGraphPin& InputPin)
	{
		OutputPin.BreakLinkTo(&InputPin);
		Node.PinConnectionListChanged(&InputPin);
	}

	bool HasElementType(
		const UEdGraphPin& Pin,
		const FEdGraphPinType& ArrayType,
		const EPinContainerType ContainerType)
	{
		return Pin.PinType.ContainerType == ContainerType
			&& Pin.PinType.PinCategory == ArrayType.PinCategory
			&& Pin.PinType.PinSubCategory == ArrayType.PinSubCategory
			&& Pin.PinType.PinSubCategoryObject == ArrayType.PinSubCategoryObject;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilArrayNodeWildcardTest,
	"DirectiveUtilities.ArrayNodeWildcardTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilArrayNodeWildcardTest::RunTest(const FString& Parameters)
{
	using namespace DirectiveUtilArrayNodeTest;

	const TArray<FArrayNodeCase> NodeCases = {
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_NextIndex), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_PreviousIndex), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveDuplicates), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetValidFirstItemCopy), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetValidLastItemCopy), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetValidItemFromIndexCopy), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetRandomItem), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_LastValue), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Pop), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_PopFirst), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_RemoveAtSwap), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Slice), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Rotate), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetDistinct), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_CountOccurrences), {{TEXT("ItemToCount"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetMostCommon), {{TEXT("OutItem"), EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_Sample), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_SampleFromStream), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_SampleWeighted), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_SampleWeightedFromStream), {{TEXT("OutArray"), EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilArrayFunctionLibrary, Array_GetPage), {{TEXT("OutArray"), EPinContainerType::Array}}}
	};

	const TArray<FEdGraphPinType> ArrayTypes = {
		MakeArrayType(UEdGraphSchema_K2::PC_String),
		MakeArrayType(UEdGraphSchema_K2::PC_Object, NAME_None, UDirectiveUtilTestObject::StaticClass()),
		MakeArrayType(UEdGraphSchema_K2::PC_Struct, NAME_None, FDirectiveUtilCollisionValue::StaticStruct())
	};

	for (const FArrayNodeCase& NodeCase : NodeCases)
	{
		UEdGraph* Graph = MakeGraph();
		UK2Node_CallArrayFunction* Node = AddFunctionNode(*Graph, NodeCase.FunctionName);
		UEdGraphPin* TargetArray = Node->FindPin(TEXT("TargetArray"));
		TestNotNull(*FString::Printf(TEXT("%s should have a TargetArray pin"), *NodeCase.FunctionName.ToString()), TargetArray);
		if (!TargetArray)
		{
			continue;
		}

		TestTrue(
			*FString::Printf(TEXT("%s should begin as a wildcard array"), *NodeCase.FunctionName.ToString()),
			TargetArray->PinType.IsArray() && TargetArray->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard);
		FCompilerResultsLog ModuleValidationLog;
		FBlueprintEditorUtils::ValidateEditorOnlyNodes(Node, ModuleValidationLog);
		TestEqual(
			*FString::Printf(TEXT("%s should be valid in a runtime Blueprint"), *NodeCase.FunctionName.ToString()),
			ModuleValidationLog.NumWarnings,
			0);

		for (int32 TypeIndex = 0; TypeIndex < ArrayTypes.Num(); ++TypeIndex)
		{
			const FEdGraphPinType& ArrayType = ArrayTypes[TypeIndex];
			UEdGraphPin* ArrayOutput = AddArrayOutput(
				*Graph,
				*FString::Printf(TEXT("ArrayOutput%d"), TypeIndex),
				ArrayType);
			Connect(*ArrayOutput, *Node, *TargetArray);
			TestTrue(
				*FString::Printf(TEXT("%s should resolve TargetArray type %d"), *NodeCase.FunctionName.ToString(), TypeIndex),
				HasElementType(*TargetArray, ArrayType, EPinContainerType::Array));

			for (const FDependentPin& ExpectedPin : NodeCase.DependentPins)
			{
				const UEdGraphPin* DependentPin = Node->FindPin(ExpectedPin.Name);
				TestNotNull(
					*FString::Printf(TEXT("%s should have a %s pin"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()),
					DependentPin);
				if (DependentPin)
				{
					TestTrue(
						*FString::Printf(TEXT("%s should resolve %s type %d"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString(), TypeIndex),
						HasElementType(*DependentPin, ArrayType, ExpectedPin.ContainerType));
				}
			}

			Disconnect(*ArrayOutput, *Node, *TargetArray);
			TestTrue(
				*FString::Printf(TEXT("%s should reset after type %d"), *NodeCase.FunctionName.ToString(), TypeIndex),
				TargetArray->PinType.IsArray() && TargetArray->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard);
		}
	}

	return true;
}
