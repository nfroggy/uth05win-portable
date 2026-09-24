#include "CommonFunctionGraphic.h"
#include "../../Game/Game.h"
#include <math.h>


namespace th5w{

CCommonFunctionGraphic::CCommonFunctionGraphic(void)
{
}

CCommonFunctionGraphic::~CCommonFunctionGraphic(void)
{
}

bool CCommonFunctionGraphic::LoadPIFromDat(C2DImage** pRet,unsigned char *outPalette,
										   Cth5DatFile *pDatFile,char *piFileName)
{
	int piIdx=pDatFile->GetChildFileIndex(piFileName);
	if (piIdx==-1)
		return false;
	th5w::CPIFile piFile(pDatFile->GetChildFileContentPointer(piIdx),
						   pDatFile->GetChildFileLength(piIdx));
	if (piFile.m_bValidFormat==false)
		return false;
	C2DImage *p2DImage=C2DImage::Create2DImage(piFile.GetWidth(),piFile.GetHeight());
	if (piFile.ExtractImageAndPalette(p2DImage->GetDataPointer(),outPalette,NULL)==false)
	{
		p2DImage->Destroy();
		return false;
	}
	if (p2DImage->UploadToTexture()==false)
	{
		p2DImage->Destroy();
		return false;
	}

	*pRet=p2DImage;
	return true;
}

bool CCommonFunctionGraphic::LoadCD2CDGFromDat(C2DImage** pRet,Cth5DatFile *pDatFile,
											   char *cd2cdgFileName,int spriteIdx, unsigned char *inPalette)
{
	int cd2cdgIdx=pDatFile->GetChildFileIndex(cd2cdgFileName);
	if (cd2cdgIdx==-1)
		return false;
	th5w::CCD2CDGFile cd2cdgFile(pDatFile->GetChildFileContentPointer(cd2cdgIdx),
								   pDatFile->GetChildFileLength(cd2cdgIdx));
	if (cd2cdgFile.m_bValidFormat==false)
		return false;

	C2DImage *p2DImage=C2DImage::Create2DImage(cd2cdgFile.GetWidth(),cd2cdgFile.GetHeight());
	if (cd2cdgFile.ExtractImage(p2DImage->GetDataPointer(),spriteIdx,inPalette)==false)
	{
		p2DImage->Destroy();
		return false;
	}
	if (p2DImage->UploadToTexture()==false)
	{
		p2DImage->Destroy();
		return false;
	}

	*pRet=p2DImage;
	return true;
}

bool CCommonFunctionGraphic::LoadBFTFromDat(C2DImage** pRet,unsigned char *outPalette,Cth5DatFile *pDatFile,
											char *bftFileName,int patternIdx,int transparentColorIdx)
{
	int bftIdx=pDatFile->GetChildFileIndex(bftFileName);
	if (bftIdx==-1)
		return false;
	th5w::CBFTFile bftFile(pDatFile->GetChildFileContentPointer(bftIdx),
							 pDatFile->GetChildFileLength(bftIdx));
	if (bftFile.m_bValidFormat==false)
		return false;

	C2DImage *p2DImage=C2DImage::Create2DImage(bftFile.GetWidth(),bftFile.GetHeight());
	if (bftFile.ExtractImageAndPalette(p2DImage->GetDataPointer(),outPalette,patternIdx,
									   NULL,transparentColorIdx)==false)
	{
		p2DImage->Destroy();
		return false;
	}
	if (p2DImage->UploadToTexture()==false)
	{
		p2DImage->Destroy();
		return false;
	}

	*pRet=p2DImage;
	return true;
}

bool CCommonFunctionGraphic::LoadCD2CDGAllSpriteFromDat(C2DImageArray *pOutImgArray,Cth5DatFile *pDatFile,
														char *cd2cdgFileName,unsigned char *inPalette)
{
	int cd2cdgIdx=pDatFile->GetChildFileIndex(cd2cdgFileName);
	if (cd2cdgIdx==-1)
		return false;
	th5w::CCD2CDGFile cd2cdgFile(pDatFile->GetChildFileContentPointer(cd2cdgIdx),
								   pDatFile->GetChildFileLength(cd2cdgIdx));
	if (cd2cdgFile.m_bValidFormat==false)
		return false;

	int imgArrayOldSize=pOutImgArray->NImage();
	for (int i=0;i<cd2cdgFile.NSprite();i++)
	{
		C2DImage *p2DImage=C2DImage::Create2DImage(cd2cdgFile.GetWidth(),cd2cdgFile.GetHeight());
		if (cd2cdgFile.ExtractImage(p2DImage->GetDataPointer(),i,inPalette)==false)
		{
			p2DImage->Destroy();
			pOutImgArray->UnloadRange(imgArrayOldSize);
			return false;
		}
		if (p2DImage->UploadToTexture()==false)
		{
			p2DImage->Destroy();
			pOutImgArray->UnloadRange(imgArrayOldSize);
			return false;
		}
		pOutImgArray->AddImage(p2DImage);
	}

	return true;

}

bool CCommonFunctionGraphic::LoadBFTAllPatternFromDat(C2DImageArray *pOutImgArray,unsigned char *outPalette,
													  Cth5DatFile *pDatFile,char *bftFileName,
													  int transparentColorIdx, unsigned char *inPalette)
{
	int bftIdx=pDatFile->GetChildFileIndex(bftFileName);
	if (bftIdx==-1)
		return false;
	th5w::CBFTFile bftFile(pDatFile->GetChildFileContentPointer(bftIdx),
							 pDatFile->GetChildFileLength(bftIdx));
	if (bftFile.m_bValidFormat==false)
		return false;

	int imgArrayOldSize=pOutImgArray->NImage();
	for (int i=0;i<bftFile.NPattern();i++)
	{
		C2DImage *p2DImage=C2DImage::Create2DImage(bftFile.GetWidth(),bftFile.GetHeight());
		if (bftFile.ExtractImageAndPalette(p2DImage->GetDataPointer(),outPalette,i,
										   inPalette,transparentColorIdx)==false)
		{
			p2DImage->Destroy();
			pOutImgArray->UnloadRange(imgArrayOldSize);
			return false;
		}
		if (p2DImage->UploadToTexture()==false)
		{
			p2DImage->Destroy();
			pOutImgArray->UnloadRange(imgArrayOldSize);
			return false;
		}
		pOutImgArray->AddImage(p2DImage);
	}

	return true;
}

bool CCommonFunctionGraphic::LoadBinaryImageFile(C2DImage **pRet,unsigned char transparentColor[3], const char *fileName)
{
    SDL_Surface *loaded = SDL_LoadBMP(fileName);
    if (!loaded) return false;
    SDL_Surface *img = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGB24);
    SDL_DestroySurface(loaded);
    if (!img) return false;
    const int width = img->w, height = img->h;
    C2DImage *pImg = C2DImage::Create2DImage(width, height);
    unsigned char *pWrite = pImg->GetDataPointer();
    for (int y = height - 1; y >= 0; --y) {
        auto *row = static_cast<unsigned char *>(img->pixels) + y * img->pitch;
        for (int x = 0; x < width; ++x, pWrite += 4) {
            // The original CImage reader supplied BGR pixels.
            const bool transparent = row[x*3] == transparentColor[2] &&
                row[x*3+1] == transparentColor[1] && row[x*3+2] == transparentColor[0];
            memset(pWrite, transparent ? 0 : 255, 4);
        }
    }
    SDL_DestroySurface(img);

