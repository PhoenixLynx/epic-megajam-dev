// TAKE 4471 - title screen, the ending's choice, and the credits
#include "TakeAct3Widgets.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

namespace
{
	const FLinearColor TInk(0.05f, 0.09f, 0.11f);
	const FLinearColor TMint(0.50f, 0.83f, 0.77f);

	UTextBlock* Txt(UWidgetTree* Tree, const FString& S, int32 Size, const FLinearColor& C, bool bBold = false, int32 Spacing = 0)
	{
		UTextBlock* T = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FSlateFontInfo F = T->GetFont();
		F.Size = Size;
		if (bBold) { F.TypefaceFontName = TEXT("Bold"); }
		F.LetterSpacing = Spacing;
		T->SetFont(F);
		T->SetColorAndOpacity(FSlateColor(C));
		T->SetText(FText::FromString(S));
		T->SetJustification(ETextJustify::Center);
		return T;
	}

	UTakeIndexedButton* Btn(UWidgetTree* Tree, const FString& Label, int32 Index, bool bPrimary)
	{
		UTakeIndexedButton* B = Tree->ConstructWidget<UTakeIndexedButton>(UTakeIndexedButton::StaticClass());
		B->SetBackgroundColor(bPrimary ? TMint : FLinearColor(0.95f, 0.97f, 0.97f, 0.92f));
		B->Init(Index);
		UTextBlock* T = Txt(Tree, FString::Printf(TEXT("   %s   "), *Label), 20, bPrimary ? FLinearColor::White : TInk, true, 120);
		B->AddChild(T);
		return B;
	}

	UBorder* FullScreen(UWidgetTree* Tree, UCanvasPanel* Root, const FLinearColor& C)
	{
		UBorder* B = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
		B->SetBrushColor(C);
		UCanvasPanelSlot* S = Root->AddChildToCanvas(B);
		S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		S->SetOffsets(FMargin(0.f));
		return B;
	}
}

// ---------------------------------------------------------------- Title
TSharedRef<SWidget> UTakeTitleWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		FullScreen(WidgetTree, Root, FLinearColor::Black);
		if (UTexture2D* Art = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Props/Textures/T_TitleKeyArt.T_TitleKeyArt")))
		{
			UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			Img->SetBrushFromTexture(Art, false);
			UCanvasPanelSlot* S = Root->AddChildToCanvas(Img);
			S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			S->SetOffsets(FMargin(0.f));
		}
		// A soft white band on the left for the title and menu
		UBorder* Band = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Band->SetBrushColor(FLinearColor(0.97f, 0.98f, 0.99f, 0.82f));
		Band->SetPadding(FMargin(56.f, 48.f));
		UCanvasPanelSlot* BandSlot = Root->AddChildToCanvas(Band);
		BandSlot->SetAnchors(FAnchors(0.06f, 0.5f));
		BandSlot->SetAlignment(FVector2D(0.f, 0.5f));
		BandSlot->SetAutoSize(true);
		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		Band->SetContent(Col);
		Col->AddChildToVerticalBox(Txt(WidgetTree, TEXT("HALCYON PRESENTS"), 14, FLinearColor(0.33f, 0.40f, 0.45f), true, 400));
		Col->AddChildToVerticalBox(Txt(WidgetTree, TEXT("TAKE 4471"), 72, TInk, true, 200))->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
		Col->AddChildToVerticalBox(Txt(WidgetTree, TEXT("You are exactly where you need to be."), 16, FLinearColor(0.33f, 0.40f, 0.45f)))->SetPadding(FMargin(0.f, 4.f, 0.f, 28.f));
		struct FItem { const TCHAR* Label; int32 Id; bool bShow; };
		const FItem Items[] = { { TEXT("START"), 0, true }, { TEXT("CONTINUE"), 1, bHasSave }, { TEXT("QUIT"), 2, true } };
		for (const FItem& It : Items)
		{
			if (!It.bShow) { continue; }
			UTakeIndexedButton* B = Btn(WidgetTree, It.Label, It.Id, It.Id == 0);
			B->OnIndexedClicked.AddDynamic(this, &UTakeTitleWidget::Pick);
			Col->AddChildToVerticalBox(B)->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
		}
		Col->AddChildToVerticalBox(Txt(WidgetTree, TEXT("Made with Unreal Engine  ·  Epic AI MegaJam 2026"), 12, FLinearColor(0.33f, 0.40f, 0.45f)))->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- Choice
TSharedRef<SWidget> UTakeChoiceWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		FullScreen(WidgetTree, Root, FLinearColor(0.f, 0.f, 0.f, 0.55f));
		UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UCanvasPanelSlot* ColSlot = Root->AddChildToCanvas(Col);
		ColSlot->SetAnchors(FAnchors(0.5f, 0.62f));
		ColSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ColSlot->SetAutoSize(true);
		Col->AddChildToVerticalBox(Txt(WidgetTree, Question, 34, FLinearColor::White, true, 200))->SetHorizontalAlignment(HAlign_Center);
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UTakeIndexedButton* Yes = Btn(WidgetTree, TEXT("YES"), 0, true);
		Yes->OnIndexedClicked.AddDynamic(this, &UTakeChoiceWidget::Pick);
		UTakeIndexedButton* No = Btn(WidgetTree, TEXT("NO"), 1, false);
		No->OnIndexedClicked.AddDynamic(this, &UTakeChoiceWidget::Pick);
		Row->AddChildToHorizontalBox(Yes)->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
		Row->AddChildToHorizontalBox(No);
		UVerticalBoxSlot* RowSlot = Col->AddChildToVerticalBox(Row);
		RowSlot->SetHorizontalAlignment(HAlign_Center);
		RowSlot->SetPadding(FMargin(0.f, 28.f, 0.f, 0.f));
	}
	return Super::RebuildWidget();
}

