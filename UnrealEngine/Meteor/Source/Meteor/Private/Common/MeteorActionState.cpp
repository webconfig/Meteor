
#include "Common/MeteorActionState.h"
#include "Components/InputCommandComponent.h"
#include "Components/CombatSystemComponent.h"

#include "AttackCharacter.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

FActionState::FActionState(AAttackCharacter* owner)
	: Owner(owner)
{
	InputCommandCP = owner->GetInputCommandComponent();
	CombatSystemCP = owner->GetCombatSystemComponent();

	AnimInstance = owner->GetMesh()->GetAnimInstance();
}

FIdleState::FIdleState(AAttackCharacter* owner)
	: FActionState(owner)
{

}

void FIdleState::Enter()
{
	InputCommandCP->CurrentStateNo = -1;
	InputCommandCP->RecreateNextPoses(-1);
}

void FIdleState::Execute(float DeltaTime)
{
	// Attack
	if (InputCommandCP->CurrentStateNo != -1)
	{
		InputCommandCP->bHasControl = false;
		InputCommandCP->bBeginChain = false;

		FAttackState* CombatAttack = CombatSystemCP->ActoinStateFactory->CreateAttackState(Owner);
		CombatAttack->Init(InputCommandCP->CurrentStateNo, ATTACK_ANIM_STATE::ANIM_PLYING, 0.0f);
		CombatSystemCP->ChangeActionState(CombatAttack);
		InputCommandCP->CurrentStateNo = -1;
		return;
	}
	else
	{
		InputCommandCP->bHasControl = true;
		InputCommandCP->bBeginChain = true;
	}

	// Jump / 在移动过程中产生falling
	if (InputCommandCP->bJumpKeyDown || Owner->GetCharacterMovement()->IsFalling())
	{
		FJumpState* Jump = CombatSystemCP->ActoinStateFactory->CreateJumpState(Owner);
		CombatSystemCP->ChangeActionState(Jump);
		return;
	}

	// Crouch

	// Guard
	if (InputCommandCP->bGuardKeyDown && !Owner->GetCharacterMovement()->IsFalling())
	{
		FGuardState* Guard = CombatSystemCP->ActoinStateFactory->CreateGuardState(Owner);
		Guard->Init(InputCommandCP->GuardPoseIndex);
		CombatSystemCP->ChangeActionState(Guard);
		return;
	}
}

void FIdleState::Exit()
{
	
}

void FIdleState::Clear()
{

}

FAttackState::FAttackState(AAttackCharacter* owner)
	: FActionState(owner)
{
	PoseLockFrame = -1;
	bLockFrameFlag = false;
	PoseIndex = -1;
	PoseMtg = nullptr;
	PoseSectionInfo = nullptr;
}

void FAttackState::Init(int32 Pose, ATTACK_ANIM_STATE AnimState, float TransitTIme)
{
	PoseIndex = Pose;
	EnterAnimState = AnimState;
	AnimTransitTime = TransitTIme;

	PoseMtg = CombatSystemCP->GetPoseMontage(PoseIndex);
	PoseSectionInfo = CombatSystemCP->GetAnimMetaData(PoseMtg);

	PoseLockFrame = InputCommandCP->GetPoseLockFrame(PoseIndex);
	if (PoseLockFrame != -1)
	{
		bLockFrameFlag = true;
	}
}

void FAttackState::Enter()
{
	if (PoseMtg == nullptr)
	{
		FIdleState* idleState = CombatSystemCP->ActoinStateFactory->CreateIdleState(Owner);
		CombatSystemCP->ChangeActionState(idleState);
		return;
	}

	Owner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_BEGIN)
	{
		CacheBlendIn = PoseMtg->BlendIn;
		PoseMtg->BlendIn.SetBlendTime(AnimTransitTime);
		AnimInstance->Montage_Play(PoseMtg);
		AnimInstance->Montage_JumpToSection(PoseSectionInfo->NextPoseIn, PoseMtg);
		AnimInstance->Montage_Pause(PoseMtg);
		AnimTransitTimer = 0.0f;
	}
	else if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_LINK)
	{
		CacheBlendIn = PoseMtg->BlendIn;
		PoseMtg->BlendIn.SetBlendTime(CombatSystemCP->FrameLinkTime);
		AnimInstance->Montage_Play(PoseMtg);
		AnimInstance->Montage_Pause(PoseMtg);
		AnimTransitTimer = 0.0f;
	}
	else if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_PLYING)
	{
		AnimInstance->Montage_Stop(0.0f);
		AnimInstance->Montage_Play(PoseMtg);
		CacheBlendIn = PoseMtg->BlendIn;
	}

	if (bLockFrameFlag)
	{
		AnimInstance->Montage_Pause(PoseMtg);
	}

	FPoseStateInfo* PoseInfo = InputCommandCP->GetPoseStateInfo(PoseIndex);
	ActionStateDef* ActionDef = InputCommandCP->GetActionStateInfo(PoseIndex);


	if (PoseInfo->Link != -1)
	{
		FOnMontageBlendingOutStarted BlendStartDele;
		BlendStartDele.BindUObject(CombatSystemCP, &UCombatSystemComponent::OnPoseMontageEnded);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendStartDele, PoseMtg);
	}
	else
	{
		FOnMontageEnded EndedDel;
		EndedDel.BindUObject(CombatSystemCP, &UCombatSystemComponent::OnPoseMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndedDel, PoseMtg);
	}
}

