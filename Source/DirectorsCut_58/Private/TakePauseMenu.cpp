// TAKE 4471 - pause menu
#include "TakePauseMenu.h"
#include "DirectorsCut_58.h"
#include "TakeWidgets.h"
#include "TakeSaveGame.h"
#include "TakeTypes.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/App.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace
{
	const FLinearColor PInk(0.05f, 0.09f, 0.11f);
	const FLinearColor PMuted(0.33f, 0.40f, 0.45f);
	const FLinearColor PMint(0.50f, 0.83f, 0.77f);
	const FLinearColor PSoft(0.93f, 0.96f, 0.97f);
	enum EItem { Resume, Tasks, Settings, Save, Load, Quit, Notes };
}

UTextBlock* UTakePauseMenu::MakeText(const FString& Text, int32 Size, const FLinearColor& Color, bool bBold)
{
	UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	FSlateFontInfo Font = T->GetFont();
	Font.Size = Size;
	if (bBold) { Font.TypefaceFontName = TEXT("Bold"); }
	T->SetFont(Font);
	T->SetColorAndOpacity(FSlateColor(Color));
	T->SetText(FText::FromString(Text));
	T->SetAutoWrapText(true);
	return T;
}

UTakeIndexedButton* UTakePauseMenu::MakeButton(const FString& Text, int32 Index, bool bPrimary)
{
	UTakeIndexedButton* B = WidgetTree->ConstructWidget<UTakeIndexedButton>(UTakeIndexedButton::StaticClass());
	B->SetBackgroundColor(bPrimary ? PMint : PSoft);
	B->Init(Index);
	UTextBlock* T = MakeText(Text, 16, bPrimary ? FLinearColor::White : PInk, true);
	T->SetAutoWrapText(false);
	B->AddChild(T);
	return B;
}

