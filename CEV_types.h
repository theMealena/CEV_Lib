#ifndef CEV_TYPES_H_INCLUDED
#define CEV_TYPES_H_INCLUDED

#include <SDL_mixer.h>
#include <SDL_ttf.h>


#define IS_PIC(x) (((x)==IS_BMP) + ((x)==IS_PNG) + ((x)==IS_JPG))
#define FILE_TYPE_NUM 18
#define FILE_TYPE_LIST {"default", "dat", "dtx", "bmp", "png", "jpg", "gif", "wav", "ttf", "sps", "men", "scl", "map", "mp3", "plx", "wtr", "txt", "ani"}


/** \brief defines file type
 */
typedef enum FILE_TYPE
{IS_DEFAULT = 0,    //unknow / undefined
  IS_DAT    = 1,    //any data  .dat
  IS_DTX    = 2,    //CEV_Text  .dtx
  IS_BMP    = 3,    //bmp       .bmp
  IS_PNG    = 4,    //png       .png
  IS_JPG    = 5,    //jpg       .jpg
  IS_GIF    = 6,    //gif       .gif
  IS_WAV    = 7,    //wave      .wav
  IS_FONT   = 8,    //font.ttf  .ttf
  IS_SPS    = 9,    //animation .sps
  IS_MENU   = 10,   //menu      .men
  IS_SCROLL = 11,   //scroller  .scl
  IS_MAP    = 12,   //map       .map
  IS_MUSIC  = 13,   //mp3       .mp3
  IS_PLX    = 14,   //parallax  .plx
  IS_WTHR   = 15,   //weather   .wtr
  IS_TXT    = 16,   //text file .txt
  IS_ANI    = 17    //short animation .ani
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
 * \param ptr : filled with Surface's pixel data.
 *
 * \return SDL_Surface* as result, NULL on failure.
 *
 * note : ptr content is to be freed after Surface is freed.
 */
SDL_Surface* CEV_textureToSurface(SDL_Texture* src, void** ptr);


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
 * note : x,y set to 0.
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
 *
 */
int CEV_blitSurfaceToTexture(SDL_Surface *src, SDL_Texture* dst, SDL_Rect* srcRect, SDL_Rect* dstRect);

#endif // CEV_TYPES_H_INCLUDED
