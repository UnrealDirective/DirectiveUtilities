// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilEditorAssetAuditLibrary.h"

#include "AssetRegistry/DirectiveUtilAssetRegistry.h"
#include "AssetRegistry/DirectiveUtilDependencyCycleFinder.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/AssetManager.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "UObject/ObjectRedirector.h"

namespace
{
	using namespace UE::AssetRegistry;

	bool IsInsidePath(const FName PackageName, const FName Path)
	{
		const FString Package = PackageName.ToString();
		FString Parent = Path.ToString();
		Parent.RemoveFromEnd(TEXT("/"));
		return Package == Parent || Package.StartsWith(Parent + TEXT("/"));
	}

	bool ContainsPathSegment(const FName PackagePath, const FString& Segment)
	{
		const FString Path = TEXT("/") + PackagePath.ToString().TrimChar(TEXT('/')) + TEXT("/");
		return Path.Contains(TEXT("/") + Segment + TEXT("/"));
	}

	bool IsExcluded(const FAssetData& Asset, const FDirectiveUtilAssetAuditOptions& Options)
	{
		if (Asset.AssetClassPath == UObjectRedirector::StaticClass()->GetClassPathName())
		{
			return true;
		}

		if (ContainsPathSegment(Asset.PackagePath, TEXT("Developers"))
			|| ContainsPathSegment(Asset.PackagePath, TEXT("__ExternalActors__"))
			|| ContainsPathSegment(Asset.PackagePath, TEXT("__ExternalObjects__"))
			|| ContainsPathSegment(Asset.PackagePath, TEXT("Test"))
			|| ContainsPathSegment(Asset.PackagePath, TEXT("Tests")))
		{
			return true;
		}

		for (const FName Path : Options.ExcludedPackagePaths)
		{
			if (IsInsidePath(Asset.PackageName, Path))
			{
				return true;
			}
		}

		return false;
	}

	TArray<FAssetData> GetScannedAssets(IAssetRegistry& AssetRegistry, const FDirectiveUtilAssetAuditOptions& Options)
	{
		DirectiveUtilitiesEditor::EnsureAssetRegistryScan(AssetRegistry);

		FARFilter Filter;
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
		AssetRegistry.GetAssets(Filter, Assets);
		Assets.RemoveAll([&Options](const FAssetData& Asset) {
			return !Asset.IsValid() || IsExcluded(Asset, Options);
		});
		Assets.Sort([](const FAssetData& Left, const FAssetData& Right) {
			return Left.GetSoftObjectPath().LexicalLess(Right.GetSoftObjectPath());
		});
		return Assets;
	}

	bool IsPrimaryAsset(const FAssetData& Asset)
	{
		const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		return AssetManager && AssetManager->GetPrimaryAssetIdForData(Asset).IsValid();
	}

	EDirectiveUtilAssetDependencyType GetDependencyType(const FAssetDependency& Dependency)
	{
		if (EnumHasAnyFlags(Dependency.Category, EDependencyCategory::SearchableName))
		{
			return EDirectiveUtilAssetDependencyType::SearchableName;
		}

		if (EnumHasAnyFlags(Dependency.Category, EDependencyCategory::Manage))
		{
			return EnumHasAnyFlags(Dependency.Properties, EDependencyProperty::Direct)
				? EDirectiveUtilAssetDependencyType::DirectManagement
				: EDirectiveUtilAssetDependencyType::IndirectManagement;
		}

		return EnumHasAnyFlags(Dependency.Properties, EDependencyProperty::Hard)
			? EDirectiveUtilAssetDependencyType::HardPackage
			: EDirectiveUtilAssetDependencyType::SoftPackage;
	}

	bool PackageExists(IAssetRegistry& AssetRegistry, const FName PackageName)
	{
		if (PackageName.IsNone())
		{
			return true;
		}

		const FString Package = PackageName.ToString();
		if (Package.StartsWith(TEXT("/Script/")) || Package.StartsWith(TEXT("/Memory/")) || Package.StartsWith(TEXT("/Temp/")))
		{
			return true;
		}

		TArray<FAssetData> PackageAssets;
		return AssetRegistry.GetAssetsByPackageName(PackageName, PackageAssets, true) && !PackageAssets.IsEmpty();
	}

