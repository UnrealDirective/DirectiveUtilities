// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilEditorBlueprintLibrary.h"

#include "AssetRegistry/DirectiveUtilAssetRegistry.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Blueprint/BlueprintSupport.h"
#include "Components/ActorComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"

namespace
{
	bool IsPackageInsideBlueprintSearchPath(const FName PackageName, const FName Path)
	{
		const FString Package = PackageName.ToString();
		FString Parent = Path.ToString();
		Parent.RemoveFromEnd(TEXT("/"));
		return Package == Parent || Package.StartsWith(Parent + TEXT("/"));
	}

	TArray<FAssetData> GetBlueprintAssets(const FDirectiveUtilBlueprintSearchOptions& Options)
	{
		IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
		if (!AssetRegistry)
		{
			return {};
		}

		DirectiveUtilitiesEditor::EnsureAssetRegistryScan(*AssetRegistry);

		FARFilter Filter;
		Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;
		Filter.bRecursivePaths = true;
		if (Options.PackagePaths.IsEmpty())
		{
			Filter.PackagePaths.Add(TEXT("/Game"));
		}
		else
		{
			Filter.PackagePaths.Append(Options.PackagePaths);
		}

		TArray<FAssetData> Assets;
		AssetRegistry->GetAssets(Filter, Assets);
		Assets.RemoveAll([&Options](const FAssetData& Asset) {
			for (const FName Path : Options.ExcludedPackagePaths)
			{
				if (IsPackageInsideBlueprintSearchPath(Asset.PackageName, Path))
				{
					return true;
				}
			}
			return false;
		});
		Assets.Sort([](const FAssetData& Left, const FAssetData& Right) {
			return Left.GetSoftObjectPath().LexicalLess(Right.GetSoftObjectPath());
		});
		return Assets;
	}

	UBlueprint* LoadBlueprint(const FAssetData& Asset)
	{
		return Cast<UBlueprint>(Asset.GetAsset());
	}

	FString GetClassObjectPath(const UClass* Class)
	{
		return Class ? Class->GetPathName() : FString();
	}

	bool IsDirectParent(const FAssetData& Asset, const UClass* ParentClass)
	{
		const FString ParentTag = Asset.GetTagValueRef<FString>(FBlueprintTags::ParentClassPath);
		return FPackageName::ExportTextPathToObjectPath(ParentTag) == GetClassObjectPath(ParentClass);
	}

	bool HasComponentClass(const UBlueprint* Blueprint, const UClass* ComponentClass, const bool bIncludeDerivedComponents)
	{
		TSet<const UBlueprint*> Visited;
		for (const UBlueprint* Current = Blueprint; Current && !Visited.Contains(Current);)
		{
			Visited.Add(Current);
			if (Current->SimpleConstructionScript)
			{
				for (const USCS_Node* Node : Current->SimpleConstructionScript->GetAllNodes())
				{
					if (!Node || !Node->ComponentClass)
					{
						continue;
					}

					const bool bMatches = bIncludeDerivedComponents
						? Node->ComponentClass->IsChildOf(ComponentClass)
						: Node->ComponentClass == ComponentClass;
					if (bMatches)
					{
						return true;
					}
				}
			}

			Current = Current->ParentClass ? UBlueprint::GetBlueprintFromClass(Current->ParentClass) : nullptr;
		}
		return false;
	}
}

EDirectiveUtilBlueprintCompileStatus UDirectiveUtilEditorBlueprintLibrary::GetBlueprintCompileStatus(
	const UBlueprint* Blueprint)
{
	if (!IsValid(Blueprint))
	{
		return EDirectiveUtilBlueprintCompileStatus::Unknown;
	}

	switch (Blueprint->Status)
	{
	case BS_Dirty:
		return EDirectiveUtilBlueprintCompileStatus::Dirty;
	case BS_Error:
		return EDirectiveUtilBlueprintCompileStatus::Error;
	case BS_UpToDate:
		return EDirectiveUtilBlueprintCompileStatus::UpToDate;
	case BS_BeingCreated:
		return EDirectiveUtilBlueprintCompileStatus::BeingCreated;
	case BS_UpToDateWithWarnings:
		return EDirectiveUtilBlueprintCompileStatus::UpToDateWithWarnings;
	case BS_Unknown:
	default:
		return EDirectiveUtilBlueprintCompileStatus::Unknown;
	}
}

