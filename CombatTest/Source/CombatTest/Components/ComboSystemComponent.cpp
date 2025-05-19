// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatTest/Components/ComboSystemComponent.h"
#include "InputAction.h"
#include "Runtime/Engine/Classes/Animation/AnimMontage.h"

//DEFINE_LOG_CATEGORY(LogComboSystemComponent)

// Sets default values for this component's properties
UComboSystemComponent::UComboSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UComboSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UComboSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ComboGraphMap.Empty();
}

void UComboSystemComponent::InitializeComboSystem(USkeletalMeshComponent* SkeletalMesh)
{
	CurrentSkeletalMesh = SkeletalMesh;
	CurrentMeshAnimInstance = CurrentSkeletalMesh->GetAnimInstance();

	CurrentMeshAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UComboSystemComponent::OnBeginMontageNotify);
	CurrentMeshAnimInstance->OnMontageEnded.AddDynamic(this, &UComboSystemComponent::OnMontageEnded);
}

void UComboSystemComponent::OnBeginMontageNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (CurrentComboNode)
	{
		if (CurrentComboNode->AttackWaitPolicy == EAttackWaitPolicy::WaitForNotify && NotifyName == "CanAttack")
		{
			bCanAttack = true;
		}
	}

	if (CurrentActiveComboGraph)
	{
		CurrentActiveComboGraph->MontageBeginNotify.Broadcast(NotifyName);
	}
}

void UComboSystemComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		bCanAttack = true;

		if (CurrentComboNode && CurrentComboNode->bIsEndOfCombo)
		{
			CurrentComboNode = nullptr;
		}

		if (CurrentActiveComboGraph)
		{
			CurrentActiveComboGraph->MontageEnded.Broadcast();
		}
	}
}

void UComboSystemComponent::AddComboGraph(FString ComboGraphName, UComboGraphComponent* ComboGraph)
{
	if (!ComboGraphMap.Contains(ComboGraphName))
	{
		ComboGraphMap.Add(ComboGraphName, ComboGraph);
	}
	else
	{
		//UE_LOG(LogComboSystemComponent, Verbose, TEXT("%s Added %s Combo Graph to Combo System"), __FUNCTIONW__, ComboGraphName);
	}
}

void UComboSystemComponent::ActivateComboGraph(FString ComboGraphName)
{
	if (ComboGraphMap.Contains(ComboGraphName))
	{
		CurrentActiveComboGraph = ComboGraphMap[ComboGraphName];
		CurrentActiveComboGraphStartingNodes = CurrentActiveComboGraph->GetComboGraphStartingNodes();
	}
	else
	{
		//UE_LOG();
	}
}

void UComboSystemComponent::AddAndActivateComboGraph(FString ComboGraphName, UComboGraphComponent* ComboGraph)
{
	if (ComboGraphMap.Contains(ComboGraphName))
	{
		CurrentActiveComboGraph = ComboGraphMap[ComboGraphName];
		CurrentActiveComboGraphStartingNodes = CurrentActiveComboGraph->GetComboGraphStartingNodes();
	}
	else
	{
		AddComboGraph(ComboGraphName, ComboGraph);
		CurrentActiveComboGraph = ComboGraph;
		CurrentActiveComboGraphStartingNodes = CurrentActiveComboGraph->GetComboGraphStartingNodes();
		//UE_LOG();
	}
}

bool UComboSystemComponent::TryToPlayComboNode(UComboNode* Node, UInputAction* TriggerAction)
{
	if (Node->IA_Trigger && Node->IA_Trigger == TriggerAction)
	{
		if (Node->MontageToPlay)
		{
			if (CurrentMeshAnimInstance)
			{
				if (Node->AttackWaitPolicy != EAttackWaitPolicy::None)
				{
					bCanAttack = false;
				}
				else
				{
					bCanAttack = true;
				}
				 
				CurrentMeshAnimInstance->Montage_Play(Node->MontageToPlay);

				CurrentComboNode = Node;

				return true;
			}
		}
	}

	return false;
}

void UComboSystemComponent::ProcessInputEvent(UInputAction* TriggerAction)
{
	if (bCanAttack)
	{
		if (CurrentSkeletalMesh)
		{
			if (CurrentComboNode)
			{
				for (UComboNode* Node : CurrentComboNode->ChildrenNodes)
				{
					if (TryToPlayComboNode(Node, TriggerAction))
					{
						return;
					}
				}
			}
			else
			{
				if (CurrentActiveComboGraphStartingNodes.Num() > 0)
				{
					for (UComboNode* Node : CurrentActiveComboGraphStartingNodes)
					{
						if (TryToPlayComboNode(Node, TriggerAction))
						{
							return;
						}
					}
				}
			}
		}
	}
}