TSharedRef<SWidget> UTakePauseMenu::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Dim->SetBrushColor(FLinearColor(0.f, 0.02f, 0.03f, 0.55f));
		UCanvasPanelSlot* DimSlot = Root->AddChildToCanvas(Dim);
		DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		DimSlot->SetOffsets(FMargin(0.f));

		UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Card->SetBrushColor(FLinearColor(0.97f, 0.98f, 0.99f, 0.98f));
		Card->SetPadding(FMargin(36.f, 30.f));
		UCanvasPanelSlot* CardSlot = Root->AddChildToCanvas(Card);
		CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CardSlot->SetSize(FVector2D(1040.f, 700.f));

		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Card->SetContent(Col);
		const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		Col->AddChildToVerticalBox(MakeText(TEXT("HALCYON  \u00B7  BREAK"), 13, PMint, true));
		Col->AddChildToVerticalBox(MakeText(FString::Printf(TEXT("Paused.  Take %d."), GI ? GI->TakeNumber : 1), 30, PInk, true));

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = Col->AddChildToVerticalBox(Row);
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RowSlot->SetPadding(FMargin(0.f, 20.f, 0.f, 0.f));

		// Left: menu
		UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Row->AddChildToHorizontalBox(Menu)->SetPadding(FMargin(0.f, 0.f, 30.f, 0.f));
		struct FItem { int32 Id; const TCHAR* Label; };
		const FItem Items[] = { { Resume, TEXT("Resume") }, { Tasks, TEXT("Tasks") }, { Notes, TEXT("To-do list") }, { Settings, TEXT("Settings") },
		                        { Save, TEXT("Save") }, { Load, TEXT("Load") }, { Quit, TEXT("Quit to desktop") } };
		for (const FItem& It : Items)
		{
			UTakeIndexedButton* B = MakeButton(It.Label, It.Id, It.Id == Resume);
			B->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnMenu);
			Menu->AddChildToVerticalBox(B)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}
		StatusText = MakeText(TEXT(""), 14, PMuted);
		Menu->AddChildToVerticalBox(StatusText)->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));

		// Right: pages
		Pages = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass());
		Row->AddChildToHorizontalBox(Pages)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Page 0: tasks
		UVerticalBox* TasksPage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		TasksPage->AddChildToVerticalBox(MakeText(TEXT("TODAY"), 13, PMuted, true));
		TasksText = MakeText(TEXT(""), 18, PInk);
		TasksPage->AddChildToVerticalBox(TasksText)->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		SaveInfo = MakeText(TEXT(""), 13, PMuted);
		TasksPage->AddChildToVerticalBox(SaveInfo)->SetPadding(FMargin(0.f, 24.f, 0.f, 0.f));
		Pages->AddChild(TasksPage);

		// Page 1: settings
		UVerticalBox* SetPage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		SetPage->AddChildToVerticalBox(MakeText(TEXT("VOLUME"), 13, PMuted, true));
		USlider* Vol = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
		Vol->SetMinValue(0.f);
		Vol->SetMaxValue(1.f);
		Vol->SetValue(FApp::GetVolumeMultiplier());
		Vol->SetSliderBarColor(PMuted);
		Vol->SetSliderHandleColor(PMint);
		Vol->OnValueChanged.AddDynamic(this, &UTakePauseMenu::OnVolume);
		SetPage->AddChildToVerticalBox(Vol)->SetPadding(FMargin(0.f, 6.f, 0.f, 18.f));

		SetPage->AddChildToVerticalBox(MakeText(TEXT("GRAPHICS"), 13, PMuted, true));
		UHorizontalBox* Q = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		const TCHAR* Levels[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
		for (int32 L = 0; L < 4; ++L)
		{
			UTakeIndexedButton* B = MakeButton(Levels[L], L);
			B->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnQuality);
			Q->AddChildToHorizontalBox(B)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		}
		SetPage->AddChildToVerticalBox(Q)->SetPadding(FMargin(0.f, 6.f, 0.f, 18.f));

		SetPage->AddChildToVerticalBox(MakeText(TEXT("WINDOW"), 13, PMuted, true));
		UHorizontalBox* W = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		const TCHAR* Modes[] = { TEXT("Fullscreen"), TEXT("Borderless"), TEXT("Windowed") };
		for (int32 M = 0; M < 3; ++M)
		{
			UTakeIndexedButton* B = MakeButton(Modes[M], M);
			B->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnWindowMode);
			W->AddChildToHorizontalBox(B)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		}
		SetPage->AddChildToVerticalBox(W)->SetPadding(FMargin(0.f, 6.f, 0.f, 18.f));

		SetPage->AddChildToVerticalBox(MakeText(TEXT("DIALOGUE"), 13, PMuted, true));
		UHorizontalBox* D = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		const TCHAR* DModes[] = { TEXT("Subtitles"), TEXT("Over their heads"), TEXT("Both") };
		for (int32 M = 0; M < 3; ++M)
		{
			UTakeIndexedButton* B = MakeButton(DModes[M], M);
			B->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnDialogueMode);
			D->AddChildToHorizontalBox(B)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		}
		SetPage->AddChildToVerticalBox(D)->SetPadding(FMargin(0.f, 6.f, 0.f, 18.f));

		SetPage->AddChildToVerticalBox(MakeText(TEXT("CONTROLS"), 13, PMuted, true));
		SetPage->AddChildToVerticalBox(MakeText(
			TEXT("WASD  move      Mouse  look      Space  jump\n")
			TEXT("E  use / talk / pick up / drop      Click or Right Mouse  throw\n")
			TEXT("E or Tab  log out of your terminal      Esc or P  pause\n")
			TEXT("N  write a playtest note      F8  skip the current tasks (playtest)"), 15, PInk))->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		Pages->AddChild(SetPage);

		// Page 2: playtest notes (saved to Saved/PlaytestNotes.txt with the time, take and place) + skip
		UVerticalBox* NotesPage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		NotesPage->AddChildToVerticalBox(MakeText(TEXT("TO-DO LIST"), 13, PMuted, true));
		NotesPage->AddChildToVerticalBox(MakeText(TEXT("Type a note, then Save note. It's stamped with the game clock, act and where you're standing."), 14, PMuted))->SetPadding(FMargin(0.f, 4.f, 0.f, 8.f));
		// The playtest checklist, when Saved/VerifyChecklist.txt exists (a dev aid; the packaged game has none, so it's just notes)
		{
			FString List;
			const FString ListPath = FPaths::ProjectSavedDir() / TEXT("VerifyChecklist.txt");
			if (FFileHelper::LoadFileToString(List, *ListPath))
			{
				List.ParseIntoArrayLines(ChecklistLines, false);
				UScrollBox* ListScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
				for (int32 LineIdx = 0; LineIdx < ChecklistLines.Num(); ++LineIdx)
				{
					const FString Line = ChecklistLines[LineIdx].TrimStartAndEnd();
					if (Line.IsEmpty()) { continue; }
					const bool bItem = Line.StartsWith(TEXT("[ ]")) || Line.StartsWith(TEXT("[x]")) || Line.StartsWith(TEXT("[X]"));
					if (!bItem)
					{
						ListScroll->AddChild(MakeText(Line, 14, PInk, true));
						continue;
					}
					UHorizontalBox* CheckRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
					UCheckBox* Box = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
					Box->SetIsChecked(!Line.StartsWith(TEXT("[ ]")));
					Box->OnCheckStateChanged.AddDynamic(this, &UTakePauseMenu::OnChecklistChanged);
					CheckRow->AddChildToHorizontalBox(Box)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
					UHorizontalBoxSlot* TextSlot = CheckRow->AddChildToHorizontalBox(MakeText(Line.RightChop(3).TrimStart(), 14, PInk));
					TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
					ListScroll->AddChild(CheckRow);
					CheckBoxes.Add(Box);
					CheckLineIndex.Add(LineIdx);
				}
				UVerticalBoxSlot* ListSlot = NotesPage->AddChildToVerticalBox(ListScroll);
				ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				ListSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
			}
		}
		NotesBox = WidgetTree->ConstructWidget<UMultiLineEditableTextBox>(UMultiLineEditableTextBox::StaticClass());
		UVerticalBoxSlot* NoteSlot = NotesPage->AddChildToVerticalBox(NotesBox);
		NoteSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		UHorizontalBox* NoteButtons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTakeIndexedButton* SaveNote = MakeButton(TEXT("Save note"), 0, true);
		SaveNote->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnSaveNote);
		NoteButtons->AddChildToHorizontalBox(SaveNote)->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
		UTakeIndexedButton* SaveResume = MakeButton(TEXT("Save note & resume"), 2, true);
		SaveResume->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnSaveNote);
		NoteButtons->AddChildToHorizontalBox(SaveResume)->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
		UTakeIndexedButton* Skip = MakeButton(TEXT("Skip current tasks"), 1);
		Skip->OnIndexedClicked.AddDynamic(this, &UTakePauseMenu::OnSaveNote);
		NoteButtons->AddChildToHorizontalBox(Skip);
		NotesPage->AddChildToVerticalBox(NoteButtons)->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));
		NotesPage->AddChildToVerticalBox(MakeText(TEXT("Skip (also F8 in game): finishes the tasks on your list, so you can see what comes next."), 13, PMuted))->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		Pages->AddChild(NotesPage);
	}
	return Super::RebuildWidget();
}

void UTakePauseMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bFocusNotes && NotesBox) { bFocusNotes = false; NotesBox->SetKeyboardFocus(); }
}

void UTakePauseMenu::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	RefreshTasks();
	RefreshSaveInfo();
}

void UTakePauseMenu::RefreshTasks()
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!TasksText || !GI) { return; }
	FString T = GI->TaskListText() + TEXT("\n\n");   // the current act's badge tasks
	if (!GI->Objective.IsEmpty()) { T += FString::Printf(TEXT("Next:  %s"), *GI->Objective); }
	else { T += GI->MorningTasksDone() ? TEXT("Next:  10:00 meeting, Conference Room.") : TEXT("Your desk is #4471, by the window."); }
	T += FString::Printf(TEXT("\n\nAccess passes:  %d / 3"), GI->SoulBits);
	T += TEXT("\n\nNOTES  (glyph signatures you've found)\n");
	if (GI->GlyphNotes.Num() == 0) { T += TEXT("Nothing yet. Some things here have squares on them. Look closer."); }
	for (const FString& N : GI->GlyphNotes) { T += N + TEXT("\n"); }
	if (GI->HRModuleDue()) { T += GI->HotBevModuleDue() ? TEXT("\nHR: Hot Beverage Safety training is due (your terminal, Training tab).") : TEXT("\nHR: Respectful Workplace training is due (your terminal, Training tab)."); }
	TasksText->SetText(FText::FromString(T));
}

