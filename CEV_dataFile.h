//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   0.2    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**   CEV    |  05-2018      |   1.0.2  | map added      **/
//**   CEV    |  07-2019      |   1.0.3  | parallax added **/
//**   CEV    |  01-2020      |   1.0.4  | weather added  **/
//**   CEV    |  02-2022      |   1.0.4  | documentation  **/
//**********************************************************/
//- CEV 2021 05 20- removed capsule data free from L_capsuleToXxx functions -> capsule cleared in calling functions.

#ifndef CEV_FILE_LOAD_H_INCLUDED
#define CEV_FILE_LOAD_H_INCLUDED

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "CEV_table.h"
#include "CEV_animator.h"
#include "CEV_texts.h"
#include "CEV_scroll.h"
#include "CEV_gif.h"
#include "CEV_types.h"
#include "CEV_selection.h"
#include "CEV_maps.h"
#include "CEV_weather.h"
#include "CEV_parallax.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_PIC(x) (((x)==IS_BMP) + ((x)==IS_PNG) + ((x)==IS_JPG))
#define FILE_TYPE_NUM 16
#define FILE_TYPE_LIST {"default", "dat", "txt", "bmp", "png", "jpg", "gif", "wav", "ttf", "sps", "men", "scl", "map", "mp3", "plx", "wtr"}


/** \brief defines file type
 */
typedef enum FILE_TYPE
{IS_DEFAULT = 0,
  IS_DAT    = 1,
  IS_TXT    = 2,
  IS_BMP    = 3,
  IS_PNG    = 4,
  IS_JPG    = 5,
  IS_GIF    = 6,
  IS_WAV    = 7,
  IS_FONT   = 8,
  IS_SPS    = 9,
  IS_MENU   = 10,
  IS_SCROLL = 11,
  IS_MAP    = 12,
  IS_MUSIC  = 13,
  IS_PLX    = 14,
  IS_WTHR   = 15
}FILE_TYPE;



//** \brief File encapsulation
// */
//typedef struct CEV_Capsule
//{/**structure containing data and associated informations **/

//    uint32_t    type, /**< IS_BMP / IS_PNG... */
//                size; /**< data size in bytes */
//    void        *data;/**< raw data */
//}
//CEV_Capsule;



/*----------USER END FUNCTIONS---------*/


/** \brief fetches anything
 *
 * \param index : data index to fetch.
 * \param file : opened file to fetch from.
 *
 * \return void* on anything that was requested, NULL on error.
 * \return CEV_Capsule* if DEFAULT or DAT file is spotted.
 */
void* CEV_anyFetch(unsigned int index, FILE* file);


/** \brief fetches CEV_Capsule from file.
 *
 * \param index : data index to fetch.
* \param src : FILE* to read from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetch(unsigned int index, FILE* src, CEV_Capsule* dst);


/** \brief loads file into memory as it
 *
 * \param caps : CEV_Capsule* to store file.
 * \param fileName : file to be opened and stored.
 *
 * \return any of the function status.
 */
int CEV_capsuleLoad(CEV_Capsule* caps, const char* fileName);


/** \brief Extract exploitable data from capsule
 *
 * \param caps : CEV_Capsule* to extract file from
 * \param freeData : Destroys capsule content if true.
 *
 * \return void* on resulting object. NULL on error.
 * \note Capsule content is kept as if extraction fails.
 */
void* CEV_capsuleExtract(CEV_Capsule* caps, bool freeData);



            /*---SDL_Surfaces---*/


/** \brief Loads any picture file from BMP/PNG/JPG.
 *
 * \param fileName : file to open.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceLoad(const char* fileName);


/** \brief fetch pic as SDL_Surface from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetch(unsigned int index, const char* fileName);




            /*---SDL_Textures---*/

/** \brief Loads any picture file from BMP/PNG/JPG.
 *
 * \param fileName : file to open.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureLoad(const char* fileName);


/** \brief fetch pic as SDL_Texture from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetch(unsigned int index, const char* fileName);



          /*---CEV_Text---*/

/** \brief fetch text as CEV_Text from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
CEV_Text* CEV_textFetch(unsigned int index, const char* fileName);


        /*---TTF_Font from compiled file---*/

/** \brief fetch font as CEV_Font from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
CEV_Font* CEV_fontFetch(unsigned int index, const char* fileName);


        /*----WAV from compiled file----*/

/** \brief fetch wave as CEV_Chunk from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetch(unsigned int index, const char* fileName);


        /*---MUSIC from compiled file---*/

/** \brief fetch music as CEV_Music from compiled file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music *CEV_musicFetch(unsigned int index, const char* fileName);


        /*---Animations---*/

/** \brief loads animation set from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return SP_AnimList* on success, NULL on error.
 */
SP_AnimList* CEV_animListFetch(unsigned int index, char* fileName);


/** \brief loads gif from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetch(unsigned int index, char* fileName);

        /*---scroll---*/

/** \brief loads text scroller from file.
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText *CEV_scrollFetch(unsigned int index, char* fileName);

        /*---menu---*/

/** \brief Laods menu from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 *
 */
CEV_Menu *CEV_menuFetch(int index, char* fileName);

        /*---maps---*/

/** \brief Loads tile map from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 *
 */
CEV_TileMap *CEV_mapFetch(int index, char* fileName);

        /*---parallax background---*/

/** \brief Loads parallax from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 *
 */
CEV_Parallax *CEV_parallaxFetch(int index, char* fileName);


/** \brief Loads weather from file
 *
 * \param index : data index in file.
 * \param fileName : file to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather *CEV_weatherFetch(int index, char* fileName);


/*----- Encapsulation -----*/

/** \brief mem to file.
 *
 * \param caps : CEV_Capsule* to be dumped.
 * \param dst: FILE* to write into at actual position.
 * \return readWriteErr is set on error.
 *
 */
void CEV_capsuleWrite(CEV_Capsule *caps, FILE *dst);


/** \brief file to mem.
 *
 * \param caps : CEV_Capsule* to be filled.
 * \param src : FILE* to read from actual position.
 *
 * \return 0 on success, any value otherwise.
 * \note readWriteErr is set.
 */
int CEV_capsuleRead(FILE *src, CEV_Capsule *caps);


/** \brief file to mem.
 *
 * \param src : SDL_RWops* to read from actual position.
 * \param dst : CEV_Capsule* to be filled.
 *
 * \return void.
 */
void CEV_capsuleRead_RW(SDL_RWops* src, CEV_Capsule* dst);


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


/** \brief filename to enum type.
 *
 * \param fileName : name of file to deduce type from
 *
 * \return any of FILE_TYPE enum
 */
int CEV_fileToType(char* fileName);


/** \brief Attributes file extension from file type.
 *
 * \param type : FILE_TYPE to fetch extension from.
 *
 * \return char* : string with extension.
 * \note extension is given without '.' separator.
 */
char* CEV_fileTypeToExt(FILE_TYPE type);

#ifdef __cplusplus
}
#endif

#endif // PIC_LOAD_H_INCLUDED
