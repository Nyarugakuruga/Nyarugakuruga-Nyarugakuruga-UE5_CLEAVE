// Copyright Epic Games, Inc. All Rights Reserved.

#include "AddNotifyNamer.h"
#include "AddNotifyNamerStyle.h"
#include "AddNotifyNamerCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Editor.h"
#include "Selection.h"

#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Styling/AppStyle.h"

//アニメーション関連とコンテンツブラウザ関連のヘッダを追加
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ScopedTransaction.h"
#include "FileHelpers.h"


static const FName AddNotifyNamerTabName("AddNotifyNamer");

#define LOCTEXT_NAMESPACE "FAddNotifyNamerModule"

void FAddNotifyNamerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FAddNotifyNamerStyle::Initialize();
	FAddNotifyNamerStyle::ReloadTextures();

	FAddNotifyNamerCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAddNotifyNamerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FAddNotifyNamerModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAddNotifyNamerModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AddNotifyNamerTabName, FOnSpawnTab::CreateRaw(this, &FAddNotifyNamerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FAddNotifyNamerTabTitle", "AddNotifyNamer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FAddNotifyNamerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAddNotifyNamerStyle::Shutdown();

	FAddNotifyNamerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AddNotifyNamerTabName);
}

//新しいルール行を追加する関数
void FAddNotifyNamerModule::AddNewRuleRow()
{
	TSharedPtr<FRenameRuleItem> NewItem = MakeShareable(new FRenameRuleItem());
	RuleItems.Add(NewItem);
	RebuildRuleListUI();
}

//指定されたインデックスの行を削除する関数
void FAddNotifyNamerModule::RemoveRuleRow(int32 Index)
{
	if (RuleItems.IsValidIndex(Index))
	{
		RuleItems.RemoveAt(Index);
		RebuildRuleListUI();
	}
}


//動的にUIを再構築する関数
void FAddNotifyNamerModule::RebuildRuleListUI()
{
	if (!RuleListWidget.IsValid()) return;

	RuleListWidget->ClearChildren();

	for (int32 i = 0; i < RuleItems.Num(); ++i)
	{
		int32 CurrentIndex = i;
		TSharedPtr<FRenameRuleItem> Item = RuleItems[i];

		RuleListWidget->AddSlot()
			.AutoHeight()
			.Padding(0, 2)
			[
				SNew(SHorizontalBox)
					//作成するNotifyの名前を入力するテキストボックス
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(Item->NotifyNameTextBox, SEditableTextBox)
							.HintText(LOCTEXT("NotifyNameHint", "New Notify Name"))
					]
					//削除ボタン
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0, 0, 0)
					[
						SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.ContentPadding(FMargin(4, 2))
							.OnClicked_Lambda([this, CurrentIndex]()
								{
									RemoveRuleRow(CurrentIndex);
									return FReply::Handled();
								})
							[
								SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
							]
					]
			];
	}
}


//Notifyを追加して保存する処理を行う関数
FReply FAddNotifyNamerModule::OnCreateNotifiesClicked()
{
	//コンテンツブラウザで選択されているアセットを取得
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	if (SelectedAssets.IsEmpty())
	{
		return FReply::Handled();
	}

	TArray<UPackage*> PackagesToSave;

	{
		//トランザクションを開始して、複数のアセットに対する変更をまとめて扱う
		FScopedTransaction Transaction(LOCTEXT("CreateNotifyTracksTransaction", "Batch Add Notify Tracks"));

		for (const FAssetData& AssetData : SelectedAssets)
		{
			UAnimSequenceBase* AnimAsset = Cast<UAnimSequenceBase>(AssetData.GetAsset());
			if (!AnimAsset) continue;

			AnimAsset->Modify();
			bool bAddedAny = false;

			//UIで指定された名前のNotifyを追加する
			for (const TSharedPtr<FRenameRuleItem>& Item : RuleItems)
			{
				if (!Item.IsValid() || !Item->NotifyNameTextBox.IsValid()) continue;

				FString NewTrackNameStr = Item->NotifyNameTextBox->GetText().ToString().TrimStartAndEnd();
				if (NewTrackNameStr.IsEmpty()) continue;

				FName NewTrackFName(*NewTrackNameStr);


				//既に同じ名前のトラックが存在するかを確認
				bool bAlreadyExists = false;
				for (const FAnimNotifyTrack& ExistingTrack : AnimAsset->AnimNotifyTracks)
				{
					if (ExistingTrack.TrackName == NewTrackFName)
					{
						bAlreadyExists = true;
						break;
					}
				}


				//同じ名前のトラックが存在しない場合、新しいトラックを追加
				if (!bAlreadyExists)
				{
					FAnimNotifyTrack NewTrack;
					NewTrack.TrackName = NewTrackFName;
					NewTrack.TrackColor = FLinearColor::White; 

					AnimAsset->AnimNotifyTracks.Add(NewTrack);
					bAddedAny = true;
				}
			}


			//アセットに変更が加えられた場合、キャッシュを更新し、アセットを保存する準備を行う
			if (bAddedAny)
			{
				AnimAsset->RefreshCacheData();
				AnimAsset->PostEditChange();
				AnimAsset->MarkPackageDirty();

				PackagesToSave.AddUnique(AnimAsset->GetOutermost());
			}
		}
	}

	//変更が加えられたパッケージを保存する
	if (!PackagesToSave.IsEmpty())
	{
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	}

	return FReply::Handled();
}


