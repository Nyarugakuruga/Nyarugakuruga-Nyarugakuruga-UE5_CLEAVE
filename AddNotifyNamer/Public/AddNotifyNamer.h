// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FRenameRuleItem
{
	TSharedPtr<SEditableTextBox> NotifyNameTextBox;
};

class FToolBarBuilder;
class FMenuBuilder;

class FAddNotifyNamerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	void RebuildRuleListUI();
	void AddNewRuleRow();
	void RemoveRuleRow(int32 Index);

	TSharedPtr<SVerticalBox> RuleListWidget;
	TArray<TSharedPtr<FRenameRuleItem>> RuleItems;

	FReply OnCreateNotifiesClicked();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
