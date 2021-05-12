#ifndef CEV_TYPES_H_INCLUDED
#define CEV_TYPES_H_INCLUDED

#include <SDL_mixer.h>
#include <SDL_ttf.h>

/** \brief TTF Font overlay
 */
typedef struct CEV_Font
{/**font overlay to keep track of allocated data ptr*/
    TTF_Font * font;
    void *virtualFile;
}
CEV_Font;


/** \brief Mix chunk overlay
 */
typedef struct CEV_Chunk
{/**chunk overlay to keep track of allocated data ptr*/
    Mix_Chunk* sound;
    void *virtualFile;
}
CEV_Chunk;


/** \brief Mix music overlay
 */
typedef struct CEV_Music
{/**music overlay to keep track of allocated data ptr*/
    Mix_Music* music;
    void *virtualFile;
}
CEV_Music;


/** \brief Closes opened CEV_Font.
 *
 * \param font : CEV_Font* to close.
 *
 * \return N/A.
 */
void CEV_fontClose(CEV_Font* font);


/** \brief Closes opened CEV_Chunk.
 *
 * \param font : CEV_Chunk* to close.
 *
 * \return N/A.
 */
void CEV_waveClose(CEV_Chunk* chunk);


/** \brief Closes opened CEV_Music.
 *
 * \param music : CEV_Music* to close.
 *
 * \return N/A.
 */
void CEV_musicClose(CEV_Music* music);


#endif // CEV_TYPES_H_INCLUDED