void FAttackState::Execute(float DeltaTime)
{
	UAnimMontage* ActiveAnimMtg = AnimInstance->GetCurrentActiveMontage();
	
	if (ActiveAnimMtg && ActiveAnimMtg == PoseMtg)
	{
		// Combat 过渡
		if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_BEGIN)
		{
			AnimTransitTimer += DeltaTime;
			if (AnimTransitTimer > AnimTransitTime)
			{
				AnimInstance->Montage_Resume(PoseMtg);
				EnterAnimState = ATTACK_ANIM_STATE::ANIM_PLYING;
			}
		}

		if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_LINK)
		{
			AnimTransitTimer += DeltaTime;
			if (AnimTransitTimer > AnimTransitTime)
			{
				AnimInstance->Montage_Resume(PoseMtg);
				EnterAnimState = ATTACK_ANIM_STATE::ANIM_PLYING;
			}
		}

		if (bLockFrameFlag)
		{
			if (Owner->GetCharacterMovement()->IsFalling())
			{
				AnimInstance->Montage_Pause(PoseMtg);
				return;
			}
			else
			{
				AnimInstance->Montage_Resume(PoseMtg);
				bLockFrameFlag = false;
			}
		}

		// 正常的Playing
		if (EnterAnimState == ATTACK_ANIM_STATE::ANIM_PLYING)
		{
			FName sectionName = AnimInstance->Montage_GetCurrentSection(PoseMtg);
			if (PoseSectionInfo)
			{
				if (sectionName.IsEqual(PoseSectionInfo->NextPoseOut))
				{
					InputCommandCP->bBeginChain = true; // 有一帧的滞后
					if (InputCommandCP->CurrentStateNo != -1)
					{
						InputCommandCP->bBeginChain = false;
						InputCommandCP->bHasControl = false;

						FAttackState* CombatAttack = CombatSystemCP->ActoinStateFactory->CreateAttackState(Owner);
						CombatAttack->Init(InputCommandCP->CurrentStateNo, ATTACK_ANIM_STATE::ANIM_BEGIN, PoseSectionInfo->NextPoseTime);
						CombatSystemCP->ChangeActionState(CombatAttack);
						InputCommandCP->CurrentStateNo = -1;
						return;
					}
				}

				float* ratio = PoseSectionInfo->SectionSpeeds.Find(sectionName);
				if (ratio)
				
					AnimInstance->Montage_SetPlayRate(ActiveAnimMtg, *ratio);
				else
					AnimInstance->Montage_SetPlayRate(ActiveAnimMtg, 1.0f);
			}
		}

		// HitPause

		// 
	}
}

void FAttackState::Exit()
{
	if (PoseMtg)
	{
		PoseMtg->BlendIn = CacheBlendIn;
	}
}

void FAttackState::Clear()
{
	FActionState::Clear();
	
	PoseIndex = -1;
	PoseMtg = nullptr;
	PoseSectionInfo = nullptr;
	PoseLockFrame = -1;
	bLockFrameFlag = false;

	EnterAnimState = ATTACK_ANIM_STATE::ANIM_PLYING;
	AnimTransitTime = 0.0f;
	AnimTransitTimer = 0.0f;
}

FGuardState::FGuardState(AAttackCharacter* owner)
	: FActionState(owner)
{
	PoseIndex = -1;
	GuardMtg = nullptr;
}

void FGuardState::Init(int32 Pose)
{
	PoseIndex = Pose;
	GuardMtg = CombatSystemCP->GetPoseMontage(PoseIndex);
}

void FGuardState::Enter()
{
	AnimInstance->Montage_Play(GuardMtg, 1.0f);
	InputCommandCP->bHasControl = false;
}

void FGuardState::Execute(float DeltaTime)
{
	if (!InputCommandCP->bGuardKeyDown)
	{
		AnimInstance->Montage_Stop(0.2f, GuardMtg);
		FIdleState* idleState = CombatSystemCP->ActoinStateFactory->CreateIdleState(Owner);
		CombatSystemCP->ChangeActionState(idleState);
	}
}

void FGuardState::Exit()
{
	InputCommandCP->bHasControl = true;
}

void FGuardState::Clear()
{
	PoseIndex = -1;
	GuardMtg = nullptr;
}

FJumpState::FJumpState(AAttackCharacter* owner)
	: FActionState(owner)
{

}

void FJumpState::Enter()
{

}

void FJumpState::Execute(float DeltaTime)
{
	// Attack
	if (InputCommandCP->CurrentStateNo != -1)
	{
		InputCommandCP->bHasControl = false;
		InputCommandCP->bBeginChain = false;

		FAttackState* CombatAttack = CombatSystemCP->ActoinStateFactory->CreateAttackState(Owner);
		CombatAttack->Init(InputCommandCP->CurrentStateNo, ATTACK_ANIM_STATE::ANIM_PLYING, 0.0f);
		CombatSystemCP->ChangeActionState(CombatAttack);
		InputCommandCP->CurrentStateNo = -1;
	}
	else
	{
		InputCommandCP->bHasControl = true;
		InputCommandCP->bBeginChain = true;
	}

	if (Owner->GetCharacterMovement()->IsFalling() == false)
	{
		FIdleState* idleState = CombatSystemCP->ActoinStateFactory->CreateIdleState(Owner);
		CombatSystemCP->ChangeActionState(idleState);
	}
}

void FJumpState::Exit()
{

}

void FJumpState::Clear()
{

}
