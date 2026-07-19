// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "AssetRegistry/IAssetRegistry.h"

namespace DirectiveUtilitiesEditor
{
	inline void EnsureAssetRegistryScan(IAssetRegistry& AssetRegistry)
	{
		if (!AssetRegistry.IsSearchAllAssets())
		{
			AssetRegistry.SearchAllAssets(true);
		}

		if (AssetRegistry.IsLoadingAssets())
		{
			AssetRegistry.WaitForCompletion();
		}
	}
}