void UTakePauseMenu::RefreshSaveInfo()
{
	if (!SaveInfo) { return; }
	if (const UTakeSaveGame* S = Cast<UTakeSaveGame>((UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::ProgressSlot, 0) ? UGameplayStatics::LoadGameFromSlot(UTakeSaveGame::ProgressSlot, 0) : nullptr)))
	{
		SaveInfo->SetText(FText::FromString(FString::Printf(TEXT("Last save: Take %d, %s"), S->TakeNumber, *S->SavedAt.ToString(TEXT("%H:%M")))));
	}
	else
	{
		SaveInfo->SetText(FText::FromString(TEXT("No save yet.")));
	}
}

void UTakePauseMenu::SetStatus(const FString& Text)
{
	if (StatusText) { StatusText->SetText(FText::FromString(Text)); }
}

void UTakePauseMenu::OnMenu(int32 Item)
{
	switch (Item)
	{
	case Resume:   OnResumeRequested.Broadcast(); break;
	case Tasks:    if (Pages) { Pages->SetActiveWidgetIndex(0); } RefreshTasks(); break;
	case Settings: if (Pages) { Pages->SetActiveWidgetIndex(1); } break;
	case Notes:    if (Pages) { Pages->SetActiveWidgetIndex(2); } bFocusNotes = true; break;   // focused next tick, so the N that opened this doesn't get typed
	case Save:     SetStatus(SaveProgress(this) ? TEXT("Saved. HALCYON remembers.") : TEXT("Save failed.")); RefreshSaveInfo(); break;
	case Load:
		if (LoadProgress(this)) { SetStatus(TEXT("Loaded.")); RefreshTasks(); OnResumeRequested.Broadcast(); }
		else { SetStatus(TEXT("No save to load.")); }
		break;
	case Quit:
		UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, false);
		break;
	default: break;
	}
}

void UTakePauseMenu::OnChecklistChanged(bool bChecked)
{
	// Write every tick back, so the list keeps its state between plays (and Claude can read what's been checked)
	for (int32 K = 0; K < CheckBoxes.Num() && K < CheckLineIndex.Num(); ++K)
	{
		if (!CheckBoxes[K] || !ChecklistLines.IsValidIndex(CheckLineIndex[K])) { continue; }
		FString& Line = ChecklistLines[CheckLineIndex[K]];
		const int32 At = Line.Find(TEXT("["));
		if (At != INDEX_NONE && At + 2 < Line.Len()) { Line[At + 1] = CheckBoxes[K]->IsChecked() ? TEXT('x') : TEXT(' '); }
	}
	const FString ListPath = FPaths::ProjectSavedDir() / TEXT("VerifyChecklist.txt");
	FFileHelper::SaveStringToFile(FString::Join(ChecklistLines, TEXT("\n")), *ListPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	SetStatus(bChecked ? TEXT("Checked.") : TEXT("Unchecked."));
}

void UTakePauseMenu::OnQuality(int32 Level)
{
	if (UGameUserSettings* S = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		S->SetOverallScalabilityLevel(Level);
		S->ApplySettings(false);
		S->SaveSettings();
		const TCHAR* Names[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
		SetStatus(FString::Printf(TEXT("Graphics: %s"), Names[FMath::Clamp(Level, 0, 3)]));
	}
}

void UTakePauseMenu::OnWindowMode(int32 Mode)
{
	if (UGameUserSettings* S = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		const EWindowMode::Type M = Mode == 0 ? EWindowMode::Fullscreen : (Mode == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
		S->SetFullscreenMode(M);
		S->ApplySettings(false);
		S->SaveSettings();
		SetStatus(Mode == 0 ? TEXT("Fullscreen") : (Mode == 1 ? TEXT("Borderless") : TEXT("Windowed")));
	}
}

void UTakePauseMenu::OnVolume(float Value)
{
	FApp::SetVolumeMultiplier(Value);
	UTakeSaveGame* S = Cast<UTakeSaveGame>((UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::SettingsSlot, 0) ? UGameplayStatics::LoadGameFromSlot(UTakeSaveGame::SettingsSlot, 0) : nullptr));
	if (!S) { S = Cast<UTakeSaveGame>(UGameplayStatics::CreateSaveGameObject(UTakeSaveGame::StaticClass())); }
	if (S)
	{
		S->MasterVolume = Value;
		UGameplayStatics::SaveGameToSlot(S, UTakeSaveGame::SettingsSlot, 0);
	}
}

void UTakePauseMenu::ApplySavedSettings()
{
	if (const UTakeSaveGame* S = Cast<UTakeSaveGame>((UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::SettingsSlot, 0) ? UGameplayStatics::LoadGameFromSlot(UTakeSaveGame::SettingsSlot, 0) : nullptr)))
	{
		FApp::SetVolumeMultiplier(S->MasterVolume);
		TakeSettings::DialogueMode = FMath::Clamp(S->DialogueMode, 0, 2);
	}
}

void UTakePauseMenu::OnDialogueMode(int32 Mode)
{
	TakeSettings::DialogueMode = FMath::Clamp(Mode, 0, 2);
	UTakeSaveGame* S = Cast<UTakeSaveGame>((UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::SettingsSlot, 0) ? UGameplayStatics::LoadGameFromSlot(UTakeSaveGame::SettingsSlot, 0) : nullptr));
	if (!S) { S = Cast<UTakeSaveGame>(UGameplayStatics::CreateSaveGameObject(UTakeSaveGame::StaticClass())); }
	if (S)
	{
		S->DialogueMode = TakeSettings::DialogueMode;
		UGameplayStatics::SaveGameToSlot(S, UTakeSaveGame::SettingsSlot, 0);
	}
	const TCHAR* Names[] = { TEXT("Subtitles"), TEXT("Over their heads"), TEXT("Both") };
	SetStatus(FString::Printf(TEXT("Dialogue: %s"), Names[TakeSettings::DialogueMode]));
}

