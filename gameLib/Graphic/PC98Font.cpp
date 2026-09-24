#include "PC98Font.h"
#include "../../Game/Game.h"
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iconv.h>
#include <array>
#include <map>
#include <vector>
#include <algorithm>

namespace th5w {
bool CPC98Font::s_bFontLoaded = false;
GLuint CPC98Font::s_fontTex = 0;
namespace {
FT_Library library = nullptr;
FT_Face face = nullptr;
std::map<unsigned int, GLuint> glyphs;
iconv_t converter = (iconv_t)-1;

GLuint Glyph(unsigned int codepoint, int width)
{
    const unsigned int key = codepoint * 2 + (width == 16);
    auto found = glyphs.find(key);
    if (found != glyphs.end()) return found->second;
    std::array<unsigned char, 16*16*2> pixels{};
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO) == 0) {
        const auto &bitmap = face->glyph->bitmap;
        for (int y = 0; y < int(bitmap.rows); ++y) {
            const int dy = 13 - face->glyph->bitmap_top + y;
            if (dy < 0 || dy >= 16) continue;
            for (int x = 0; x < int(bitmap.width); ++x) {
                const int dx = face->glyph->bitmap_left + x;
                if (dx < 0 || dx >= width) continue;
                const auto *row = bitmap.buffer + y * bitmap.pitch;
                const unsigned char alpha = bitmap.pixel_mode == FT_PIXEL_MODE_MONO
                    ? ((row[x / 8] & (0x80 >> (x % 8))) ? 255 : 0) : row[x];
                pixels[((15-dy)*16+dx)*2] = 255;
                pixels[((15-dy)*16+dx)*2+1] = alpha;
            }
        }
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 16, 16, 0,
                 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glyphs.emplace(key, texture);
    return texture;
}
bool LeadByte(unsigned char c)
{
    if (CGame::GVar().m_textCodePage == 932)
        return (c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc);
    return c >= 0x81 && c <= 0xfe;
}
unsigned int Unicode(char *text, int length)
{
    if (converter == (iconv_t)-1) return '?';
    std::array<unsigned char, 4> result{};
    char *input = text, *output = reinterpret_cast<char *>(result.data());
    size_t inSize = length, outSize = result.size();
    iconv(converter, nullptr, nullptr, nullptr, nullptr);
    if (iconv(converter, &input, &inSize, &output, &outSize) == size_t(-1)) return '?';
    return result[0] | (unsigned(result[1]) << 8) | (unsigned(result[2]) << 16) | (unsigned(result[3]) << 24);
}
}

