// Copyright Epic Games, Inc. All Rights Reserved.

#include "AddNotifyNamerCommands.h"

#define LOCTEXT_NAMESPACE "FAddNotifyNamerModule"

void FAddNotifyNamerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "AddNotifyNamer", "Bring up AddNotifyNamer window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