	class FAssetAuditScan
	{
	public:
		FAssetAuditScan(IAssetRegistry& InAssetRegistry, const FDirectiveUtilAssetAuditOptions& InOptions)
			: AssetRegistry(InAssetRegistry)
			, bIncludePrimaryAssets(InOptions.bIncludePrimaryAssets)
			, Assets(GetScannedAssets(InAssetRegistry, InOptions))
		{
		}

		const TArray<FAssetData>& GetAssets() const
		{
			return Assets;
		}

		const TArray<FAssetDependency>& GetDependencies(const FName PackageName)
		{
			if (const TArray<FAssetDependency>* Existing = Dependencies.Find(PackageName))
			{
				return *Existing;
			}

			TArray<FAssetDependency>& Result = Dependencies.Add(PackageName);
			AssetRegistry.GetDependencies(FAssetIdentifier(PackageName), Result, EDependencyCategory::All);
			return Result;
		}

		const TArray<FName>& GetReferencers(const FName PackageName)
		{
			if (const TArray<FName>* Existing = Referencers.Find(PackageName))
			{
				return *Existing;
			}

			TArray<FName>& Result = Referencers.Add(PackageName);
			AssetRegistry.GetReferencers(PackageName, Result, EDependencyCategory::All);
			Result.Remove(PackageName);
			Result.Sort(FNameLexicalLess());
			return Result;
		}

		bool IncludesPrimaryAssets() const
		{
			return bIncludePrimaryAssets;
		}

		IAssetRegistry& GetAssetRegistry() const
		{
			return AssetRegistry;
		}

	private:
		IAssetRegistry& AssetRegistry;
		bool bIncludePrimaryAssets;
		TArray<FAssetData> Assets;
		TMap<FName, TArray<FAssetDependency>> Dependencies;
		TMap<FName, TArray<FName>> Referencers;
	};

	FString EscapeCsv(const FString& Value)
	{
		FString Escaped = Value.Replace(TEXT("\""), TEXT("\"\""));
		return FString::Printf(TEXT("\"%s\""), *Escaped);
	}

	FString DependencyTypeToString(const EDirectiveUtilAssetDependencyType Type)
	{
		switch (Type)
		{
		case EDirectiveUtilAssetDependencyType::HardPackage:
			return TEXT("Hard Package");
		case EDirectiveUtilAssetDependencyType::SoftPackage:
			return TEXT("Soft Package");
		case EDirectiveUtilAssetDependencyType::SearchableName:
			return TEXT("Searchable Name");
		case EDirectiveUtilAssetDependencyType::DirectManagement:
			return TEXT("Direct Management");
		case EDirectiveUtilAssetDependencyType::IndirectManagement:
			return TEXT("Indirect Management");
}

		return TEXT("Unknown");
	}

	TArray<FAssetData> FindUnreferencedCandidates(FAssetAuditScan& Scan)
	{
		TArray<FAssetData> Candidates;
		for (const FAssetData& Asset : Scan.GetAssets())
		{
			if ((!IsPrimaryAsset(Asset) || Scan.IncludesPrimaryAssets()) && Scan.GetReferencers(Asset.PackageName).IsEmpty())
			{
				Candidates.Add(Asset);
			}
		}
		return Candidates;
	}

