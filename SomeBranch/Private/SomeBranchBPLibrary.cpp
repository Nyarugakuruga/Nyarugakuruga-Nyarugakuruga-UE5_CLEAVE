// Copyright Epic Games, Inc. All Rights Reserved.

#include "SomeBranchBPLibrary.h"
#include "SomeBranch.h"

USomeBranchBPLibrary::USomeBranchBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USomeBranchBPLibrary::SomeBranch(
	bool A,
	bool B,
	ESomeBranchResult& Branches
)
{
	if (A && B)
	{
		Branches = ESomeBranchResult::A_And_B;
	}
	else if (A && !B)
	{
		Branches = ESomeBranchResult::A_And_Not_B;
	}
	else
	{
		Branches = ESomeBranchResult::Not_A;
	}
}