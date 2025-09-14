#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SurvivalPlayerController.generated.h"

class UInputMappingContext;

UCLASS(abstract)
class SURVIVAL_API ASurvivalPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ASurvivalPlayerController();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input", meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> DefaultMappingContexts;

	virtual void SetupInputComponent() override;
};
