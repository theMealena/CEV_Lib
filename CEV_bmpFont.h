#ifndef CEV_BMPFONT_H_INCLUDED
#define CEV_BMPFONT_H_INCLUDED

#include <SDL.h>

#define CEV_BMPFONT_NUM   7
#define CEV_BMPFONT_CHAR_NUM 60


typedef enum E_CEV_BmpFontSize
{
    CEV_BMPFONT_SMALL   = 8,
    CEV_BMPFONT_MED     = 16,
    CEV_BMPFONT_LARGE   = 32
}
CEV_BmpFontSize;


typedef struct S_CEV_bmpFont
{
    SDL_Surface* font;
    SDL_Rect dimFont, dimChar;
}
CEV_BmpFont;


CEV_BmpFont* CEV_bmpFontCreate(SDL_Surface* fonts, int which, CEV_BmpFontSize size);

void CEV_bmpFontFree(CEV_BmpFont* font);

int CEV_bmpFontDispValue(CEV_BmpFont* font, SDL_Texture* dst, int value, SDL_Rect pos);

int CEV_bmpFontDispText(CEV_BmpFont* font, SDL_Texture* dst, const char* text, SDL_Rect pos);


#endif // CEV_BMPFONT_H_INCLUDED
