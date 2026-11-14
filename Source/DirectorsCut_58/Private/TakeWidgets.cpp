// TAKE 4471 - C++-built UI
#include "TakeWidgets.h"
#include "TakeTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/Spacer.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Engine/Texture2D.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeGrabComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"

namespace
{
	const FLinearColor Ink(0.05f, 0.09f, 0.11f);
	const FLinearColor Muted(0.33f, 0.40f, 0.45f);
	const FLinearColor Mint(0.50f, 0.83f, 0.77f);
	const FLinearColor Paper(0.97f, 0.98f, 0.99f, 0.98f);
	const FLinearColor Soft(0.93f, 0.96f, 0.97f);
	const FLinearColor Good(0.25f, 0.62f, 0.45f);
	const FLinearColor Warn(0.78f, 0.36f, 0.30f);
	const TCHAR* GlyphNames[8] = { TEXT("Witness"), TEXT("Reel"), TEXT("Eye"), TEXT("Lock"), TEXT("Slate"), TEXT("Spiral"), TEXT("Signal"), TEXT("Keyhole") };
}

// ---------------------------------------------------------------- Indexed button
void UTakeIndexedButton::Init(int32 InIndex)
{
	Index = InIndex;
	OnClicked.AddDynamic(this, &UTakeIndexedButton::HandleClicked);
}

void UTakeIndexedButton::HandleClicked()
{
	OnIndexedClicked.Broadcast(Index);
}

// ---------------------------------------------------------------- Prompt
TSharedRef<SWidget> UTakePromptWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Prompt"));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = 18;
		Label->SetFont(Font);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.92f)));
		Label->SetShadowOffset(FVector2D(1.f, 1.f));
		Label->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));
		Label->SetJustification(ETextJustify::Center);
		UCanvasPanelSlot* PromptSlot = Root->AddChildToCanvas(Label);
		PromptSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PromptSlot->SetAlignment(FVector2D(0.5f, 0.f));
		PromptSlot->SetAutoSize(true);
		PromptSlot->SetPosition(FVector2D(0.f, 48.f));

		Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Subtitle"));
		FSlateFontInfo SubFont = Subtitle->GetFont();
		SubFont.Size = 22;
		Subtitle->SetFont(SubFont);
		Subtitle->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		Subtitle->SetShadowOffset(FVector2D(1.5f, 1.5f));
		Subtitle->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
		Subtitle->SetJustification(ETextJustify::Center);
		Subtitle->SetAutoWrapText(true);
		UCanvasPanelSlot* SubSlot = Root->AddChildToCanvas(Subtitle);
		SubSlot->SetAnchors(FAnchors(0.5f, 1.f));
		SubSlot->SetAlignment(FVector2D(0.5f, 1.f));
		SubSlot->SetSize(FVector2D(1100.f, 90.f));
		SubSlot->SetPosition(FVector2D(0.f, -70.f));

		// Badge task list (top left)
		TaskPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TaskPanel"));
		TaskPanel->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.82f));
		TaskPanel->SetPadding(FMargin(18.f, 12.f));
		TaskText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Tasks"));
		FSlateFontInfo TaskFont = TaskText->GetFont();
		TaskFont.Size = 15;
		TaskText->SetFont(TaskFont);
		TaskText->SetColorAndOpacity(FSlateColor(FLinearColor(0.05f, 0.09f, 0.11f)));
		TaskPanel->SetContent(TaskText);
		UCanvasPanelSlot* TaskSlot = Root->AddChildToCanvas(TaskPanel);
		TaskSlot->SetAnchors(FAnchors(0.f, 0.f));
		TaskSlot->SetAutoSize(true);
		TaskSlot->SetPosition(FVector2D(32.f, 32.f));
		TaskPanel->SetVisibility(ESlateVisibility::Collapsed);

		// Clock, take and passes (top right)
		ClockText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Clock"));
		FSlateFontInfo ClockFont = ClockText->GetFont();
		ClockFont.Size = 16;
		ClockFont.TypefaceFontName = TEXT("Bold");
		ClockText->SetFont(ClockFont);
		ClockText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.9f)));
		ClockText->SetShadowOffset(FVector2D(1.f, 1.f));
		ClockText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.6f));
		ClockText->SetJustification(ETextJustify::Right);
		UCanvasPanelSlot* ClockSlot = Root->AddChildToCanvas(ClockText);
		ClockSlot->SetAnchors(FAnchors(1.f, 0.f));
		ClockSlot->SetAlignment(FVector2D(1.f, 0.f));
		ClockSlot->SetAutoSize(true);
		ClockSlot->SetPosition(FVector2D(-32.f, 32.f));

		// Crosshair: a small soft dot
		UBorder* Dot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Crosshair"));
		Dot->SetBrushColor(FLinearColor(1.f, 1.f, 1.f, 0.75f));
		UCanvasPanelSlot* DotSlot = Root->AddChildToCanvas(Dot);
		DotSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		DotSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		DotSlot->SetSize(FVector2D(4.f, 4.f));

		// Slate: full-screen black + "TAKE n" (cold open, rewind). Added last so it covers the rest of the HUD.
		SlateBlack = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SlateBlack"));
		SlateBlack->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 1.f));
		UCanvasPanelSlot* BlackSlot = Root->AddChildToCanvas(SlateBlack);
		BlackSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		BlackSlot->SetOffsets(FMargin(0.f));
		SlateBlack->SetVisibility(ESlateVisibility::Collapsed);
		SlateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SlateText"));
		FSlateFontInfo SlateFont = SlateText->GetFont();
		SlateFont.Size = 64;
		SlateFont.TypefaceFontName = TEXT("Bold");
		SlateFont.LetterSpacing = 300;
		SlateText->SetFont(SlateFont);
		SlateText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)));
		SlateText->SetJustification(ETextJustify::Center);
		UCanvasPanelSlot* SlateTextSlot = Root->AddChildToCanvas(SlateText);
		SlateTextSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		SlateTextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		SlateTextSlot->SetAutoSize(true);
		SlateText->SetVisibility(ESlateVisibility::Collapsed);
	}
	return Super::RebuildWidget();
}

