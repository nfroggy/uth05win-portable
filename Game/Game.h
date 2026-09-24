#pragma once
#include "../gameLib/Window/GameWindow.h"
#include "../gameLib/Input/KeyInputDI.h"
#include "../gameLib/th5DatFile/th5DatFile.h"
#include "../gameLib/PMDPlayer/PMDPlayer.h"
#include "../gameLib/SE/SoundEffect.h"

#include "Scheme.h"
#include "SchemeSwitcher.h"

#include "GlobalVariableAndStatistics.h"

namespace th5w{

class CGame
{
public:
	static CGame *s_pCurGame;
	inline static CGlobalVariableAndStatistics& GVar() {return s_pCurGame->m_globalVar;}

	int m_windowWidth,m_windowHeight;
	struct { int left, top, right, bottom; } m_playArea;

	CScheme *m_pCurScheme;

	int m_curSchemeID;

public:
	CGameWindow m_gameWindow;
	CKeyInputDI m_input;
	CSoundEffect m_soundEffect;

	Cth5DatFile m_th5Dat1;
	Cth5DatFile m_th5Dat2;
	Cth5DatFile m_modDat;

	CGlobalVariableAndStatistics m_globalVar;

public:
    Uint64 m_lastFrameTime = 0;
    float m_fpsLimit = 60;
    float m_thisFrameDuration = 1.0f / 60;
    Uint64 m_fpsTimeArray[20]{};
    int m_fpsTimeIdx = 0;

	double m_fps;

protected:
	bool Initialize();
	void UpdateFPS();
	void LoadModConfig();
public:
	Cth5DatFile* FindResource(const char *fileName);
public:
	bool Run(int frameLimit = 0);
	void SetVSYNC(bool bOn);

public:
	void OnDeactivate();
	void OnActivate();
	void OnGameEnd();
	void OnPaintInactivate();

public:
	CGame(void);
	~CGame(void);
};

}