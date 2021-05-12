#include <stdlib.h>
#include <SDL_Mixer.h>
#include <SDL_ttf.h>
#include "CEV_types.h"
#include "CEV_mixSystem.h"

void CEV_fontClose(CEV_Font* font)
{/*closes overlay font*/

    if(IS_NULL(font))
        return;

    TTF_CloseFont(font->font);
    free(font->virtualFile);

    free(font);
}


void CEV_waveClose(CEV_Chunk* chunk)
{/*closes overlay sound*/

    if(IS_NULL(chunk))
        return;

    if(chunk->sound != NULL)
        Mix_FreeChunk(chunk->sound);

    if(chunk->virtualFile != NULL)
        free(chunk->virtualFile);

    free(chunk);
}


void CEV_musicClose(CEV_Music* music)
{/*closes overlay music*/

    if(IS_NULL(music))
        return;

    if(music->music != NULL)
        Mix_FreeMusic(music->music);

    if(music->virtualFile != NULL)
        free(music->virtualFile);

    if(music != NULL)
        free(music);

    CEV_soundSystemGet()->loadedMusic = NULL;
}
