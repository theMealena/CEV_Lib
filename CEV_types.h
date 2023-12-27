#ifndef CEV_TYPES_H_INCLUDED
#define CEV_TYPES_H_INCLUDED

#include <stdio.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>


#define IS_PIC(x) (((x)==IS_BMP) + ((x)==IS_PNG) + ((x)==IS_JPG))
#define FILE_TYPE_NUM 20
#define FILE_TYPE_LIST {"default", "dat", "dtx", "bmp", "png", "jpg", "gif", "wav", "ttf", "sps", "men", "scl", "map", "mp3", "plx", "wtr", "txt", "ani", "obj", "bmf"}


/** \brief defines file type
 */
typedef enum FILE_TYPE
{
    IS_DEFAULT  = 0,    /**< unknown / generic */
    IS_DAT      = 1,    /**< any data  .dat */
    IS_DTX      = 2,    /**< CEV_Text  .dtx */
    IS_BMP      = 3,    /**< bmp       .bmp */
    IS_PNG      = 4,    /**< png       .png */
    IS_JPG      = 5,    /**< jpg       .jpg */
    IS_GIF      = 6,    /**< gif       .gif */
    IS_WAV      = 7,    /**< wave      .wav */
    IS_FONT     = 8,    /**< font.ttf  .ttf */
    IS_SPS      = 9,    /**< sprite    .sps */
    IS_MENU     = 10,   /**< menu      .men */
    IS_SCROLL   = 11,   /**< scroller  .scl */
    IS_MAP      = 12,   /**< map       .map */
    IS_MUSIC    = 13,   /**< mp3       .mp3 */
    IS_PRLX     = 14,   /**< parallax  .plx */
    IS_WTHR     = 15,   /**< weather   .wtr */
    IS_TXT      = 16,   /**< text file .txt */  // TODO (drx#1#09/15/23): Doublon avec CEV_Text ?
    IS_ANI      = 17,   /**< short animation .ani */
    IS_OBJ      = 18,   /**< is game object .obj */
    IS_BMPFONT  = 19    /**< is bmp font .bmf */
}
FILE_TYPE;


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
 *  note : music overlay to keep track of allocated data ptr
 */
typedef struct CEV_Music
{
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
 * \param chunk : CEV_Chunk* to close.
 *
 * \return N/A.
 */
void CEV_waveClose(CEV_Chunk* chunk);


/** \brief Creates CEV_Music from file (mp3)
 *
 * \param fileName : char* as name of file to open.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music* CEV_musicLoad(char *fileName);


/** \brief Closes opened CEV_Music.
 *
 * \param music : CEV_Music* to close.
 *
 * \return N/A.
 */
void CEV_musicClose(CEV_Music* music);


/** \brief Clear structure content.
 *
 * \param music : CEV_Music* to clear.
 *
 * \return void
 */
void CEV_musicClear(CEV_Music* music);


/** \brief Copies Texture into surface.
 *
 * \param src : SDL_Texture* to copy.
 * \param pxlData : filled with Surface's pixel data.
 *
 * \return SDL_Surface* as result, NULL on failure.
 *
 * \note ptr content is to be freed after Surface is freed.
 */
SDL_Surface* CEV_textureToSurface(SDL_Texture* src, void** pxlData);


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


/** \brief Gets texture dimension as SDL_Rect.
 *
 * \param src : SDL_Texture* to fetch dimensions from.
 *
 * \return SDL_Rect filled with dimensions.
 *
 * \note x,y set to 0.
 */
SDL_Rect CEV_textureDimGet(SDL_Texture* src);


/** \brief BLits surface onto texture.
 *
 * \param src : SDL_Surface* as surface to read.
 * \param dst : SDL_Texture* to blit onto.
 * \param srcRect : SDL_Rect* src rect as clip.
 * \param dstRect : SDL_Rect* dst rect as blit position.
 *
 * \return int of standard function status.
 */
int CEV_blitSurfaceToTexture(SDL_Surface *src, SDL_Texture* dst, SDL_Rect* srcRect, SDL_Rect* dstRect);


/** \brief SDL_Surface into CEV_Capsule.
 *
 * \param src : SDL_Surface* to encap.
 * \param dst : CEV_Capsule* to load result into.
 *
 * \return int from std function status.
 */
int CEV_surfaceToCapsule(SDL_Surface* src, CEV_Capsule* dst);


/** \brief loads file into a capsule as it
 *
 * \param caps : CEV_Capsule* to store file.
 * \param fileName : file to be opened and stored.
 *
 * \return any of the function status.
 */
int CEV_capsuleFromFile(CEV_Capsule* caps, const char* fileName);


/** \brief writes capsule into file.
 *
 * \param src : CEV_Capsule* to be written.
 * \param dst : FILE* to write into at actual position.
 *
 * \return readWriteErr is set on error.
 *
 * \note file is written as it is, it is necessary to place yourself
  in the file before writing.
 */
void CEV_capsuleTypeWrite(CEV_Capsule *src, FILE *dst);


/** \brief reads capsule from file.
 *
 * \param src : FILE* to read from actual position.
 * \param dst : CEV_Capsule* to be filled.
 *
 * \return readWriteErr is set.
 *
 * \note file is read as it is, it is necessary to place yourself
  in the file before reading.
 */
void CEV_capsuleTypeRead(FILE *src, CEV_Capsule *dst);


/** \brief writes capsule into virtual file.
 *
 * \param src : CEV_Capsule* to read from.
 * \param dst : SDL_RWops* to write into.
 *
 * \return void.
 */
void CEV_capsuleTypeWrite_RW(CEV_Capsule* src, SDL_RWops* dst);


/** \brief virtual file to mem.
 *
 * \param caps : CEV_Capsule* to be filled.
 * \param src : SDL_RWops* to read from actual position.
 *
 * \return readWriteErr is set.
 */
void CEV_capsuleTypeRead_RW(SDL_RWops* src, CEV_Capsule* dst);


/** \brief clean up / free fileInfo content.
 *
 * \param caps : CEV_Capsule* to clear.
 *
 * \return N/A.
 */
void CEV_capsuleClear(CEV_Capsule *caps);


/** \brief free content and itself.
 *
 * \param caps : CEV_Capsule* to free.
 *
 * \return N/A.
 */
void CEV_capsuleDestroy(CEV_Capsule *caps);
#endif // CEV_TYPES_H_INCLUDED