bool CPC98Font::CreateTextureForCommonChar()
{
    if (face) return true;
    if (FT_Init_FreeType(&library)) return false;
    FcPattern *pattern = FcPatternCreate();
    FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8 *>("monospace"));
    FcPatternAddString(pattern, FC_LANG, reinterpret_cast<const FcChar8 *>(
        CGame::GVar().m_textCodePage == 936 ? "zh-cn" : "ja"));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult result;
    FcPattern *match = FcFontMatch(nullptr, pattern, &result);
    FcPatternDestroy(pattern);
    bool success = false;
    if (match) {
        FcChar8 *filename = nullptr;
        int index = 0;
        FcPatternGetInteger(match, FC_INDEX, 0, &index);
        if (FcPatternGetString(match, FC_FILE, 0, &filename) == FcResultMatch)
            success = FT_New_Face(library, reinterpret_cast<char *>(filename), index, &face) == 0;
        FcPatternDestroy(match);
    }
    if (!success || FT_Set_Pixel_Sizes(face, 0, 16)) return false;
    char encoding[32];
    snprintf(encoding, sizeof(encoding), "CP%d", CGame::GVar().m_textCodePage);
    converter = iconv_open("UTF-32LE", encoding);
    return converter != (iconv_t)-1;
}
void CPC98Font::DestroyTextureForCommonChar()
{
    for (const auto &glyph : glyphs) glDeleteTextures(1, &glyph.second);
    glyphs.clear();
    if (face) FT_Done_Face(face);
    if (library) FT_Done_FreeType(library);
    face = nullptr;
    library = nullptr;
    if (converter != (iconv_t)-1) iconv_close(converter);
    converter = (iconv_t)-1;
}
void CPC98Font::FinalizeCache()
{
    if (s_fontTex) glDeleteTextures(1, &s_fontTex);
    s_fontTex = 0;
    s_bFontLoaded = false;
}
bool CPC98Font::LoadAnex86BMP(const char *filename)
{
    SDL_Surface *loaded = SDL_LoadBMP(filename);
    if (!loaded) return false;
    SDL_Surface *img = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGB24);
    SDL_DestroySurface(loaded);
    if (!img) return false;
    if (img->w != 2048 || img->h != 2048) { SDL_DestroySurface(img); return false; }
    std::vector<unsigned char> pixels(2048*2048*2);
    for (int y = 0; y < 2048; ++y) {
        auto *row = static_cast<unsigned char *>(img->pixels) + y * img->pitch;
        for (int x = 0; x < 2048; ++x) {
            const int index = ((2047-y)*2048+x)*2;
            pixels[index] = 255;
            pixels[index+1] = row[x*3] < 128 ? 255 : 0;
        }
    }
    SDL_DestroySurface(img);
    glGenTextures(1, &s_fontTex);
    glBindTexture(GL_TEXTURE_2D, s_fontTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 2048, 2048, 0,
                 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    s_bFontLoaded = true;
    return true;
}
int CPC98Font::ConvertCharToTexXY(float *outX,float *outY,unsigned char *str)
{
	unsigned char twoBytes[2];
	twoBytes[0]=str[0];
	twoBytes[1]=str[1];
	if (twoBytes[0]<128)
	{
		*outX=8*twoBytes[0]/2048.0f;
		*outY=(2032)/2048.0f;			//1-byte ASCII chars are located on top row of bmp
		return 1;
	}

	if (!((twoBytes[0]>=0x81&&twoBytes[0]<=0x9f)||(twoBytes[0]>=0xe0&&twoBytes[0]<=0xfc)))
	{
		printf("char code error 0x%x%x\n",twoBytes[0],twoBytes[1]);
		return 0;
	}

	//second byte of Shift-JIS must be in range 40-fc and can not be 7f
	if (twoBytes[1]<0x40||twoBytes[1]>0xfc||twoBytes[1]==0x7f)
	{
		printf("char code error 0x%x%x\n",twoBytes[0],twoBytes[1]);
		return 0;
	}

	//convert character code to coordinate in anex86.bmp
	if (twoBytes[1]>0x7f)
		twoBytes[1]--;					//no space between 0x7e and 0x80 in bmp, hence later chars moved 1 forward
	if (twoBytes[0]>=0xe0)
		twoBytes[0]-=0xe0-0xa0;			//no space between 0x90 and 0xe0 in bmp, hence later chars moved leftward
	int charX=16+(twoBytes[0]-0x81)*32;	//two 16-pixel columns for each group of codes with the same first byte
	if (twoBytes[1]>=0x9e)				//chars in the second column, although the second column start with 0x9f,
	{									//we use 0x9e here because we have decremented it by 1 above.
		charX+=16;
		twoBytes[1]-=0x9e - 0x40;			//also convert the y to be the same for the first column
	}
	int charY=528+(twoBytes[1]-0x40)*16;

	charY=2047-charY-15;				//from bmp top-down Y to GL texture bottom-up Y, also become
										//coordinate of lower-left corner
	*outX=(charX)/2048.0f;
	*outY=(charY)/2048.0f;

	return 2;
}

bool CPC98Font::DrawString(char *str, int nChar, int x, int y, float r, float g, float b,
                           float fade, bool upperLeft)
{
    const bool system = CGame::GVar().m_bUseSystemFont;
    if ((system && !face) || (!system && !s_bFontLoaded)) return false;
    if (upperLeft) y = CGame::s_pCurGame->m_windowHeight - y - 16;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, CGame::s_pCurGame->m_windowWidth, 0, CGame::s_pCurGame->m_windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(r, g, b, fade);
    for (int i = 0; i < nChar && *str; ++i) {
        int length = LeadByte(static_cast<unsigned char>(*str)) && str[1] ? 2 : 1;
        float u = 0, v = 0, du, dv;
        if (system) {
            glBindTexture(GL_TEXTURE_2D, Glyph(Unicode(str, length), length * 8));
            du = length * 0.5f;
            dv = 1;
        } else {
            length = ConvertCharToTexXY(&u, &v, reinterpret_cast<unsigned char *>(str));
            if (!length) break;
            glBindTexture(GL_TEXTURE_2D, s_fontTex);
            du = length * 8.f / 2048;
            dv = 16.f / 2048;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(u, v); glVertex2f(x, y);
        glTexCoord2f(u+du, v); glVertex2f(x+length*8, y);
        glTexCoord2f(u+du, v+dv); glVertex2f(x+length*8, y+16);
        glTexCoord2f(u, v+dv); glVertex2f(x, y+16);
        glEnd();
        str += length;
        x += length * 8;
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    return true;
}
}
