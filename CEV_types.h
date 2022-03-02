#ifndef CEV_TYPES_H_INCLUDED
#define CEV_TYPES_H_INCLUDED

#include <SDL_mixer.h>
#include <SDL_ttf.h>


/** \brief File encapsulation
 */
typedef struct CEV_Capsule
{/**structure containing data and associated informations **/

    uint32_t    type, /**< IS_BMP / IS_PNG... */
                size; /**< data size in bytes */
    void        *data;/**< raw data */
}
CEV_Capsule;


/** \brief TTF Font overlay
 */
typedef struct CEV_Font
{/**font overlay to keep track of allocated data ptr*/

    TTF_Font * font;    /**< actual font */
    void *virtualFile;  /**< font's raw data handle */
}
CEV_Font;


/** \brief Mix chunk overlay
 */
typedef struct CEV_Chunk
{/**chunk overlay to keep track of allocated data ptr*/

    Mix_Chunk* sound;   /**< actual chunk */
    void *virtualFile;  /**< chunk raw data */
}
CEV_Chunk;


/** \brief Mix music overlay
 */
typedef struct CEV_Music
{/**music overlay to keep track of allocated data ptr*/

    Mix_Music* music;   /**< Actual Mix_Music */
    void *virtualFile;  /**< Music raw data */
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


/** \brief Copies Texture into surface.
 *
 * \param src : SDL_Texture* to copy.
 *
 * \return SDL_Surface* as result, NULL on failure.
 *
 */
SDL_Surface* CEV_textureToSurface(SDL_Texture* src);



/** \brief Saves texture as png file.
 *
 * \param src : SDL_Texture* to convert.
 * \param fileName : char* as resulting file name.
 *
 * \return int : negative on failure.
 *
 */
int CEV_textureSavePNG(SDL_Texture *src, char* fileName);


/** \brief Saves texture as png into SDL_virtual file.
 *
 * \param src : SDL_Texture* to convert.
 * \param dst : SDL_RWops* to write into.
 *
 * \return int : negative on failure.
 */
int CEV_textureSavePNG_RW(SDL_Texture *src, SDL_RWops* dst);


/** \brief Fills CEV_Capsule with png from texture.
 *
 * \param src : SDL_Texture* to convert.
 * \param dst : CEV_Capsule* to fill.
 *
 * \return int : std function status.
 */
int CEV_textureToCapsule(SDL_Texture* src, CEV_Capsule* dst);


#endif // CEV_TYPES_H_INCLUDED