//Pluginのタブを生成する関数
TSharedRef<SDockTab> FAddNotifyNamerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	RuleListWidget = SNew(SVerticalBox);

	if (RuleItems.Num() == 0)
	{
		AddNewRuleRow();
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
				+ SScrollBox::Slot()
				.Padding(10)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBorder)
								.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
								.Padding(8)
								[
									SNew(SVerticalBox)
										//ヘッダー行(タイトル、行追加、前削除)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.FillWidth(1.0f)
												.VAlign(VAlign_Center)
												[
													SNew(STextBlock)
														.Text(LOCTEXT("RuleHeader", "Notifies to Create"))
														.Font(FCoreStyle::Get().GetFontStyle("NormalFontBold"))
												]
												+ SHorizontalBox::Slot()
												.AutoWidth()
												.Padding(2, 0)
												[
													SNew(SButton)
														.ButtonStyle(FAppStyle::Get(), "SimpleButton")
														.OnClicked_Lambda([this]()
															{
																AddNewRuleRow();
																return FReply::Handled();
															})
														[
															SNew(SImage)
																.Image(FAppStyle::Get().GetBrush("Icons.PlusCircle"))
														]
												]
											+ SHorizontalBox::Slot()
												.AutoWidth()
												.Padding(2, 0)
												[
													SNew(SButton)
														.ButtonStyle(FAppStyle::Get(), "SimpleButton")
														.OnClicked_Lambda([this]()
															{
																RuleItems.Empty();
																RebuildRuleListUI();
																return FReply::Handled();
															})
														[
															SNew(SImage)
																.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
														]
												]
										]
									//動的入力リストの表示
									+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0, 6, 0, 0)
										[
											RuleListWidget.ToSharedRef()
										]

										//実行ボタン
										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0, 12, 0, 0)
										[
											SNew(SButton)
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												.ContentPadding(FMargin(10, 6))
												.OnClicked(FOnClicked::CreateRaw(this, &FAddNotifyNamerModule::OnCreateNotifiesClicked))
												[
													SNew(STextBlock)
														.Text(LOCTEXT("CreateNotifiesBtn", "Create Notifies"))
														.Font(FCoreStyle::Get().GetFontStyle("NormalFontBold"))
												]
										]
								]
						]
				]
		];
}

//ボタンクリック時にタブを開く関数
void FAddNotifyNamerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AddNotifyNamerTabName);
}


//コンテンツブラウザの右クリックメニューに追加する関数
void FAddNotifyNamerModule::RegisterMenus()
{

	FToolMenuOwnerScoped OwnerScoped(this);

	//AnimMontageの右クリックメニューにAddNotifyNamerを追加
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.AnimMontage");
	if (!Menu)
	{
		return;
	}
	const FName SectionName = "AddNotifyName";
	if (!Menu->FindSection(SectionName))
	{
		Menu->AddSection(SectionName, FText::FromString("FilterTool"));
	}
	FToolMenuSection& Section = Menu->FindOrAddSection(SectionName);
	Section.AddMenuEntryWithCommandList(FAddNotifyNamerCommands::Get().OpenPluginWindow, PluginCommands);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAddNotifyNamerModule, AddNotifyNamer)