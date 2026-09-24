#include "../gameLib/Misc/Random.h"
#include "Game.h"
#include "../gameLib/Graphic/PC98Font.h"
#include "../gameLib/Graphic/Th5ExtFont.h"
#include "../gameLib/CommonFunction/CommonFunctionSystem.h"
#include <time.h>

namespace th5w{

CGame* CGame::s_pCurGame=NULL;

CGame::CGame(void)
{
	m_windowWidth=640;
	m_windowHeight=480;
	m_playArea.left=32;
	m_playArea.top=16+40;
	m_playArea.right=m_playArea.left+384;
	m_playArea.bottom=m_playArea.top+368;

	m_pCurScheme=NULL;

	memset(m_fpsTimeArray,0,sizeof(m_fpsTimeArray));
	m_fpsTimeIdx=0;
	m_fps=0;
}

CGame::~CGame(void)
{
}

void CGame::UpdateFPS()
{
    const Uint64 now = SDL_GetTicksNS();
    const int index = m_fpsTimeIdx % 20;
    if (m_fpsTimeArray[index] && now > m_fpsTimeArray[index])
        m_fps = 20e9 / double(now - m_fpsTimeArray[index]);
    m_fpsTimeArray[index] = now;
    m_fpsTimeIdx = (m_fpsTimeIdx + 1) % 20;
}

void CGame::SetVSYNC(bool on)
{
    const auto *mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(m_gameWindow.m_window));
    const bool matches = mode && SDL_fabsf(mode->refresh_rate - m_fpsLimit) < 0.1f;
    SDL_GL_SetSwapInterval(on && matches ? 1 : 0);
}

Cth5DatFile* CGame::FindResource(const char *fileName)
{
	if (m_modDat.GetChildFileIndex(fileName)!=-1)
		return &m_modDat;
	if (m_th5Dat1.GetChildFileIndex(fileName)!=-1)
		return &m_th5Dat1;
	if (m_th5Dat2.GetChildFileIndex(fileName)!=-1)
		return &m_th5Dat2;

	return NULL;
}

void CGame::LoadModConfig()
{
	int fileIdx=m_modDat.GetChildFileIndex("mod.ini");
	if (fileIdx==-1)
		return;
	m_modDat.Childfseek(fileIdx,0,SEEK_SET);
	char curLine[1000];
	while(m_modDat.Childfgets(curLine,1000,fileIdx)!=NULL)
	{
		if (strlen(curLine)>0&&curLine[strlen(curLine)-1]=='\n')		//replace trailing '\n' with 0
			curLine[strlen(curLine)-1]=0;

		char *ptr=curLine;

		while(ptr[0]==' ') ptr++;										//remove leading spaces

		if (strlen(ptr)<2)
			continue;

		if (curLine[0]=='/'&&curLine[1]=='/')							//commented line
			continue;

		if (strchr(ptr,'=')==NULL)										//'=' not found
			continue;

		char *value=strchr(ptr,'=');
		value[0]=0;														//cut the string to 2 parts

		value++;
		while(value[0]==' ') value++;									//remove leading spaces

		char variable[100];
		sscanf(ptr,"%s",variable);										//sscanf to remove trailing spaces

		if (strcmp(variable,"TEXT_CODEPAGE")==0)
		{
			sscanf(value,"%d",&GVar().m_textCodePage);
			continue;
		}
		if (strcmp(variable,"FONT_CHARSET")==0)
		{
			sscanf(value,"%d",&GVar().m_fontCharset);
			continue;
		}
		if (strcmp(variable,"USE_SYSTEM_FONT")==0)
		{
			int v;
			sscanf(value,"%d",&v);
			GVar().m_bUseSystemFont=(v==0?false:true);
			continue;
		}
	}
}

namespace {
bool StartupError(const char *message)
{
    SDL_Log("%s", message);
    return false;
}
}

