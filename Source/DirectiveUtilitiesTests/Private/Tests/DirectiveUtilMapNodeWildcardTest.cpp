// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Misc/AutomationTest.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Libraries/DirectiveUtilMapFunctionLibrary.h"
#include "Nodes/DirectiveUtilMapNodeMigration.h"
#include "Nodes/K2Node_DirectiveUtilMapAppend.h"
#include "Tests/DirectiveUtilTestObject.h"

namespace DirectiveUtilMapNodeTest
{
	struct FDependentPin
	{
		FName Name;
		FName Category;
		EPinContainerType ContainerType;
	};

	struct FMapNodeCase
	{
		FName FunctionName;
		TArray<FDependentPin> DependentPins;
	};

	FEdGraphPinType MakeMapType(
		const FName KeyCategory,
		const FName ValueCategory,
		UObject* KeySubCategoryObject = nullptr,
		UObject* ValueSubCategoryObject = nullptr)
	{
		FEdGraphPinType PinType;
		PinType.PinCategory = KeyCategory;
		PinType.PinSubCategoryObject = KeySubCategoryObject;
		PinType.PinValueType.TerminalCategory = ValueCategory;
		PinType.PinValueType.TerminalSubCategoryObject = ValueSubCategoryObject;
		PinType.ContainerType = EPinContainerType::Map;
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

	UEdGraphPin* AddMapOutput(UEdGraph& Graph, const FName Name, const FEdGraphPinType& PinType)
	{
		UEdGraphNode* SourceNode = NewObject<UEdGraphNode>(&Graph);
		Graph.AddNode(SourceNode);
		return SourceNode->CreatePin(EGPD_Output, PinType, Name);
	}

	UK2Node_CallFunction* AddFunctionNode(UEdGraph& Graph, const FName FunctionName)
	{
		UK2Node_CallFunction* Node = NewObject<UK2Node_CallFunction>(&Graph);
		Graph.AddNode(Node);
		const UFunction* Function = UDirectiveUtilMapFunctionLibrary::StaticClass()->FindFunctionByName(FunctionName);
		Node->FunctionReference.SetExternalMember(FunctionName, UDirectiveUtilMapFunctionLibrary::StaticClass());
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

	bool HasMapType(const UEdGraphPin& Pin, const FName KeyCategory, const FName ValueCategory)
	{
		return Pin.PinType.IsMap()
			&& Pin.PinType.PinCategory == KeyCategory
			&& Pin.PinType.PinValueType.TerminalCategory == ValueCategory;
	}

	bool HasMapType(const UEdGraphPin& Pin, const FEdGraphPinType& ExpectedType)
	{
		return Pin.PinType.IsMap()
			&& Pin.PinType.PinCategory == ExpectedType.PinCategory
			&& Pin.PinType.PinSubCategory == ExpectedType.PinSubCategory
			&& Pin.PinType.PinSubCategoryObject == ExpectedType.PinSubCategoryObject
			&& Pin.PinType.PinValueType == ExpectedType.PinValueType;
	}

	bool HasElementType(
		const UEdGraphPin& Pin,
		const FName Category,
		UObject* SubCategoryObject,
		const EPinContainerType ContainerType)
	{
		return Pin.PinType.PinCategory == Category
			&& Pin.PinType.PinSubCategoryObject == SubCategoryObject
			&& Pin.PinType.ContainerType == ContainerType;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilMapNodeWildcardTest,
	"DirectiveUtilities.MapNodeWildcardTests",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMapNodeWildcardTest::RunTest(const FString& Parameters)
{
	using namespace DirectiveUtilMapNodeTest;

	const TArray<FMapNodeCase> NodeCases = {
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_FindOrAdd), {{TEXT("Key"), UEdGraphSchema_K2::PC_String, EPinContainerType::None}, {TEXT("Value"), UEdGraphSchema_K2::PC_Int, EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_ClearValues), {}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_GetKeysByValue), {{TEXT("Value"), UEdGraphSchema_K2::PC_Int, EPinContainerType::None}, {TEXT("Keys"), UEdGraphSchema_K2::PC_String, EPinContainerType::Array}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_HasValue), {{TEXT("Value"), UEdGraphSchema_K2::PC_Int, EPinContainerType::None}}},
		{GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_RemoveKeys), {{TEXT("Keys"), UEdGraphSchema_K2::PC_String, EPinContainerType::Array}}},
	};

	const FEdGraphPinType StringIntMap = MakeMapType(UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int);
	for (const FMapNodeCase& NodeCase : NodeCases)
	{
		UEdGraph* Graph = MakeGraph();
		UK2Node_CallFunction* Node = AddFunctionNode(*Graph, NodeCase.FunctionName);
		UEdGraphPin* TargetMap = Node->FindPin(TEXT("TargetMap"));
		TestNotNull(*FString::Printf(TEXT("%s should have a TargetMap pin"), *NodeCase.FunctionName.ToString()), TargetMap);
		if (!TargetMap)
		{
			continue;
		}

		TestTrue(*FString::Printf(TEXT("%s should begin as a wildcard map"), *NodeCase.FunctionName.ToString()), HasMapType(*TargetMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));
		UEdGraphPin* MapOutput = AddMapOutput(*Graph, TEXT("MapOutput"), StringIntMap);
		Connect(*MapOutput, *Node, *TargetMap);
		TestTrue(*FString::Printf(TEXT("%s should resolve its map type"), *NodeCase.FunctionName.ToString()), HasMapType(*TargetMap, UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int));

		for (const FDependentPin& ExpectedPin : NodeCase.DependentPins)
		{
			const UEdGraphPin* DependentPin = Node->FindPin(ExpectedPin.Name);
			TestNotNull(*FString::Printf(TEXT("%s should have a %s pin"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()), DependentPin);
			if (DependentPin)
			{
				TestEqual(*FString::Printf(TEXT("%s should resolve %s"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()), DependentPin->PinType.PinCategory, ExpectedPin.Category);
				TestEqual(*FString::Printf(TEXT("%s should preserve the %s container"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()), DependentPin->PinType.ContainerType, ExpectedPin.ContainerType);
			}
		}

		Disconnect(*MapOutput, *Node, *TargetMap);
		TestTrue(*FString::Printf(TEXT("%s should reset after disconnect"), *NodeCase.FunctionName.ToString()), HasMapType(*TargetMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));
		for (const FDependentPin& ExpectedPin : NodeCase.DependentPins)
		{
			const UEdGraphPin* DependentPin = Node->FindPin(ExpectedPin.Name);
			if (DependentPin)
			{
				TestEqual(*FString::Printf(TEXT("%s should reset %s after disconnect"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()), DependentPin->PinType.PinCategory, UEdGraphSchema_K2::PC_Wildcard);
			}
		}
	}

	const FEdGraphPinType ObjectStructMap = MakeMapType(
		UEdGraphSchema_K2::PC_Object,
		UEdGraphSchema_K2::PC_Struct,
		UDirectiveUtilTestObject::StaticClass(),
		FDirectiveUtilCollisionValue::StaticStruct());
	for (const FMapNodeCase& NodeCase : NodeCases)
	{
		UEdGraph* Graph = MakeGraph();
		UK2Node_CallFunction* Node = AddFunctionNode(*Graph, NodeCase.FunctionName);
		UEdGraphPin* TargetMap = Node->FindPin(TEXT("TargetMap"));
		if (!TestNotNull(*FString::Printf(TEXT("%s should have a typed TargetMap pin"), *NodeCase.FunctionName.ToString()), TargetMap))
		{
			continue;
		}

		UEdGraphPin* MapOutput = AddMapOutput(*Graph, TEXT("ObjectStructMapOutput"), ObjectStructMap);
		Connect(*MapOutput, *Node, *TargetMap);
		TestTrue(
			*FString::Printf(TEXT("%s should preserve map subtype objects"), *NodeCase.FunctionName.ToString()),
			HasMapType(*TargetMap, ObjectStructMap));
		for (const FDependentPin& ExpectedPin : NodeCase.DependentPins)
		{
			const UEdGraphPin* DependentPin = Node->FindPin(ExpectedPin.Name);
			if (!DependentPin)
			{
				continue;
			}
			const bool bUsesKeyType = ExpectedPin.Name == TEXT("Key") || ExpectedPin.Name == TEXT("Keys");
			TestTrue(
				*FString::Printf(TEXT("%s should preserve %s subtype"), *NodeCase.FunctionName.ToString(), *ExpectedPin.Name.ToString()),
				HasElementType(
					*DependentPin,
					bUsesKeyType ? UEdGraphSchema_K2::PC_Object : UEdGraphSchema_K2::PC_Struct,
					bUsesKeyType
						? static_cast<UObject*>(UDirectiveUtilTestObject::StaticClass())
						: static_cast<UObject*>(FDirectiveUtilCollisionValue::StaticStruct()),
					ExpectedPin.ContainerType));
		}

		Disconnect(*MapOutput, *Node, *TargetMap);
		TestTrue(
			*FString::Printf(TEXT("%s should reset after a typed disconnect"), *NodeCase.FunctionName.ToString()),
			HasMapType(*TargetMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));
	}

	UEdGraph* AppendGraph = MakeGraph();
	UK2Node_DirectiveUtilMapAppend* AppendNode = NewObject<UK2Node_DirectiveUtilMapAppend>(AppendGraph);
	AppendGraph->AddNode(AppendNode);
	AppendNode->AllocateDefaultPins();
	TestTrue(
		"Append node should be defined in an uncooked-only module",
		AppendNode->GetClass()->GetOutermost()->HasAnyPackageFlags(PKG_UncookedOnly));
	FCompilerResultsLog ModuleValidationLog;
	FBlueprintEditorUtils::ValidateEditorOnlyNodes(AppendNode, ModuleValidationLog);
	TestEqual("Append should be valid in a runtime Blueprint", ModuleValidationLog.NumWarnings, 0);
	TestEqual(
		"Append should call the runtime append function",
		AppendNode->GetTargetFunction(),
		UDirectiveUtilMapFunctionLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_Append)));

	UEdGraphPin* TargetMap = AppendNode->FindPin(TEXT("TargetMap"));
	UEdGraphPin* SourceMap = AppendNode->FindPin(TEXT("SourceMap"));
	TestNotNull("Append should have a TargetMap pin", TargetMap);
	TestNotNull("Append should have a SourceMap pin", SourceMap);
	if (!TargetMap || !SourceMap)
	{
		return false;
	}

	TestTrue("Append TargetMap should begin as a wildcard map", HasMapType(*TargetMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));
	TestTrue("Append SourceMap should begin as a wildcard map", HasMapType(*SourceMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));

	UEdGraphPin* SourceOutput = AddMapOutput(*AppendGraph, TEXT("SourceOutput"), StringIntMap);
	Connect(*SourceOutput, *AppendNode, *SourceMap);
	TestTrue("Append TargetMap should resolve from SourceMap", HasMapType(*TargetMap, UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int));
	TestTrue("Append SourceMap should resolve from SourceMap", HasMapType(*SourceMap, UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int));

	FString DisallowReason;
	const FEdGraphPinType NameIntMap = MakeMapType(UEdGraphSchema_K2::PC_Name, UEdGraphSchema_K2::PC_Int);
	UEdGraphPin* MismatchedKeyOutput = AddMapOutput(*AppendGraph, TEXT("MismatchedKeyOutput"), NameIntMap);
	TestTrue("Append should reject a different key type", AppendNode->IsConnectionDisallowed(TargetMap, MismatchedKeyOutput, DisallowReason));

	const FEdGraphPinType StringStringMap = MakeMapType(UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_String);
	UEdGraphPin* MismatchedValueOutput = AddMapOutput(*AppendGraph, TEXT("MismatchedValueOutput"), StringStringMap);
	TestTrue("Append should reject a different value type", AppendNode->IsConnectionDisallowed(TargetMap, MismatchedValueOutput, DisallowReason));
	TestFalse("Append should accept matching map types", AppendNode->IsConnectionDisallowed(TargetMap, SourceOutput, DisallowReason));

	Disconnect(*SourceOutput, *AppendNode, *SourceMap);
	TestTrue("Append TargetMap should reset after disconnect", HasMapType(*TargetMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));
	TestTrue("Append SourceMap should reset after disconnect", HasMapType(*SourceMap, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PC_Wildcard));

	UEdGraphPin* TypedSourceOutput = AddMapOutput(*AppendGraph, TEXT("TypedSourceOutput"), ObjectStructMap);
	Connect(*TypedSourceOutput, *AppendNode, *SourceMap);
	TestTrue("Append TargetMap should preserve subtype objects", HasMapType(*TargetMap, ObjectStructMap));
	TestTrue("Append SourceMap should preserve subtype objects", HasMapType(*SourceMap, ObjectStructMap));
	Disconnect(*TypedSourceOutput, *AppendNode, *SourceMap);

	UEdGraph* MigrationGraph = MakeGraph();
	UBlueprint* MigrationBlueprint = CastChecked<UBlueprint>(MigrationGraph->GetOuter());
	UK2Node_CallFunction* LegacyAppendNode = AddFunctionNode(
		*MigrationGraph,
		GET_FUNCTION_NAME_CHECKED(UDirectiveUtilMapFunctionLibrary, Map_Append));
	LegacyAppendNode->NodePosX = 320;
	LegacyAppendNode->NodePosY = 180;
	UEdGraphPin* LegacyTargetMap = LegacyAppendNode->FindPin(TEXT("TargetMap"));
	UEdGraphPin* LegacyOverwrite = LegacyAppendNode->FindPin(TEXT("bOverwriteExisting"));
	TestNotNull("Legacy Append should have a TargetMap pin", LegacyTargetMap);
	TestNotNull("Legacy Append should have an overwrite pin", LegacyOverwrite);
	if (!LegacyTargetMap || !LegacyOverwrite)
	{
		return false;
	}

	LegacyOverwrite->DefaultValue = TEXT("false");
	UEdGraphPin* LegacyMapOutput = AddMapOutput(*MigrationGraph, TEXT("LegacyMapOutput"), StringIntMap);
	Connect(*LegacyMapOutput, *LegacyAppendNode, *LegacyTargetMap);
	TestTrue("Legacy Append should be upgraded", DirectiveUtilMapNodeMigration::UpgradeLegacyAppendNodes(*MigrationBlueprint));

	TArray<UK2Node_DirectiveUtilMapAppend*> MigratedAppendNodes;
	MigrationGraph->GetNodesOfClass(MigratedAppendNodes);
	TestEqual("Migration should create one Append node", MigratedAppendNodes.Num(), 1);
	if (MigratedAppendNodes.Num() != 1)
	{
		return false;
	}

	UK2Node_DirectiveUtilMapAppend* MigratedAppendNode = MigratedAppendNodes[0];
	TestEqual("Migration should preserve the X position", MigratedAppendNode->NodePosX, 320);
	TestEqual("Migration should preserve the Y position", MigratedAppendNode->NodePosY, 180);
	TestEqual("Migration should preserve the overwrite default", MigratedAppendNode->FindPinChecked(TEXT("bOverwriteExisting"))->DefaultValue, FString(TEXT("false")));
	TestTrue("Migrated TargetMap should preserve its connection", HasMapType(*MigratedAppendNode->FindPinChecked(TEXT("TargetMap")), UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int));
	TestTrue("Migrated SourceMap should resolve from TargetMap", HasMapType(*MigratedAppendNode->FindPinChecked(TEXT("SourceMap")), UEdGraphSchema_K2::PC_String, UEdGraphSchema_K2::PC_Int));

	return true;
}
