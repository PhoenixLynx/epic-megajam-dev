// TAKE 4471 - UI built entirely in C++ (no Widget Blueprints needed):
//   UTakeIndexedButton  - a button that reports its index when clicked
//   UTakePromptWidget   - "E  Use terminal" prompt under the crosshair
//   UTakeTerminalWidget - HALCYON Mail on desk #4471 (reads /Game/Data/DT_Emails)
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "TakeWidgets.generated.h"

class UTextBlock;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeIndexedClicked, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakeLogoutRequested);

UCLASS()
class DIRECTORSCUT_58_API UTakeIndexedButton : public UButton
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 Index = 0;
	UPROPERTY(BlueprintAssignable) FTakeIndexedClicked OnIndexedClicked;
	void Init(int32 InIndex);
private:
	UFUNCTION() void HandleClicked();
};

UCLASS()
class DIRECTORSCUT_58_API UTakePromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPrompt(const FString& Text);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	UPROPERTY() TObjectPtr<UTextBlock> Label;
	FString Current;
};

UCLASS()
class DIRECTORSCUT_58_API UTakeTerminalWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeLogoutRequested OnLogoutRequested;
	UFUNCTION() void ShowEmail(int32 Index);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
private:
	struct FMail { FString Sender, Subject, Body; bool bPhishing = false; };
	TArray<FMail> Mails;
	UPROPERTY() TObjectPtr<UTextBlock> Reader;
	UPROPERTY() TObjectPtr<UTextBlock> ReaderHeader;
	void LoadMail();
	UTextBlock* MakeText(const FString& Text, int32 Size, const FLinearColor& Color, bool bBold = false);
};
