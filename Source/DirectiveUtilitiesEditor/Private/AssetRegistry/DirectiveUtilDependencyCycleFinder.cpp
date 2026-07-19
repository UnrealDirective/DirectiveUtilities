// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "AssetRegistry/DirectiveUtilDependencyCycleFinder.h"

namespace DirectiveUtilitiesEditor
{
	FDependencyCycleFinder::FDependencyCycleFinder(const TMap<FName, TArray<FName>>& InGraph)
		: Graph(InGraph)
	{
	}

	TArray<FDirectiveUtilAssetDependencyCycle> FDependencyCycleFinder::Find() const
	{
		const TArray<FName> FinishOrder = BuildFinishOrder();
		const TMap<FName, TArray<FName>> ReverseGraph = BuildReverseGraph();
		TSet<FName> Visited;
		TArray<FDirectiveUtilAssetDependencyCycle> Cycles;
		for (int32 Index = FinishOrder.Num() - 1; Index >= 0; --Index)
		{
			const FName Root = FinishOrder[Index];
			if (Visited.Contains(Root))
			{
				continue;
			}

			FDirectiveUtilAssetDependencyCycle Cycle;
			TArray<FName> Pending = {Root};
			Visited.Add(Root);
			while (!Pending.IsEmpty())
			{
				const FName Package = Pending.Pop(EAllowShrinking::No);
				Cycle.Packages.Add(Package);
				for (const FName Referencer : ReverseGraph.FindRef(Package))
				{
					if (!Visited.Contains(Referencer))
					{
						Visited.Add(Referencer);
						Pending.Add(Referencer);
					}
				}
			}

			const bool bSelfCycle = Cycle.Packages.Num() == 1
				&& Graph.FindRef(Cycle.Packages[0]).Contains(Cycle.Packages[0]);
			if (Cycle.Packages.Num() > 1 || bSelfCycle)
			{
				Cycle.Packages.Sort(FNameLexicalLess());
				Cycles.Add(MoveTemp(Cycle));
			}
		}

		Cycles.Sort([](const FDirectiveUtilAssetDependencyCycle& Left, const FDirectiveUtilAssetDependencyCycle& Right) {
			return Left.Packages[0].LexicalLess(Right.Packages[0]);
		});
		return Cycles;
	}

	TArray<FName> FDependencyCycleFinder::BuildFinishOrder() const
	{
		TArray<FName> Packages;
		Graph.GetKeys(Packages);
		Packages.Sort(FNameLexicalLess());

		TArray<FName> FinishOrder;
		FinishOrder.Reserve(Packages.Num());
		TSet<FName> Visited;
		for (const FName Root : Packages)
		{
			if (Visited.Contains(Root))
			{
				continue;
			}

			TArray<FTraversalFrame> Pending = {{Root, 0}};
			Visited.Add(Root);
			while (!Pending.IsEmpty())
			{
				FTraversalFrame& Frame = Pending.Last();
				const TArray<FName>& Dependencies = Graph.FindRef(Frame.Package);
				if (Frame.NextDependencyIndex < Dependencies.Num())
				{
					const FName Dependency = Dependencies[Frame.NextDependencyIndex++];
					if (!Visited.Contains(Dependency))
					{
						Visited.Add(Dependency);
						Pending.Add({Dependency, 0});
					}
					continue;
				}

				FinishOrder.Add(Frame.Package);
				Pending.Pop(EAllowShrinking::No);
			}
		}
		return FinishOrder;
	}

	TMap<FName, TArray<FName>> FDependencyCycleFinder::BuildReverseGraph() const
	{
		TMap<FName, TArray<FName>> ReverseGraph;
		for (const TPair<FName, TArray<FName>>& Node : Graph)
		{
			ReverseGraph.FindOrAdd(Node.Key);
			for (const FName Dependency : Node.Value)
			{
				ReverseGraph.FindOrAdd(Dependency).Add(Node.Key);
			}
		}

		for (TPair<FName, TArray<FName>>& Node : ReverseGraph)
		{
			Node.Value.Sort(FNameLexicalLess());
		}
		return ReverseGraph;
	}
}