	TArray<FDirectiveUtilMissingAssetReference> FindMissingReferences(FAssetAuditScan& Scan)
	{
		TArray<FDirectiveUtilMissingAssetReference> MissingReferences;
		for (const FAssetData& Asset : Scan.GetAssets())
		{
			for (const FAssetDependency& Dependency : Scan.GetDependencies(Asset.PackageName))
			{
				if (!PackageExists(Scan.GetAssetRegistry(), Dependency.AssetId.PackageName))
				{
					FDirectiveUtilMissingAssetReference& MissingReference = MissingReferences.AddDefaulted_GetRef();
					MissingReference.ReferencingAsset = Asset;
					MissingReference.MissingPackage = Dependency.AssetId.PackageName;
					MissingReference.DependencyType = GetDependencyType(Dependency);
				}
			}
		}

		MissingReferences.Sort([](const FDirectiveUtilMissingAssetReference& Left, const FDirectiveUtilMissingAssetReference& Right) {
			if (Left.ReferencingAsset.PackageName != Right.ReferencingAsset.PackageName)
			{
				return Left.ReferencingAsset.PackageName.LexicalLess(Right.ReferencingAsset.PackageName);
			}
			return Left.MissingPackage.LexicalLess(Right.MissingPackage);
		});
		return MissingReferences;
	}

	TArray<FDirectiveUtilAssetDependencyCycle> FindDependencyCycles(FAssetAuditScan& Scan)
	{
		TSet<FName> ScannedPackages;
		for (const FAssetData& Asset : Scan.GetAssets())
		{
			ScannedPackages.Add(Asset.PackageName);
		}

		TMap<FName, TArray<FName>> Graph;
		for (const FName Package : ScannedPackages)
		{
			TArray<FName>& Edges = Graph.Add(Package);
			for (const FAssetDependency& Dependency : Scan.GetDependencies(Package))
			{
				if (ScannedPackages.Contains(Dependency.AssetId.PackageName))
				{
					Edges.AddUnique(Dependency.AssetId.PackageName);
				}
			}
			Edges.Sort(FNameLexicalLess());
		}

		return DirectiveUtilitiesEditor::FDependencyCycleFinder(Graph).Find();
	}
}

TArray<FAssetData> UDirectiveUtilEditorAssetAuditLibrary::FindUnreferencedAssetCandidates(
	const FDirectiveUtilAssetAuditOptions& Options)
{
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return {};
	}

	FAssetAuditScan Scan(*AssetRegistry, Options);
	return FindUnreferencedCandidates(Scan);
}

TArray<FDirectiveUtilMissingAssetReference> UDirectiveUtilEditorAssetAuditLibrary::FindMissingAssetReferences(
	const FDirectiveUtilAssetAuditOptions& Options)
{
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return {};
	}

	FAssetAuditScan Scan(*AssetRegistry, Options);
	return FindMissingReferences(Scan);
}

TArray<FDirectiveUtilAssetDependencyCycle> UDirectiveUtilEditorAssetAuditLibrary::FindAssetDependencyCycles(
	const FDirectiveUtilAssetAuditOptions& Options)
{
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return {};
	}

	FAssetAuditScan Scan(*AssetRegistry, Options);
	return FindDependencyCycles(Scan);
}