	if (pImg->UploadToTexture()==false)
	{
		pImg->Destroy();
		return false;
	}
	*pRet=pImg;

	return true;
}

void CCommonFunctionGraphic::ScreenFade(float fadeScale)
{
	if (fadeScale<0) fadeScale=0;
	if (fadeScale>200) fadeScale=200;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,1,0,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	float overlayColor;
	if (fadeScale<100)
		overlayColor=0;
	else
		overlayColor=1;
	float overlayAlpha=fabs(fadeScale-100)/100;
	glColor4f(overlayColor,overlayColor,overlayColor,overlayAlpha);
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(2,-1);
		glVertex2f(2,2);
		glVertex2f(-1,2);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CCommonFunctionGraphic::DrawRectangle(float XLow,float YLow,float XHigh,float YHigh,
										   float colorR,float colorG,float colorB,float alpha,bool bSolid)
{
	YLow=CGame::s_pCurGame->m_windowHeight-1-YLow;
	YHigh=CGame::s_pCurGame->m_windowHeight-1-YHigh;
	if (XLow>XHigh)
	{float tmp=XLow;XLow=XHigh;XHigh=tmp;}
	if (YLow>YHigh)
	{float tmp=YLow;YLow=YHigh;YHigh=tmp;}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,CGame::s_pCurGame->m_windowWidth,
			   0,CGame::s_pCurGame->m_windowHeight);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(colorR,colorG,colorB,alpha);
	if (bSolid)
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	else
	{
		XHigh-=1.0f;
		YHigh-=1.0f;
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
    glBegin(GL_QUADS);
		glVertex2f(XLow,YLow);
		glVertex2f(XHigh+1,YLow);
		glVertex2f(XHigh+1,YHigh+1);
		glVertex2f(XLow,YHigh+1);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	if (!bSolid)
	{
		glBegin(GL_POINTS);
			glVertex2f(XHigh+1,YHigh+1);
		glEnd();
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

}




