bool UTakePauseMenu::SaveProgress(const UObject* WorldContext)
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	UTakeSaveGame* S = Cast<UTakeSaveGame>(UGameplayStatics::CreateSaveGameObject(UTakeSaveGame::StaticClass()));
	if (!GI || !S) { return false; }
	S->TakeNumber = GI->TakeNumber;
	S->SoulBits = GI->SoulBits;
	S->TrainingSlide = GI->TrainingSlide;
	S->bTrainingComplete = GI->bTrainingComplete;
	S->bReadEmail = GI->bReadEmail;
	S->bGotCoffee = GI->bGotCoffee;
	S->RunSeed = GI->RunSeed;
	S->BeatIndex = static_cast<int32>(GI->Beat);
	S->Objective = GI->Objective;
	S->bMetReceptionist = GI->bMetReceptionist;
	S->bGotWisp = GI->bGotWisp;
	S->WorkdayMinutes = GI->WorkdayMinutes;
	S->bJoinedCall = GI->bJoinedCall;
	S->bAttendedMeeting = GI->bAttendedMeeting;
	S->CoworkersCheckedIn = GI->CoworkersCheckedIn;
	S->bRepliedChairman = GI->bRepliedChairman;
	S->bClocksFrozen = GI->bClocksFrozen;
	S->ReadMail = GI->ReadMail;
	S->MailReplies = GI->MailReplies;
	S->GlyphNotes = GI->GlyphNotes;
	S->CoworkersHit = GI->CoworkersHit;
	S->GuardComplaints = GI->GuardComplaints;
	S->HRModulesDone = GI->HRModulesDone;
	S->CoffeesPoured = GI->CoffeesPoured;
	S->BestTossStreak = GI->BestTossStreak;
	S->bRepliedPhish = GI->bRepliedPhish;
	S->SavedAt = FDateTime::Now();
	if (const APawn* P = UGameplayStatics::GetPlayerPawn(WorldContext, 0))
	{
		S->PlayerLocation = P->GetActorLocation();
		S->PlayerRotation = P->GetControlRotation();
	}
	if (UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr)
	{
		for (TActorIterator<ATakeFearDirector> It(World); It; ++It) { S->Fear = It->Fear; break; }
	}
	const bool bOk = UGameplayStatics::SaveGameToSlot(S, UTakeSaveGame::ProgressSlot, 0);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Save %s"), bOk ? TEXT("ok") : TEXT("FAILED"));
	return bOk;
}