void UTakePromptWidget::SetSlate(float BlackAlpha, const FString& Text, float TextAlpha)
{
	if (!SlateBlack || !SlateText) { return; }
	const bool bBlack = BlackAlpha > 0.001f;
	const bool bText = TextAlpha > 0.001f && !Text.IsEmpty();
	SlateBlack->SetVisibility(bBlack ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bBlack) { SlateBlack->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, FMath::Clamp(BlackAlpha, 0.f, 1.f))); }
	SlateText->SetVisibility(bText ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bText)
	{
		if (Text != CurrentSlate) { CurrentSlate = Text; SlateText->SetText(FText::FromString(Text)); }
		SlateText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(TextAlpha, 0.f, 1.f))));
	}
}

void UTakePromptWidget::SetTasks(const FString& Text)
{
	if (Text == CurrentTasks || !TaskText || !TaskPanel) { return; }
	CurrentTasks = Text;
	TaskText->SetText(FText::FromString(Text));
	TaskPanel->SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UTakePromptWidget::SetClock(const FString& Text)
{
	if (Text == CurrentClock || !ClockText) { return; }
	CurrentClock = Text;
	ClockText->SetText(FText::FromString(Text));
}

void UTakePromptWidget::SetSubtitle(const FString& Text)
{
	if (Subtitle) { Subtitle->SetText(FText::FromString(Text)); }
}

void UTakePromptWidget::SetPrompt(const FString& Text)
{
	if (Text == Current || !Label) { return; }
	Current = Text;
	Label->SetText(FText::FromString(Text));
}

// ---------------------------------------------------------------- Terminal
UTextBlock* UTakeTerminalWidget::MakeText(const FString& Text, int32 Size, const FLinearColor& Color, bool bBold)
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

void UTakeTerminalWidget::LoadMail()
{
	Mails.Reset();
	if (const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Emails.DT_Emails")))
	{
		TArray<FTakeEmailRow*> Rows;
		Table->GetAllRows<FTakeEmailRow>(TEXT("TakeTerminal"), Rows);
		for (const FTakeEmailRow* R : Rows)
		{
			if (R && R->Act == 1)
			{
				Mails.Add({ TakeText::Fill(this, R->Sender), TakeText::Fill(this, R->Subject), TakeText::Fill(this, R->Body), R->IsPhishing });
			}
		}
	}
	if (Mails.Num() == 0)   // fallback until the DataTable is imported
	{
		Mails.Add({ TEXT("HALCYON People Team"), TEXT("Welcome back, 4471!"), TEXT("Good morning, 4471!\n\nYour task list is on your badge.\n\nRemember: you are exactly where you need to be.") });
		Mails.Add({ TEXT("The Chairman"), TEXT("Places, everyone."), TEXT("Team,\n\nBig day. Everyone knows their marks.\n\nLet's get it in one take this time.\n\n- C") });
	}
}

TSharedRef<SWidget> UTakeTerminalWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		LoadMail();
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		UBorder* Screen = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Screen"));
		Screen->SetBrushColor(Paper);
		Screen->SetPadding(FMargin(0.f));
		UCanvasPanelSlot* ScreenSlot = Root->AddChildToCanvas(Screen);
		ScreenSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		ScreenSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ScreenSlot->SetSize(FVector2D(1240.f, 720.f));

		UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Screen->SetContent(Page);

		// Header bar
		UBorder* Bar = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Bar->SetBrushColor(Mint);
		Bar->SetPadding(FMargin(24.f, 14.f));
		UHorizontalBox* BarRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Bar->SetContent(BarRow);
		UHorizontalBoxSlot* TitleSlot = BarRow->AddChildToHorizontalBox(MakeText(TEXT("HALCYON MAIL"), 20, FLinearColor::White, true));
		TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		BarRow->AddChildToHorizontalBox(MakeText(TEXT("Employee 4471  \u00B7  9:02 AM"), 14, FLinearColor::White));
		Page->AddChildToVerticalBox(Bar);

		// Tabs: Inbox | Training
		UBorder* TabBar = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		TabBar->SetBrushColor(Soft);
		TabBar->SetPadding(FMargin(16.f, 6.f));
		UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		TabBar->SetContent(Tabs);
		UTakeIndexedButton* InboxTab = MakeButton(TEXT("Inbox"), 0, FLinearColor::White, Ink);
		InboxTab->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::ShowTab);
		Tabs->AddChildToHorizontalBox(InboxTab)->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		UTakeIndexedButton* TrainTab = MakeButton(TEXT("Training"), 1, FLinearColor::White, Ink);
		TrainingTabLabel = Cast<UTextBlock>(TrainTab->GetChildAt(0));
		TrainTab->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::ShowTab);
		Tabs->AddChildToHorizontalBox(TrainTab);
		Page->AddChildToVerticalBox(TabBar);

		Pages = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass());
		UVerticalBoxSlot* PagesSlot = Page->AddChildToVerticalBox(Pages);
		PagesSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// Page 0: inbox list + reader
		UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (UWidgetSwitcherSlot* BodySlot = Cast<UWidgetSwitcherSlot>(Pages->AddChild(Body))) { BodySlot->SetPadding(FMargin(16.f)); }

		// Page 1: Compliance Training
		UScrollBox* TrainScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
		TrainingBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		TrainScroll->AddChild(TrainingBox);
		if (UWidgetSwitcherSlot* TrainSlot = Cast<UWidgetSwitcherSlot>(Pages->AddChild(TrainScroll))) { TrainSlot->SetPadding(FMargin(48.f, 24.f)); }

		USizeBox* ListBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ListBox->SetWidthOverride(420.f);
		UScrollBox* List = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
		ListBox->AddChild(List);
		Body->AddChildToHorizontalBox(ListBox);
		for (int32 I = 0; I < Mails.Num(); ++I)
		{
			UTakeIndexedButton* B = WidgetTree->ConstructWidget<UTakeIndexedButton>(UTakeIndexedButton::StaticClass());
			B->SetBackgroundColor(Soft);
			B->Init(I);
			B->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::ShowEmail);
			UVerticalBox* Entry = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
			Entry->AddChildToVerticalBox(MakeText(Mails[I].Sender, 13, Muted));
			Entry->AddChildToVerticalBox(MakeText(Mails[I].Subject, 15, Ink, true));
			B->AddChild(Entry);
			List->AddChild(B);
		}

		UVerticalBox* ReadPane = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UHorizontalBoxSlot* ReadSlot = Body->AddChildToHorizontalBox(ReadPane);
		ReadSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ReadSlot->SetPadding(FMargin(28.f, 6.f, 10.f, 6.f));
		ReaderHeader = MakeText(TEXT(""), 22, Ink, true);
		ReadPane->AddChildToVerticalBox(ReaderHeader);
		UScrollBox* ReadScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
		Reader = MakeText(TEXT(""), 16, Ink);
		ReadScroll->AddChild(Reader);
		UVerticalBoxSlot* ScrollSlot = ReadPane->AddChildToVerticalBox(ReadScroll);
		ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ScrollSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));

		// Footer
		UBorder* Foot = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Foot->SetBrushColor(FLinearColor(0.90f, 0.93f, 0.95f));
		Foot->SetPadding(FMargin(24.f, 10.f));
		Foot->SetContent(MakeText(TEXT("Click an email to read it.     E / Tab  -  log out   (E answers your phone when it rings)"), 13, Muted));
		LoadTraining();
		RebuildSlide();
		Page->AddChildToVerticalBox(Foot);
	}
	return Super::RebuildWidget();
}

void UTakeTerminalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	if (Mails.Num() > 0) { ShowEmail(0); }
}

void UTakeTerminalWidget::ShowEmail(int32 Index)
{
	if (!Mails.IsValidIndex(Index) || !Reader || !ReaderHeader) { return; }
	const FMail& M = Mails[Index];
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->bReadEmail = true; }
	ReaderHeader->SetText(FText::FromString(M.Subject));
	Reader->SetText(FText::FromString(FString::Printf(TEXT("From: %s\n\n%s"), *M.Sender, *M.Body)));
}

FReply UTakeTerminalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.IsRepeat()) { return FReply::Handled(); }   // the E that logged you in is still held
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::E || InKeyEvent.GetKey() == EKeys::Tab)
	{
		OnLogoutRequested.Broadcast();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ================================================================ Compliance Training
UTakeIndexedButton* UTakeTerminalWidget::MakeButton(const FString& Text, int32 Index, const FLinearColor& Fill, const FLinearColor& TextColor)
{
	UTakeIndexedButton* B = WidgetTree->ConstructWidget<UTakeIndexedButton>(UTakeIndexedButton::StaticClass());
	B->SetBackgroundColor(Fill);
	B->Init(Index);
	UTextBlock* T = MakeText(Text, 15, TextColor, true);
	T->SetAutoWrapText(false);
	B->AddChild(T);
	return B;
}

int32 UTakeTerminalWidget::GetSlide() const
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GI ? GI->TrainingSlide : 0;
}

