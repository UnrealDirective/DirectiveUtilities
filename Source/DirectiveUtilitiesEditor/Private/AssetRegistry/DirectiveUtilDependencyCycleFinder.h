// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Types/DirectiveUtilEditorAuditTypes.h"

namespace DirectiveUtilitiesEditor
{
	class DIRECTIVEUTILITIESEDITOR_API FDependencyCycleFinder
	{
	public:
		explicit FDependencyCycleFinder(const TMap<FName, TArray<FName>>& InGraph);

		TArray<FDirectiveUtilAssetDependencyCycle> Find() const;

	private:
		struct FTraversalFrame
		{
			FName Package;
			int32 NextDependencyIndex = 0;
		};

		TArray<FName> BuildFinishOrder() const;
		TMap<FName, TArray<FName>> BuildReverseGraph() const;

		const TMap<FName, TArray<FName>>& Graph;
	};
}
