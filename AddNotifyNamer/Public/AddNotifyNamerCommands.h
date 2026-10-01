// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AddNotifyNamerStyle.h"

class FAddNotifyNamerCommands : public TCommands<FAddNotifyNamerCommands>
{
public:

	FAddNotifyNamerCommands()
		: TCommands<FAddNotifyNamerCommands>(TEXT("AddNotifyNamer"), NSLOCTEXT("Contexts", "AddNotifyNamer", "AddNotifyNamer Plugin"), NAME_None, FAddNotifyNamerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};