TArray<FAssetData> UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByCompileStatus(
	const EDirectiveUtilBlueprintCompileStatus CompileStatus,
	const FDirectiveUtilBlueprintSearchOptions& Options)
{
	TArray<FAssetData> Matches;
	for (const FAssetData& Asset : GetBlueprintAssets(Options))
	{
		if (const UBlueprint* Blueprint = LoadBlueprint(Asset))
		{
			if (GetBlueprintCompileStatus(Blueprint) == CompileStatus)
			{
				Matches.Add(Asset);
			}
		}
	}
	return Matches;
}

TArray<FAssetData> UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsByParentClass(
	UClass* ParentClass,
	const FDirectiveUtilBlueprintSearchOptions& Options,
	const bool bIncludeDescendants)
{
	TArray<FAssetData> Matches;
	if (!IsValid(ParentClass))
	{
		return Matches;
	}

	for (const FAssetData& Asset : GetBlueprintAssets(Options))
	{
		if (!bIncludeDescendants)
		{
			if (IsDirectParent(Asset, ParentClass))
			{
				Matches.Add(Asset);
			}
			continue;
		}

		const UBlueprint* Blueprint = LoadBlueprint(Asset);
		if (Blueprint && Blueprint->ParentClass && Blueprint->ParentClass->IsChildOf(ParentClass))
		{
			Matches.Add(Asset);
		}
	}
	return Matches;
}

TArray<FAssetData> UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsImplementingInterface(
	UClass* InterfaceClass,
	const FDirectiveUtilBlueprintSearchOptions& Options)
{
	TArray<FAssetData> Matches;
	if (!IsValid(InterfaceClass) || !InterfaceClass->HasAnyClassFlags(CLASS_Interface))
	{
		return Matches;
	}

	const FString InterfacePath = GetClassObjectPath(InterfaceClass);
	for (const FAssetData& Asset : GetBlueprintAssets(Options))
	{
		const FString Interfaces = Asset.GetTagValueRef<FString>(FBlueprintTags::ImplementedInterfaces);
		const FString ParentTag = Asset.GetTagValueRef<FString>(FBlueprintTags::ParentClassPath);
		UClass* TaggedParent = FindObject<UClass>(nullptr, *FPackageName::ExportTextPathToObjectPath(ParentTag));
		if (!Interfaces.Contains(InterfacePath) && TaggedParent && !TaggedParent->ImplementsInterface(InterfaceClass))
		{
			continue;
		}

		const UBlueprint* Blueprint = LoadBlueprint(Asset);
		if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->ImplementsInterface(InterfaceClass))
		{
			Matches.Add(Asset);
		}
	}
	return Matches;
}

TArray<FAssetData> UDirectiveUtilEditorBlueprintLibrary::FindBlueprintsContainingComponentClass(
	UClass* ComponentClass,
	const FDirectiveUtilBlueprintSearchOptions& Options,
	const bool bIncludeDerivedComponents)
{
	TArray<FAssetData> Matches;
	if (!IsValid(ComponentClass) || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return Matches;
	}

	for (const FAssetData& Asset : GetBlueprintAssets(Options))
	{
		if (const UBlueprint* Blueprint = LoadBlueprint(Asset))
		{
			if (HasComponentClass(Blueprint, ComponentClass, bIncludeDerivedComponents))
			{
				Matches.Add(Asset);
			}
		}
	}
	return Matches;
}

TArray<FName> UDirectiveUtilEditorBlueprintLibrary::GetUnusedBlueprintVariables(
	UBlueprint* Blueprint,
	const bool bIncludeExternallyAccessibleVariables)
{
	TArray<FName> Names;
	if (!IsValid(Blueprint))
	{
		return Names;
	}

	TArray<FProperty*> UsedVariables;
	TArray<FProperty*> UnusedVariables;
	FBlueprintEditorUtils::GetUsedAndUnusedVariables(Blueprint, UsedVariables, UnusedVariables);
	Names.Reserve(UnusedVariables.Num());
	for (const FProperty* Variable : UnusedVariables)
	{
		if (Variable && (bIncludeExternallyAccessibleVariables || FBlueprintEditorUtils::IsPropertyPrivate(Variable)))
		{
			Names.Add(Variable->GetFName());
		}
	}
	Names.Sort(FNameLexicalLess());
	return Names;
}