void UTakeTerminalWidget::SetSlide(int32 Slide)
{
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->TrainingSlide = Slide; }
}

void UTakeTerminalWidget::LoadTraining()
{
	Slides.Reset();
	if (const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ComplianceTraining.DT_ComplianceTraining")))
	{
		TArray<FTakeTrainingRow*> Rows;
		Table->GetAllRows<FTakeTrainingRow>(TEXT("TakeTraining"), Rows);
		Rows.Sort([](const FTakeTrainingRow& A, const FTakeTrainingRow& B) { return A.Order < B.Order; });
		for (const FTakeTrainingRow* R : Rows)
		{
			FSlide S{ TakeText::Fill(this, R->Title), TakeText::Fill(this, R->Body), R->Kind, {} };
			R->Options.ParseIntoArray(S.Options, TEXT("|"));
			Slides.Add(S);
		}
	}
	if (Slides.Num() == 0)   // fallback until the DataTable is imported
	{
		Slides.Add({ TEXT("Data Integrity Module 1"), TEXT("Welcome! This short module keeps our data, and you, intact."), TEXT("Slide"), {} });
		Slides.Add({ TEXT("The Integrity Rule"), TEXT("Look down each column of glyphs.\n\nIf an ODD number of your active fragments are lit there, the result is LIT.\nIf an EVEN number are lit (or none), the result is UNLIT."), TEXT("Slide"), {} });
		Slides.Add({ TEXT("Practice"), TEXT("Combine fragments until your RESULT matches the SIGNATURE."), TEXT("KeyLock"), {} });
		Slides.Add({ TEXT("Final Question"), TEXT("Do you consent to your next take?"), TEXT("Quiz"), { TEXT("Yes"), TEXT("Yes") } });
	}
	Lock = UTakeKeyLockLibrary::GenerateKeyLockForRun(this, 1, 0);
	GlyphLit.Reset();
	GlyphUnlit.Reset();
	for (int32 G = 0; G < 8; ++G)
	{
		const FString Base = FString::Printf(TEXT("/Game/UI/Glyphs/T_Glyph%d_%s_"), G, GlyphNames[G]);
		GlyphLit.Add(LoadObject<UTexture2D>(nullptr, *(Base + TEXT("Lit"))));
		GlyphUnlit.Add(LoadObject<UTexture2D>(nullptr, *(Base + TEXT("Unlit"))));
	}
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (TrainingTabLabel)
	{
		TrainingTabLabel->SetText(FText::FromString((GI && GI->bTrainingComplete) ? TEXT("Training  (done)") : TEXT("Training  (1 due)")));
	}
}

UHorizontalBox* UTakeTerminalWidget::MakeGlyphRow(int32 Value, int32 Bits, float Size, float Opacity)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	for (int32 B = 0; B < Bits; ++B)
	{
		const bool bLit = UTakeKeyLockLibrary::IsGlyphLit(Value, B);
		UTexture2D* Tex = bLit ? GlyphLit[B % 8] : GlyphUnlit[B % 8];
		UWidget* Cell = nullptr;
		if (Tex)
		{
			UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			Img->SetBrushFromTexture(Tex, false);
			Img->SetDesiredSizeOverride(FVector2D(Size, Size));
			Img->SetOpacity(Opacity);
			Cell = Img;
		}
		else   // textures not imported: plain marks
		{
			Cell = MakeText(bLit ? TEXT("\u25A0") : TEXT("\u25A1"), int32(Size * 0.6f), bLit ? Ink : Muted, true);
		}
		Row->AddChildToHorizontalBox(Cell)->SetPadding(FMargin(4.f, 2.f));
	}
	return Row;
}

