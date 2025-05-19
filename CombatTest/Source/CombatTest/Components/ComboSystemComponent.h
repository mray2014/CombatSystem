// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComboSystemComponent.generated.h"

class UInputAction;
class UAnimMontage;
class UAnimInstance;

UENUM()
enum class EAttackWaitPolicy : uint8
{
	None,
	WaitForNotify,
	WaitForMontageEnd
};

UCLASS()
class UComboNode : public UObject
{
	GENERATED_BODY()

	UComboNode* ParentNode = nullptr;

public:
	UPROPERTY()
	TArray<UComboNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadWrite)
	FString ComboName;
	UPROPERTY(BlueprintReadWrite)
	bool bIsEndOfCombo;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UInputAction> IA_Trigger;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
	UPROPERTY(BlueprintReadWrite)
	bool bUsesRootMotion;
	UPROPERTY(BlueprintReadWrite)
	EAttackWaitPolicy AttackWaitPolicy;

	UComboNode()
	{
	}
	UComboNode(UComboNode* Parent)
	{
		ParentNode = Parent;
	}

	UFUNCTION(BlueprintCallable)
	UComboNode* AddComboNode(UInputAction* TriggerAction, UAnimMontage* Montage, bool bComboEnd, EAttackWaitPolicy WaitPolicy)
	{
		UComboNode* NewNode = NewObject<UComboNode>();
		NewNode->IA_Trigger = TriggerAction;
		NewNode->MontageToPlay = Montage;
		NewNode->bIsEndOfCombo = bComboEnd;
		NewNode->AttackWaitPolicy = WaitPolicy;
		ChildrenNodes.Add(NewNode);

		return NewNode;
	}

	void DestroyNode()
	{
		for (UComboNode* Node : ChildrenNodes)
		{
			Node->DestroyNode();
		}

		//this->ConditionalBeginDestroy();
	}
};

UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class UComboGraphComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	TArray<UComboNode*> ComboGraphStartingNodes;

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageBeginNotifyDelegate, FName, NotifyName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageEndedDelegate);

	UPROPERTY(BlueprintAssignable)
	FMontageBeginNotifyDelegate MontageBeginNotify;

	UPROPERTY(BlueprintAssignable)
	FMontageEndedDelegate MontageEnded;

	UPROPERTY(BlueprintReadWrite)
	FString ComboGraphName;

	UFUNCTION(BlueprintCallable)
	const FString GetComboGraphName() const
	{
		return ComboGraphName;
	}

	UFUNCTION(BlueprintCallable)
	const TArray<UComboNode*>& GetComboGraphStartingNodes() const
	{
		return ComboGraphStartingNodes;
	}

	UFUNCTION(BlueprintCallable)
	UComboNode* StartNewComboSequence(UInputAction* TriggerAction, UAnimMontage* MontageToPlay, bool bComboEnd, EAttackWaitPolicy WaitPolicy)
	{
		UComboNode* NewNode = NewObject<UComboNode>();
		NewNode->MontageToPlay = MontageToPlay;
		NewNode->IA_Trigger = TriggerAction;
		NewNode->bIsEndOfCombo = bComboEnd;
		NewNode->AttackWaitPolicy = WaitPolicy;

		ComboGraphStartingNodes.Add(NewNode);

		return NewNode;
	}
};


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class COMBATTEST_API UComboSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UComboSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool TryToPlayComboNode(UComboNode* Node, UInputAction* TriggerAction);

	UFUNCTION()
	void OnBeginMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	TMap<FString, UComboGraphComponent*> ComboGraphMap;

	TObjectPtr<UComboGraphComponent> CurrentActiveComboGraph;
	TArray<UComboNode*> CurrentActiveComboGraphStartingNodes;

	TObjectPtr<UComboNode> CurrentComboNode;

	TObjectPtr<USkeletalMeshComponent> CurrentSkeletalMesh;

	TObjectPtr<UAnimInstance> CurrentMeshAnimInstance;

	bool bCanAttack = true;

public:
	UFUNCTION(BlueprintCallable)
	void InitializeComboSystem(USkeletalMeshComponent* SkeletalMesh);

	UFUNCTION(BlueprintCallable)
	void AddComboGraph(FString ComboGraphName, UComboGraphComponent* ComboGraph);

	UFUNCTION(BlueprintCallable)
	void ActivateComboGraph(FString ComboGraphName);

	UFUNCTION(BlueprintCallable)
	void AddAndActivateComboGraph(FString ComboGraphName, UComboGraphComponent* ComboGraph);

	UFUNCTION(BlueprintCallable)
	void ProcessInputEvent(UInputAction* TriggerAction);
		
};
