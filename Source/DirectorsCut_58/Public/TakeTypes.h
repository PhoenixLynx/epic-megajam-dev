// TAKE 4471 - DataTable row types (CSV columns in Data/Text/*.csv must match these names)
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TakeTypes.generated.h"

/** Data/Text/DT_Emails.csv */
USTRUCT(BlueprintType)
struct FTakeEmailRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") int32 Act = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") FString Sender;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") FString Subject;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") FString Body;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") bool HasGlyphClue = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") bool IsPhishing = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Email") FString ArrivesAfter;
};

/** Data/Text/DT_NPCLines.csv */
USTRUCT(BlueprintType)
struct FTakeNPCLineRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC") FString Role;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC") int32 Act = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC") bool IsAgent = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC") FString Context;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC") FString Line;
};

/** Data/Text/DT_ComplianceTraining.csv */
USTRUCT(BlueprintType)
struct FTakeTrainingRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Training") int32 Order = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Training") FString Title;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Training") FString Body;
	/** Slide, KeyLock or Quiz */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Training") FString Kind;
	/** Quiz answers separated by | */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Training") FString Options;
};

namespace TakeText
{
	/** Fill {EMP}, {TAKE} and glyph placeholders. */
	DIRECTORSCUT_58_API FString Fill(const UObject* WorldContext, const FString& In);
}
