#pragma once
#include "mygl.h"

namespace th5w {
class CPC98Font {
    static bool s_bFontLoaded;
    static GLuint s_fontTex;
    static int ConvertCharToTexXY(float *x, float *y, unsigned char *str);
public:
    static bool CreateTextureForCommonChar();
    static void DestroyTextureForCommonChar();
    static bool LoadAnex86BMP(const char *filename);
    static bool DrawString(char *str, int nChar, int x, int y, float r, float g, float b,
                           float fade = 1.0, bool upperLeft = true);
    static void FinalizeCache();
};
}