bool CGame::Initialize()
{
    m_globalVar.Initialize();
    if (!m_gameWindow.Create(false, m_windowWidth, m_windowHeight))
        return StartupError(SDL_GetError());
    if (!m_input.Initialize()) return StartupError("Keyboard initialization failed.");
    if (!CPMDPlayer::Initialize()) return StartupError("PMD audio initialization failed.");
    if (!m_soundEffect.Initialize()) return StartupError("Sound effect initialization failed.");

    struct Asset { const char *name; unsigned char md5[16]; };
    const Asset assets[] = {
        {"kaiki1.dat", {203,110,35,68,170,147,49,44,60,244,11,241,111,58,20,242}},
        {"kaiki2.dat", {202,32,227,240,57,70,193,115,160,42,98,243,123,172,175,209}},
        {"zun.com", {0,5,230,87,18,82,190,211,141,65,103,153,192,5,60,11}}
    };
    for (const auto &asset : assets) {
        unsigned char md5[16]{};
        if (!CCommonFunctionSystem::GetFileMD5(md5, asset.name) || memcmp(md5, asset.md5, 16)) {
            SDL_Log("%s is missing or corrupted. Run from the game data directory, or use --data-dir.", asset.name);
            return false;
        }
    }
    if (!m_th5Dat1.LoadFile("kaiki1.dat") || !m_th5Dat2.LoadFile("kaiki2.dat") ||
        !CTh5ExtFont::LoadZUNCOM("zun.com"))
        return StartupError("Could not load game data.");
    if (GVar().m_bUseMod) {
        if (!m_modDat.LoadFile(GVar().m_modFileName))
            return StartupError("The configured mod file is missing or corrupted.");
        LoadModConfig();
    }
    if (!GVar().m_bUseSystemFont && !CPC98Font::LoadAnex86BMP("anex86.bmp")) {
        SDL_Log("Using system fonts (optional anex86.bmp not found).");
        GVar().m_bUseSystemFont = true;
    }
    if (GVar().m_bUseSystemFont && !CPC98Font::CreateTextureForCommonChar())
        return StartupError("Could not load a system font.");
    SDL_DisableScreenSaver();
    SeedGameRandom((unsigned)time(nullptr));
    m_lastFrameTime = SDL_GetTicksNS();
    return true;
}

bool CGame::Run(int frameLimit)
{
    s_pCurGame = this;
    if (!Initialize()) {
        CPMDPlayer::Finalize();
        m_soundEffect.Finalize();
        CPC98Font::FinalizeCache();
        CPC98Font::DestroyTextureForCommonChar();
        m_gameWindow.Destroy();
        return false;
    }
    CSchemeSwitcher::GetFirstScheme(&m_pCurScheme, &m_curSchemeID);
    int frames = 0;
    while (m_pCurScheme && m_gameWindow.ProcessEvents()) {
        if (!m_gameWindow.m_bActive && !frameLimit) {
            SDL_Delay(20);
            continue;
        }
        const int result = m_pCurScheme->Step();
        m_pCurScheme->Draw();
        while (CPMDPlayer::FillSoftwareBuffer(1024)) {}
        m_gameWindow.m_pGLContext->SwapBuffers();
        const Uint64 duration = Uint64(1e9 / m_fpsLimit);
        Uint64 now = SDL_GetTicksNS();
        if (now - m_lastFrameTime < duration)
            SDL_DelayPrecise(duration - (now - m_lastFrameTime));
        now = SDL_GetTicksNS();
        m_thisFrameDuration = float(now - m_lastFrameTime) / 1e9f;
        m_lastFrameTime = now;
        UpdateFPS();
        if (result) {
            CScheme *next = nullptr;
            int nextID = m_curSchemeID;
            if (CSchemeSwitcher::SwitchScheme(&next, &nextID, m_curSchemeID, result)) {
                delete m_pCurScheme;
                m_pCurScheme = next;
                m_curSchemeID = nextID;
            }
        }
        if (frameLimit > 0 && ++frames >= frameLimit) break;
    }
    OnGameEnd();
    return true;
}

void CGame::OnDeactivate()
{
    CPMDPlayer::OnDeactivate();
    SDL_EnableScreenSaver();
}
void CGame::OnActivate()
{
    CPMDPlayer::OnActivate();
    SDL_DisableScreenSaver();
    m_lastFrameTime = SDL_GetTicksNS();
}
void CGame::OnGameEnd()
{
    delete m_pCurScheme;
    m_pCurScheme = nullptr;
    m_globalVar.SaveConfig();
    CPMDPlayer::Finalize();
    m_soundEffect.Finalize();
    CPC98Font::FinalizeCache();
    CPC98Font::DestroyTextureForCommonChar();
    SDL_EnableScreenSaver();
    m_gameWindow.Destroy();
}
void CGame::OnPaintInactivate()
{
    if (m_pCurScheme) m_pCurScheme->Draw();
    m_gameWindow.m_pGLContext->SwapBuffers();
}
}