// ---------------------------------------------------------------- Credits
TSharedRef<SWidget> UTakeCreditsWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
		FullScreen(WidgetTree, Root, FLinearColor::Black);
		UVerticalBox* Roll = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		RollSlot = Root->AddChildToCanvas(Roll);
		RollSlot->SetAnchors(FAnchors(0.5f, 1.f));
		RollSlot->SetAlignment(FVector2D(0.5f, 0.f));
		RollSlot->SetAutoSize(true);
		const FLinearColor Dim(0.6f, 0.66f, 0.68f);
		auto Add = [&](const FString& S, int32 Size, const FLinearColor& C, bool bBold, float Gap)
		{
			Roll->AddChildToVerticalBox(Txt(WidgetTree, S, Size, C, bBold, bBold ? 150 : 0))->SetPadding(FMargin(0.f, Gap, 0.f, 0.f));
		};
		Add(TEXT("TAKE 4471"), 56, FLinearColor::White, true, 0.f);
		Add(TEXT("a HALCYON production"), 18, Dim, false, 6.f);
		Add(TEXT("EMPLOYEE 4471"), 16, TMint, true, 70.f);   Add(TEXT("You"), 26, FLinearColor::White, false, 4.f);
		Add(TEXT("EMPLOYEE 4470"), 16, TMint, true, 34.f);   Add(TEXT("Also you"), 26, FLinearColor::White, false, 4.f);
		Add(TEXT("COWORKERS"), 16, TMint, true, 34.f);       Add(TEXT("As cast"), 26, FLinearColor::White, false, 4.f);
		Add(TEXT("THE CHAIRMAN"), 16, TMint, true, 34.f);    Add(TEXT("(camera off)"), 26, FLinearColor::White, false, 4.f);
		Add(TEXT("GAME, STORY, MUSIC"), 16, TMint, true, 70.f);   Add(TEXT("PhoenixLynx"), 26, FLinearColor::White, false, 4.f);
		Add(TEXT("CODE, LEVEL, SOUND DESIGN"), 16, TMint, true, 34.f);   Add(TEXT("Claude (Anthropic), with PhoenixLynx"), 22, FLinearColor::White, false, 4.f);
		Add(TEXT("ART AND FACES"), 16, TMint, true, 34.f);   Add(TEXT("Sol (ChatGPT), with PhoenixLynx"), 22, FLinearColor::White, false, 4.f);
		Add(TEXT("Made with Unreal Engine for the Epic AI MegaJam 2026"), 18, Dim, false, 60.f);
		Add(TEXT("Fonts: Poppins, Inter, Caveat (SIL OFL) · Permanent Marker (Apache 2.0)"), 14, Dim, false, 10.f);
		Add(TEXT("DIRECTED BY"), 16, TMint, true, 120.f);   Add(TEXT("________________"), 26, FLinearColor::White, false, 8.f);
		Add(TEXT("Thank you for your energy."), 20, Dim, false, 120.f);
	}
	return Super::RebuildWidget();
}

void UTakeCreditsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	T += InDeltaTime;
	if (RollSlot) { RollSlot->SetPosition(FVector2D(0.f, -60.f * T)); }   // 60 px a second, up from the bottom
	if (T > 34.f) { Finish(); }
}

FReply UTakeCreditsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (T > 3.f) { Finish(); }
	return FReply::Handled();
}

FReply UTakeCreditsWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (T > 3.f) { Finish(); }
	return FReply::Handled();
}

void UTakeCreditsWidget::Finish()
{
	if (bDone) { return; }
	bDone = true;
	OnChoice.Broadcast(0);
}
