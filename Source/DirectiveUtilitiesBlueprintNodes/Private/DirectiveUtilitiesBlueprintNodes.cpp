// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesBlueprintNodes.h"

#include "Engine/Blueprint.h"
#include "Nodes/DirectiveUtilMapNodeMigration.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

void FDirectiveUtilitiesBlueprintNodesModule::StartupModule()
{
	AssetLoadedHandle = FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &FDirectiveUtilitiesBlueprintNodesModule::HandleAssetLoaded);

	for (TObjectIterator<UBlueprint> Blueprint; Blueprint; ++Blueprint)
	{
		HandleAssetLoaded(*Blueprint);
	}
}

void FDirectiveUtilitiesBlueprintNodesModule::ShutdownModule()
{
	FCoreUObjectDelegates::OnAssetLoaded.Remove(AssetLoadedHandle);
}

void FDirectiveUtilitiesBlueprintNodesModule::HandleAssetLoaded(UObject* Asset)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
	if (Blueprint && !Blueprint->HasAnyFlags(RF_Transient))
	{
		DirectiveUtilMapNodeMigration::UpgradeLegacyAppendNodes(*Blueprint);
	}
}

IMPLEMENT_MODULE(FDirectiveUtilitiesBlueprintNodesModule, DirectiveUtilitiesBlueprintNodes)