FDirectiveUtilAssetAuditReport UDirectiveUtilEditorAssetAuditLibrary::BuildAssetAuditReport(
	const FDirectiveUtilAssetAuditOptions& Options)
{
	FDirectiveUtilAssetAuditReport Report;
	IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
	if (!AssetRegistry)
	{
		return Report;
	}

	FAssetAuditScan Scan(*AssetRegistry, Options);
	const TArray<FAssetData> Candidates = FindUnreferencedCandidates(Scan);
	Report.MissingReferences = FindMissingReferences(Scan);
	Report.DependencyCycles = FindDependencyCycles(Scan);

	TSet<FSoftObjectPath> CandidatePaths;
	for (const FAssetData& Candidate : Candidates)
	{
		CandidatePaths.Add(Candidate.GetSoftObjectPath());
	}

	TMap<FName, int32> MissingCounts;
	for (const FDirectiveUtilMissingAssetReference& MissingReference : Report.MissingReferences)
	{
		++MissingCounts.FindOrAdd(MissingReference.ReferencingAsset.PackageName);
	}

	TSet<FName> CyclicPackages;
	for (const FDirectiveUtilAssetDependencyCycle& Cycle : Report.DependencyCycles)
	{
		CyclicPackages.Append(Cycle.Packages);
	}

	for (const FAssetData& Asset : Scan.GetAssets())
	{
		FDirectiveUtilAssetAuditEntry& Entry = Report.Assets.AddDefaulted_GetRef();
		Entry.Asset = Asset;
		Entry.PackageName = Asset.PackageName;
		Entry.PackagePath = Asset.PackagePath;
		Entry.AssetClass = Asset.AssetClassPath.ToString();
		Entry.bPrimaryAsset = IsPrimaryAsset(Asset);
		Entry.DependencyCount = Scan.GetDependencies(Asset.PackageName).Num();
		Entry.ReferencerCount = Scan.GetReferencers(Asset.PackageName).Num();

		FString PackageFilename;
		if (FPackageName::DoesPackageExist(Asset.PackageName.ToString(), &PackageFilename))
		{
			Entry.DiskSize = FMath::Max<int64>(IFileManager::Get().FileSize(*PackageFilename), 0);
		}

		if (CandidatePaths.Contains(Asset.GetSoftObjectPath()))
		{
			Entry.Findings.Add(TEXT("Unreferenced candidate"));
		}
		if (const int32* MissingCount = MissingCounts.Find(Asset.PackageName))
		{
			Entry.Findings.Add(FString::Printf(TEXT("%d missing reference%s"), *MissingCount, *MissingCount == 1 ? TEXT("") : TEXT("s")));
		}
		if (CyclicPackages.Contains(Asset.PackageName))
		{
			Entry.Findings.Add(TEXT("Dependency cycle"));
		}
	}

	return Report;
}

FString UDirectiveUtilEditorAssetAuditLibrary::AssetAuditReportToCsv(const FDirectiveUtilAssetAuditReport& Report)
{
	TArray<FString> Rows;
	Rows.Add(TEXT("Asset,Package,Path,Class,Disk Size,Dependencies,Referencers,Primary Asset,Findings"));
	for (const FDirectiveUtilAssetAuditEntry& Entry : Report.Assets)
	{
		const TArray<FString> Columns = {
			EscapeCsv(Entry.Asset.GetSoftObjectPath().ToString()),
			EscapeCsv(Entry.PackageName.ToString()),
			EscapeCsv(Entry.PackagePath.ToString()),
			EscapeCsv(Entry.AssetClass),
			LexToString(Entry.DiskSize),
			LexToString(Entry.DependencyCount),
			LexToString(Entry.ReferencerCount),
			Entry.bPrimaryAsset ? TEXT("true") : TEXT("false"),
			EscapeCsv(FString::Join(Entry.Findings, TEXT("; "))),
		};
		Rows.Add(FString::Join(Columns, TEXT(",")));
	}

	Rows.Add(TEXT(""));
	Rows.Add(TEXT("Missing Reference,Referencing Asset,Dependency Type"));
	for (const FDirectiveUtilMissingAssetReference& MissingReference : Report.MissingReferences)
	{
		const TArray<FString> Columns = {
			EscapeCsv(MissingReference.MissingPackage.ToString()),
			EscapeCsv(MissingReference.ReferencingAsset.GetSoftObjectPath().ToString()),
			EscapeCsv(DependencyTypeToString(MissingReference.DependencyType)),
		};
		Rows.Add(FString::Join(Columns, TEXT(",")));
	}

	Rows.Add(TEXT(""));
	Rows.Add(TEXT("Dependency Cycle"));
	for (const FDirectiveUtilAssetDependencyCycle& Cycle : Report.DependencyCycles)
	{
		TArray<FString> Packages;
		for (const FName Package : Cycle.Packages)
		{
			Packages.Add(Package.ToString());
		}
		Rows.Add(EscapeCsv(FString::Join(Packages, TEXT(" -> "))));
	}

		return FString::Join(Rows, TEXT("\n"));
	}
