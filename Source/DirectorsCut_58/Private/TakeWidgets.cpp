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
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"

namespace
{
	const FLinearColor Ink(0.05f, 0.09f, 0.11f);
	const FLinearColor Muted(0.33f, 0.40f, 0.45f);
	const FLinearColor Mint(0.50f, 0.83f, 0.77f);
	const FLinearColor Paper(0.97f, 0.98f, 0.99f, 0.98f);
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
	}
	return Super::RebuildWidget();
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

		// Inbox list + reader
		UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* BodySlot = Page->AddChildToVerticalBox(Body);
		BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		BodySlot->SetPadding(FMargin(16.f));

		USizeBox* ListBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ListBox->SetWidthOverride(420.f);
		UScrollBox* List = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
		ListBox->AddChild(List);
		Body->AddChildToHorizontalBox(ListBox);
		for (int32 I = 0; I < Mails.Num(); ++I)
		{
			UTakeIndexedButton* B = WidgetTree->ConstructWidget<UTakeIndexedButton>(UTakeIndexedButton::StaticClass());
			B->SetBackgroundColor(FLinearColor(0.93f, 0.96f, 0.97f));
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
		Foot->SetContent(MakeText(TEXT("Click an email to read it.     E / Tab  -  log out"), 13, Muted));
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
