#ifndef CEV_BLIT_H_INCLUDED
#define CEV_BLIT_H_INCLUDED

#include <SDL.h>

#define CEV_BLIT_UNCHANGED    0
#define CEV_BLIT_INTEGER      1
#define CEV_BLIT_SCALED       2
#define CEV_BLIT_STRETCHED    4


void CEV_blitTexture(SDL_Texture* src, int mode);


void CEV_blitSurfaceToTexture(SDL_Surface *src, SDL_Rect* srcRect, SDL_Texture* dst, SDL_Rect* dstRect);

void CEV_textureClear(SDL_Texture* dst, uint32_t color);
#endif // CEV_BLIT_H_INCLUDED