void UTakeTerminalWidget::RebuildSlide()
{
	if (!TrainingBox) { return; }
	TrainingBox->ClearChildren();
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	auto Add = [this](UWidget* W, float Top = 0.f) { TrainingBox->AddChildToVerticalBox(W)->SetPadding(FMargin(0.f, Top, 0.f, 0.f)); };

	if (GI && GI->bTrainingComplete)
	{
		Add(MakeText(TEXT("HALCYON LEARNING"), 13, Mint, true));
		Add(MakeText(TEXT("Module complete."), 30, Ink, true), 6.f);
		Add(MakeText(TEXT("Thank you for your consent.\n\nYour integrity key has been recorded. Please return to your workflow.\nThere is nothing behind the walls."), 17, Ink), 18.f);
		return;
	}

	const int32 Index = FMath::Clamp(GetSlide(), 0, Slides.Num() - 1);
	const FSlide& S = Slides[Index];
	Add(MakeText(FString::Printf(TEXT("HALCYON LEARNING  \u00B7  DATA INTEGRITY MODULE 1  \u00B7  %d / %d"), Index + 1, Slides.Num()), 13, Mint, true));
	Add(MakeText(S.Title, 30, Ink, true), 6.f);
	Add(MakeText(S.Body, 17, Ink), 14.f);

	bool bCanContinue = true;
	if (S.Kind == TEXT("KeyLock"))
	{
		const int32 Bits = Lock.Bits;
		const int32 Result = UTakeKeyLockLibrary::CurrentResult(Lock, ActiveFragments);
		bLockSolved = UTakeKeyLockLibrary::EvaluateKeyLock(Lock, ActiveFragments) == ETakeLockResult::Solved;
		bCanContinue = bLockSolved;

		Add(MakeText(TEXT("SIGNATURE"), 13, Muted, true), 22.f);
		Add(MakeGlyphRow(Lock.Target, Bits, 52.f), 4.f);
		Add(MakeText(TEXT("FRAGMENTS  (click to switch on / off)"), 13, Muted, true), 18.f);
		for (int32 F = 0; F < Lock.Fragments.Num(); ++F)
		{
			const bool bOn = ActiveFragments.Contains(F);
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			UTakeIndexedButton* Toggle = MakeButton(bOn ? TEXT(" ON ") : TEXT(" OFF "), F, bOn ? Mint : Soft, bOn ? FLinearColor::White : Muted);
			Toggle->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::ToggleFragment);
			Row->AddChildToHorizontalBox(Toggle)->SetVerticalAlignment(VAlign_Center);
			Row->AddChildToHorizontalBox(MakeText(FString::Printf(TEXT("  Fragment %c  "), TCHAR('A' + F)), 15, Ink, true))->SetVerticalAlignment(VAlign_Center);
			Row->AddChildToHorizontalBox(MakeGlyphRow(Lock.Fragments[F], Bits, 40.f, bOn ? 1.f : 0.45f));
			Add(Row, 6.f);
		}
		Add(MakeText(TEXT("YOUR RESULT"), 13, Muted, true), 18.f);
		Add(MakeGlyphRow(Result, Bits, 52.f), 4.f);
		Add(MakeText(bLockSolved ? TEXT("Integrity confirmed. Thank you.") : TEXT("Not yet. Keep combining."), 16, bLockSolved ? Good : Warn, true), 10.f);
	}
	else if (S.Kind == TEXT("Quiz"))
	{
		bCanContinue = false;
		for (int32 O = 0; O < S.Options.Num(); ++O)
		{
			UTakeIndexedButton* Opt = MakeButton(S.Options[O], O, Soft, Ink);
			Opt->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::AnswerQuiz);
			Add(Opt, 10.f);
		}
		if (!QuizFeedback.IsEmpty()) { Add(MakeText(QuizFeedback, 15, Warn, true), 12.f); }
	}

	// Back / Next
	UHorizontalBox* Nav = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (Index > 0)
	{
		UTakeIndexedButton* Back = MakeButton(TEXT("  Back  "), -1, Soft, Ink);
		Back->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::TrainingNav);
		Nav->AddChildToHorizontalBox(Back)->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
	}
	if (bCanContinue)
	{
		UTakeIndexedButton* Next = MakeButton(TEXT("  Next  "), 1, Mint, FLinearColor::White);
		Next->OnIndexedClicked.AddDynamic(this, &UTakeTerminalWidget::TrainingNav);
		Nav->AddChildToHorizontalBox(Next);
	}
	Add(Nav, 26.f);
}

