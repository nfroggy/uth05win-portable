#pragma once

namespace th5w{

class CCommonFunctionInput
{
public:
	static unsigned short GetAllKeyState();
	inline static bool UpKey(unsigned short keyState)
	{
		return (keyState&1)!=0;
	}
	inline static bool DownKey(unsigned short keyState)
	{
		return (keyState&2)!=0;
	}
	inline static bool LeftKey(unsigned short keyState)
	{
		return (keyState&4)!=0;
	}
	inline static bool RightKey(unsigned short keyState)
	{
		return (keyState&8)!=0;
	}
	inline static bool XKey(unsigned short keyState)
	{
		return (keyState&16)!=0;
	}
	inline static bool ZKey(unsigned short keyState)
	{
		return (keyState&32)!=0;
	}
	inline static bool LShiftKey(unsigned short keyState)
	{
		return (keyState&64)!=0;
	}
	inline static bool LCtrlKey(unsigned short keyState)
	{
		return (keyState&128)!=0;
	}
	inline static bool ESCKey(unsigned short keyState)
	{
		return (keyState&4096)!=0;
	}
	inline static bool EnterKey(unsigned short keyState)
	{
		return (keyState&8192)!=0;
	}
	inline static bool QKey(unsigned short keyState)
	{
		return (keyState&16384)!=0;
	}
	inline static bool UpPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&1)!=0;
	}
	inline static bool DownPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&2)!=0;
	}
	inline static bool LeftPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&4)!=0;
	}
	inline static bool RightPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&8)!=0;
	}
	inline static bool XPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&16)!=0;
	}
	inline static bool ZPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&32)!=0;
	}
	inline static bool LShiftPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&64)!=0;
	}
	inline static bool LCtrlPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&128)!=0;
	}
	inline static bool ESCPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&4096)!=0;
	}
	inline static bool EnterPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&8192)!=0;
	}
	inline static bool QPressed(unsigned short keyState,unsigned short lastState)
	{
		return (keyState&(~lastState)&16384)!=0;
	}
public:
	CCommonFunctionInput(void);
	~CCommonFunctionInput(void);
};

}