bool UTakePauseMenu::LoadProgress(const UObject* WorldContext)
{
	const UTakeSaveGame* S = Cast<UTakeSaveGame>((UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::ProgressSlot, 0) ? UGameplayStatics::LoadGameFromSlot(UTakeSaveGame::ProgressSlot, 0) : nullptr));
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	if (!S || !GI) { return false; }
	GI->TakeNumber = S->TakeNumber;
	GI->SetSoulBits(S->SoulBits);
	GI->TrainingSlide = S->TrainingSlide;
	GI->bTrainingComplete = S->bTrainingComplete;
	GI->bReadEmail = S->bReadEmail;
	GI->bGotCoffee = S->bGotCoffee;
	GI->RunSeed = S->RunSeed;   // same seed = same Key Locks as when you saved
	GI->Beat = static_cast<ETakeBeat>(FMath::Clamp(S->BeatIndex, 0, static_cast<int32>(ETakeBeat::Count) - 1));
	GI->bMetReceptionist = S->bMetReceptionist;
	GI->bGotWisp = S->bGotWisp;
	GI->WorkdayMinutes = S->WorkdayMinutes;
	GI->bJoinedCall = S->bJoinedCall;
	GI->bAttendedMeeting = S->bAttendedMeeting;
	GI->CoworkersCheckedIn = FMath::Clamp(S->CoworkersCheckedIn, 0, 3);
	GI->bRepliedChairman = S->bRepliedChairman;
	GI->bClocksFrozen = S->bClocksFrozen;
	GI->ReadMail = S->ReadMail;
	GI->MailReplies = S->MailReplies;
	GI->GlyphNotes = S->GlyphNotes;
	GI->CoworkersHit = S->CoworkersHit;
	GI->GuardComplaints = S->GuardComplaints;
	GI->HRModulesDone = S->HRModulesDone;
	GI->CoffeesPoured = S->CoffeesPoured;
	GI->BestTossStreak = S->BestTossStreak;
	GI->bRepliedPhish = S->bRepliedPhish;
	GI->bDevLunchNow = false;
	GI->SetObjective(S->Objective.IsEmpty() ? GI->ObjectiveForBeat() : S->Objective);
	if (APawn* P = UGameplayStatics::GetPlayerPawn(WorldContext, 0))
	{
		if (!S->PlayerLocation.IsNearlyZero())
		{
			P->SetActorLocation(S->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (AController* C = P->GetController()) { C->SetControlRotation(S->PlayerRotation); }
		}
	}
	if (UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr)
	{
		for (TActorIterator<ATakeFearDirector> It(World); It; ++It) { It->SetFearImmediate(S->Fear); break; }
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Loaded save from %s"), *S->SavedAt.ToString());
	return true;
}

FReply UTakePauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const bool bTyping = NotesBox && NotesBox->HasKeyboardFocus();   // a "p" in a note isn't "pause"
	if (!InKeyEvent.IsRepeat() && (InKeyEvent.GetKey() == EKeys::Escape || (InKeyEvent.GetKey() == EKeys::P && !bTyping)))
	{
		OnResumeRequested.Broadcast();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTakePauseMenu::OnSaveNote(int32 Which)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (Which == 1)   // skip
	{
		if (GI) { GI->TakeSkip(); }
		RefreshTasks();
		SetStatus(TEXT("Skipped: tasks done."));
		return;
	}
	if (!NotesBox || NotesBox->GetText().IsEmpty()) { SetStatus(TEXT("Nothing to save.")); return; }
	FString Where;
	if (const APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector L = P->GetActorLocation();
		Where = FString::Printf(TEXT("x %.0f y %.0f"), L.X, L.Y);
	}
	const int32 M = GI ? FMath::FloorToInt(GI->WorkdayMinutes) : 0;
	const FString Stamp = FString::Printf(TEXT("[%s | game %d:%02d | Act %d | Take %d | %s]"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M")),
		(M / 60 + 11) % 12 + 1, M % 60, GI ? GI->CurrentAct() : 1, GI ? GI->TakeNumber : 1, *Where);
	const FString Line = Stamp + TEXT("\n") + NotesBox->GetText().ToString() + TEXT("\n\n");
	const FString Path = FPaths::ProjectSavedDir() / TEXT("PlaytestNotes.txt");
	const bool bOk = FFileHelper::SaveStringToFile(Line, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
	SetStatus(bOk ? TEXT("Note saved (Saved/PlaytestNotes.txt).") : TEXT("Couldn't save the note."));
	if (bOk) { NotesBox->SetText(FText::GetEmpty()); }
	if (bOk && Which == 2) { OnResumeRequested.Broadcast(); }   // back to the game
}
