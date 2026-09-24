#include "CommonFunctionInput.h"
#include "../../Game/Game.h"

namespace th5w{

CCommonFunctionInput::CCommonFunctionInput(void)
{
}

CCommonFunctionInput::~CCommonFunctionInput(void)
{
}

unsigned short CCommonFunctionInput::GetAllKeyState()
{
//key state map:
//low byte bits:
//	7				6			5		4		3		2		1		0
//	left control	left shift	z		x		right	left	down	up
//high byte bits:
//	7		6		5		4		3		2		1		0
//			q		enter	esc

	CGame::s_pCurGame->m_input.UpdateKeyState();
	unsigned short ret=0;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_UP))<<0;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_DOWN))<<1;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_LEFT))<<2;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_RIGHT))<<3;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_X))<<4;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_Z))<<5;
	//PC98 version does not include shift status in the key state but manage it in another variable
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_LSHIFT))<<6;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_LCTRL))<<7;

	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_ESCAPE))<<12;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_RETURN))<<13;
	ret|=((unsigned short)CGame::s_pCurGame->m_input.GetKeyState(SDL_SCANCODE_Q))<<14;

	return ret;
}

}