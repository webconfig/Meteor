
#include "Common/MeteorDef.h"

/* -command
*   list of buttons or directions, separated by commas.Each of these
*   buttons or directions is referred to as a "symbol".
*   Directions and buttons can be preceded by special characters :
*   slash(/ ) - means the key must be held down
*          egs.command = / D; hold the down direction
*               command = / DB, a; hold down - back while you press a
*   tilde(~) - to detect key releases
*          egs.command = ~a; release the a button
*               command = ~D, F, a; release down, press fwd, then a
*          If you want to detect "charge moves", you can specify
*          the time the key must be held down for (in game - ticks)
*          egs.command = ~30a; hold a for at least 30 ticks, then release
*   dollar($) - Direction - only: detect as 4 - way
*          egs.command = $D; will detect if D, DB or DF is held
*               command = $B; will detect if B, DB or UB is held
*   plus(+) - Buttons only : simultaneous press
*          egs.command = a + b; press a and b at the same time
*               command = x + y + z; press x, y and z at the same time
*   greater - than(> ) - means there must be no other keys pressed or released
*                      between the previous and the current symbol.
*          egs.command = a, > ~a; press a and release it without having hit
* ; or released any other keys in between
*   You can combine the symbols :
*     eg.command = ~30$D, a + b; hold D, DB or DF for 30 ticks, release,
* ; then press a and b together
*
*   Note: Successive direction symbols are always expanded in a manner similar
*         to this example:
*           command = F, F
*         is expanded when MUGEN reads it, to become equivalent to :
*           command = F, > ~F, > F
*
*   It is recommended that for most "motion" commads, eg.quarter - circle - fwd,
*   you start off with a "release direction".This makes the command easier
*   to do.

*   ？ 用来表示延时命令
*   U,U,?18
*	U,U 按完后，过18秒执行
*/

namespace Meteor
{
	INPUT_KEY GetInputKey(const TCHAR& c)
	{
		if (c == TCHAR('U'))
			return INPUT_KEY::KEY_UP;
		else if (c == TCHAR('D'))
			return INPUT_KEY::KEY_DOWN;
		else if (c == TCHAR('L'))
			return INPUT_KEY::KEY_LEFT;
		else if (c == TCHAR('R'))
			return INPUT_KEY::KEY_RIGHT;
		else if (c == TCHAR('A'))
			return INPUT_KEY::KEY_ATTACK;
		else if (c == TCHAR('G'))
			return INPUT_KEY::KEY_GUARD;
		else if (c == TCHAR('J'))
			return INPUT_KEY::KEY_JUMP;
		else
			return INPUT_KEY::KEY_OTHER;
	}

	Meteor::InputState CompileInputState(const FString& cmd, bool& bSucceed)
	{
		bSucceed = false;
		InputState input;
		input.Key = INPUT_KEY::KEY_OTHER;
		FString tmpCmd = cmd;
		tmpCmd.TrimStartAndEndInline();
		if (tmpCmd.IsEmpty() == false)
		{
			// 目前只支持 L,L,A 这种类型
			if (tmpCmd.Len() == 1)
			{
				TCHAR c = tmpCmd[0];
				INPUT_KEY key = GetInputKey(c);
				if (key != INPUT_KEY::KEY_OTHER)
				{
					input.Key = key;
					input.Event = INPUT_EVENT::INPUT_Pressed;
					input.HoldTime = 0.0f;
					bSucceed = true;
				}
			}
		}
		return input;
	}

	FName GetKeyMappingName(const INPUT_KEY& key)
	{
		switch (key)
		{
		case INPUT_KEY::KEY_UP:
			return FName("Up");
		case INPUT_KEY::KEY_DOWN:
			return FName("Down");
		case INPUT_KEY::KEY_LEFT:
			return FName("Left");
		case INPUT_KEY::KEY_RIGHT:
			return FName("Right");
		case INPUT_KEY::KEY_ATTACK:
			return FName("Attack");
		case INPUT_KEY::KEY_GUARD:
			return FName("Guard");
		case INPUT_KEY::KEY_JUMP:
			return FName("Jump");
		default:
			return FName();
		}
	}

	bool InputCommand::CreateInputCommand(FName n, const FString& inputCmd)
	{
		if (inputCmd.IsEmpty() == false)
		{
			FString cmd, LeftStr, RightStr;
			cmd = inputCmd;
			bool bSucceed = false;
			
			this->bIsDelayCommand = false;

			int commaPos = -1;
			if (cmd.FindChar(',', commaPos))
			{
				while (cmd.IsEmpty() == false && cmd.Split(",", &LeftStr, &RightStr))
				{
					cmd = RightStr;
					Meteor::InputState input = CompileInputState(LeftStr, bSucceed);
					if (bSucceed)
					{
						Command.Add(input);
					}
					else
					{
						Command.Reset();
						return false;
					}
				}
				Meteor::InputState input = CompileInputState(RightStr, bSucceed);
				if (bSucceed)
				{
					Command.Add(input);
				}
				else
				{
					if (RightStr.Equals("?"))
					{
						this->bIsDelayCommand = true;
					}
					else
					{
						Command.Reset();
						return false;
					}
				}
			}
			// 说明只有一个指令
			else
			{
				Meteor::InputState input = CompileInputState(cmd, bSucceed);
				if (bSucceed)
				{
					Command.Add(input);
				}
				else
				{
					Command.Reset();
					return false;
				}
			}
			this->Name = n;
			return true;
		}
		return false;
	}

	bool ActionStateRecord::operator < (const ActionStateRecord& other) const
	{
		if (this->Action && other.Action && this->Action->priority < other.Action->priority)
			return true;
		else
			return false;
	}
}