void UTakeTerminalWidget::ShowTab(int32 Tab)
{
	if (Pages) { Pages->SetActiveWidgetIndex(Tab); }
	if (Tab == 1) { RebuildSlide(); }
}

void UTakeTerminalWidget::TrainingNav(int32 Direction)
{
	QuizFeedback.Reset();
	const int32 Next = FMath::Clamp(GetSlide() + Direction, 0, Slides.Num() - 1);
	SetSlide(Next);
	RebuildSlide();
}

void UTakeTerminalWidget::ToggleFragment(int32 Fragment)
{
	if (ActiveFragments.Contains(Fragment)) { ActiveFragments.Remove(Fragment); }
	else { ActiveFragments.Add(Fragment); }
	RebuildSlide();
}

void UTakeTerminalWidget::AnswerQuiz(int32 Option)
{
	const int32 Index = GetSlide();
	if (!Slides.IsValidIndex(Index)) { return; }
	const FSlide& S = Slides[Index];
	const FString Picked = S.Options.IsValidIndex(Option) ? S.Options[Option] : FString();
	// The malware question has a right answer; HALCYON prefers you didn't find it.
	if (Picked.Contains(TEXT("Trust")))
	{
		QuizFeedback = TEXT("Correct! Every fragment is a friend.");
	}
	else if (Picked.Contains(TEXT("closer")))
	{
		QuizFeedback = TEXT("That answer is not on the list. Please choose again.");
		for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->StartCreep(FMath::Max(It->Fear, 0.12f), 2.f); break; }
		RebuildSlide();
		return;
	}
	if (Index >= Slides.Num() - 1) { FinishTraining(); return; }
	SetSlide(Index + 1);
	QuizFeedback.Reset();
	RebuildSlide();
}

