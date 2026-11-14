// TAKE 4471 - pause menu (Esc in a packaged build, P anywhere). Built in C++, no Widget Blueprint.
//   Resume · Tasks · Settings (volume, graphics, window, controls) · Save · Load · Quit
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TakePauseMenu.generated.h"

class UTextBlock;
class UVerticalBox;
class UWidgetSwitcher;
class UTakeIndexedButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakeResumeRequested);

UCLASS()
class DIRECTORSCUT_58_API UTakePauseMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeResumeRequested OnResumeRequested;

	UFUNCTION() void OnMenu(int32 Item);
	UFUNCTION() void OnQuality(int32 Level);
	UFUNCTION() void OnWindowMode(int32 Mode);
	UFUNCTION() void OnVolume(float Value);
	UFUNCTION() void OnDialogueMode(int32 Mode);

	/** Save / load the workday. Static so other code (autosave) can call them too. */
	static bool SaveProgress(const UObject* WorldContext);
	static bool LoadProgress(const UObject* WorldContext);
	/** Apply the saved master volume (call at BeginPlay). */
	static void ApplySavedSettings();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UPROPERTY() TObjectPtr<UWidgetSwitcher> Pages;
	UPROPERTY() TObjectPtr<UTextBlock> TasksText;
	UPROPERTY() TObjectPtr<UTextBlock> StatusText;
	UPROPERTY() TObjectPtr<UTextBlock> SaveInfo;
	UTextBlock* MakeText(const FString& Text, int32 Size, const FLinearColor& Color, bool bBold = false);
	UTakeIndexedButton* MakeButton(const FString& Text, int32 Index, bool bPrimary = false);
	void RefreshTasks();
	void RefreshSaveInfo();
	void SetStatus(const FString& Text);
};
