// Copyright IceRiver. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

// 当前动作的类型
UENUM(BlueprintType)
enum class MOVE_TYPE : uint8
{
	MOVE_IDLE = 0,	// I
	MOVE_ATTACK,	// A
	MOVE_GUARD,		// G
	MOVE_DASH,		// D
	MOVE_BEING_HIT,	// H
	MOVE_JUMP,		// J
};

namespace Meteor
{
	UENUM(BlueprintType)
	enum class METEOR_API INPUT_EVENT
	{
		INPUT_Pressed = 0,	// 按下 事件
		INPUT_Released,		// 释放 事件
		INPUT_Hold,			// 持续按下 事件
		INPUT_Idle,			// 键盘处于 未激活状态
	};

	UENUM(BlueprintType)
	enum class METEOR_API INPUT_KEY
	{
		KEY_UP = 0,			// U
		KEY_DOWN,			// D
		KEY_LEFT,			// L
		KEY_RIGHT,			// R
		KEY_ATTACK,			// A
		KEY_GUARD,			// G guard
		KEY_JUMP,			// J
		KEY_OTHER,			// other
	};

	// 决定角色受击动作
	UENUM(BlueprintType)
	enum class METEOR_API STATE_TYPE
	{
		TYPE_STANDING,	// S
		TYPE_CROUCHING,	// C
		TYPE_AIR,		// A
		TYPE_LYE_DOWN,	// L
		TYPE_UNCHANGED,	// U
		TYPE_UNKNOWN,
	};

	UENUM(BlueprintType)
	enum class METEOR_API ATTACK_ANIM_STATE
	{
		ANIM_BEGIN,		// 动画正在切换到下一个	
		ANIM_LINK,
		ANIM_PLYING,	// 动画正在播放
		ANIM_PAUSING,	// 动画暂停
	};

	UENUM(BlueprintType)
	enum class METEOR_API HIT_TYPE
	{
		HIT_HIT,	// 击中对手
		HIT_DODGE,	// 攻击被闪避
		HIT_MISS,	// 未击中
		HIT_GUARD,	// 攻击被防御
	};
	
	UENUM(BlueprintType)
	enum class METEOR_API ACTION_TYPE
	{
		IDLE,
		MOVE,
		JUMP,
		ATTACK,
		GUARD,
		BEING_HIT,
		LYING_DOWN,
	};

	UENUM(BlueprintType)
	enum class METEOR_API DASH_TYPE
	{
		DASH_FWD = 0,
		DASH_BWD,
		DASH_RIGHT,
		DASH_LEFT,
	};

	UENUM(BlueprintType)
	enum class METEOR_API WEAPON_TYPE
	{
		WPN_NONE		= 0,
		WPN_Fei_Biao	= 1,	// dart
		WPN_Fei_Lun		= 2,
		WPN_Huo_Tong	= 3,
		WPN_Shuang_Ci	= 4,
		WPN_Bi_Shou		= 5,	// knife
		WPN_Jian		= 6,	// sword
		WPN_Chang_Qiang = 7,
		WPN_Dao			= 8,
		WPN_Chui		= 9,	// hammer
		WPN_Qian_Kun_Dao= 10,
		WPN_ZhiHu		= 11,
		WPN_RenDao		= 12
	};

	UENUM(BlueprintType)
	struct METEOR_API InputState
	{
		INPUT_KEY Key;
		INPUT_EVENT Event;
		float HoldTime;
	};

	INPUT_KEY GetInputKey(const TCHAR& c);

	InputState CompileInputState(const FString& cmd, bool& bSucceed);

	FName GetKeyMappingName(const INPUT_KEY& key);
	
	// 输入指令遵循MUGEN的指令，后期如果要扩展指令也好处理
	struct METEOR_API InputCommand
	{
		bool CreateInputCommand(FName n, const FString& inputCmd); // 解析inputCmd来生成Command
		
		FName Name;
		TArray<InputState> Command;
		bool bIsDelayCommand;
	};

	// 用来代表某个攻击动作
	struct METEOR_API ActionStateDef
	{
		FName name;
		InputCommand cmd; // 并不是每一个ActionState都有Cmd
		STATE_TYPE type;
		MOVE_TYPE moveType;
		int poseNo;
		int juggling;
		float attack;
		int priority;
		int link;
		int lockFrame;
	};

	class METEOR_API ActionStateRecord
	{
	public:
		bool operator < (const ActionStateRecord& other) const;

		ActionStateDef* Action;
		int CmdState;
		int Timer;
		
		int DelayTimer;
	};
}