void UTakeTerminalWidget::FinishTraining()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI && !GI->bTrainingComplete)
	{
		GI->bTrainingComplete = true;
		if (GI->SoulBits < 1) { GI->SetSoulBits(1); }   // Key 1: the first soul bit (pass 1, longer reach)
	}
	if (TrainingTabLabel) { TrainingTabLabel->SetText(FText::FromString(TEXT("Training  (done)"))); }
	if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UTakeGrabComponent* Hands = P->FindComponentByClass<UTakeGrabComponent>())
		{
			Hands->ShowSubtitle(TEXT("PA"), TEXT("Thank you for your consent."), 4.f);
		}
	}
	RebuildSlide();
}

// ================================================================ Inspect
void UTakeInspectWidget::Setup(UTexture2D* InTexture, const FString& InTitle, const FString& InCaption)
{
	Picture = InTexture;
	TitleText = InTitle;
	CaptionText = InCaption;
}

TSharedRef<SWidget> UTakeInspectWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Dim->SetBrushColor(FLinearColor(0.01f, 0.02f, 0.03f, 0.88f));
		UCanvasPanelSlot* DimSlot = Root->AddChildToCanvas(Dim);
		DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		DimSlot->SetOffsets(FMargin(0.f));

		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UCanvasPanelSlot* ColSlot = Root->AddChildToCanvas(Col);
		ColSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		ColSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ColSlot->SetAutoSize(true);

		if (Picture)
		{
			const float MaxH = 640.f, MaxW = 1100.f;
			const float W = float(FMath::Max(1, Picture->GetSizeX())), H = float(FMath::Max(1, Picture->GetSizeY()));
			const float Scale = FMath::Min(MaxH / H, MaxW / W);
			UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			Img->SetBrushFromTexture(Picture, false);
			Img->SetDesiredSizeOverride(FVector2D(W * Scale, H * Scale));
			UVerticalBoxSlot* ImgSlot = Col->AddChildToVerticalBox(Img);
			ImgSlot->SetHorizontalAlignment(HAlign_Center);
		}
		auto Line = [this](const FString& Text, int32 Size, const FLinearColor& Color, bool bBold)
		{
			UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			FSlateFontInfo F = T->GetFont();
			F.Size = Size;
			if (bBold) { F.TypefaceFontName = TEXT("Bold"); }
			T->SetFont(F);
			T->SetColorAndOpacity(FSlateColor(Color));
			T->SetJustification(ETextJustify::Center);
			T->SetAutoWrapText(true);
			T->SetText(FText::FromString(Text));
			return T;
		};
		if (!TitleText.IsEmpty())
		{
			Col->AddChildToVerticalBox(Line(TitleText, 20, FLinearColor(0.50f, 0.83f, 0.77f), true))->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));
		}
		if (!CaptionText.IsEmpty())
		{
			Col->AddChildToVerticalBox(Line(CaptionText, 16, FLinearColor(0.92f, 0.94f, 0.95f), false))->SetPadding(FMargin(0.f, 8.f, 0.f, 0.f));
		}
		Col->AddChildToVerticalBox(Line(TEXT("E / Esc / click  -  put it back"), 12, FLinearColor(0.6f, 0.65f, 0.7f), false))->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));
	}
	return Super::RebuildWidget();
}

FReply UTakeInspectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.IsRepeat()) { return FReply::Handled(); }
	const FKey K = InKeyEvent.GetKey();
	if (K == EKeys::E || K == EKeys::Escape || K == EKeys::Tab || K == EKeys::P)
	{
		OnCloseRequested.Broadcast();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UTakeInspectWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnCloseRequested.Broadcast();
	return FReply::Handled();
}
