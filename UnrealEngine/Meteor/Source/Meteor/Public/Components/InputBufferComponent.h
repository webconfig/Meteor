// Copyright IceRiver. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBufferComponent.generated.h"

/**
* 用来管理玩家的输入，输出攻击指令
* 当前按键是否hold
* InputBuffer 产生的攻击命令，可能是 L-U-A U-U-A 或者 U-U
* 如果在当前InputBuffer中，已经有A键，则将其以前的键为删除 即 A-R-A -> 应该删除掉A，只保留R-A
* 可能在同一帧内同时按下多个键，所以InputBuffer中保留的是有效按键帧的数据
* 
* 属于按键判定，不应该卸载InputBuffer中
* 	Sprinting前期，只能接受 UUA的攻击指令 或者D-D-A类似的指令，其他指令都不应该产生
* 	按键滞留，即如果一直按着Up键移动，再按下Attack键，可以产生 U-A的按键效果
* 将KEY_A存InputBuffer中删除，其作为触发InputBuffer查询的位置
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class METEOR_API UInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	enum class ATTACK_KEY
	{
		KEY_U = 0,
		KEY_D,
		KEY_R,
		KEY_L,
		KEY_NUM,
	};

	struct FrameInputKey
	{
		FrameInputKey();
		FrameInputKey(ATTACK_KEY key, float time);
		void SetKeyDown(ATTACK_KEY key);
		bool GetValidChars(TArray<TCHAR>& chars) const;
		bool GetValidStr(FString& str) const;
		void Reset();

		float timeStamp;

		bool L_Down;
		bool R_Down;
		bool U_Down;
		bool D_Down;
	};

	struct PushKeyRecord
	{
		ATTACK_KEY Key;
		float StartTimeStamp;
		float EndTimeStamp;

		PushKeyRecord(ATTACK_KEY k, float t): Key(k), StartTimeStamp(t), EndTimeStamp(-1.0f) {}
	};
	
	UInputBufferComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PushAttackKey(ATTACK_KEY key, float time);

	void DebugInputBuffer();

	/** 因为每一帧可能同时按下几个键，并且由于可能的组合键有1-4个，所有全部需要生成 */
	TArray<FString> GetInputCommonds() const;

	TArray<FString> GetPosiableCombinationKey();

	void UpdateInputBuffer();

	UFUNCTION(BlueprintCallable, Category = "Input")
	bool ConsumeInputKey();

	void OnUpKeyDown();
	void OnUpKeyRelease();

	void OnDownKeyDown();
	void OnDownKeyRelease();

	void OnRightKeyDown();
	void OnRightKeyRelease();

	void OnLeftKeyDown();
	void OnLeftKeyRelease();

	void PressAttackKey(ATTACK_KEY key, float time);
	
	void ReleaseAttackKey(ATTACK_KEY key, float time);

	TArray<FString> GenerateAttackCommond() const;

protected:
	virtual void BeginPlay() override;

public:	
	/** 在Idle状态下，InputBuffer最大可以保留的时间*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Buffer)
	bool bAttackKeyDown;

	/** InputBuffer最大容量*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Buffer)
	int32 MaxInputBufferCount;

	/** 在Idle状态下，InputBuffer最大可以保留的时间*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Buffer)
	float MaxIdleHoldTime;

	bool bJumpKeyDown;

	bool bIsSprinting;

private:
	/** 保存实际的按键数据*/
	TArray<FrameInputKey> InputBuffer;

	/** 基于按键的Press-Release的按键处理*/
	TArray<PushKeyRecord> PushKeysBuffer;
};
