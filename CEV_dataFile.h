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
//**   CEV    |  02-2022      |   2.0.0  | ID compliant   **/
//**********************************************************/
//- CEV 2021 05 20- removed capsule data free from L_capsuleToXxx functions -> capsule cleared in calling functions.
//- CEV 2023 03 19- V2.0, now works with capsules IDs instead of index.

/** \file   CEV_dataFile.h
 * \author  CEV
 * \version 2.0.0
 * \date    March 2023
 * \brief   Multi file in one ressources.
 *
 * \details
 */

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
#include "CEV_menu.h"
#include "CEV_maps.h"
#include "CEV_weather.h"
#include "CEV_parallax.h"

//file content :
// uint32_tLE num of capsules
// num times :
//      uint32_t ID
//      uint32_t offset
// CEV_Capsules array

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Single capsule references.
 */
typedef struct CEV_RsrcFileHolderHeader
{
    uint32_t id;        /**< id for this capsule.*/
    size_t   offset;    /**< offset position in file of this capsule.*/
}
CEV_RsrcFileHolderHeader;


/** \brief Ressource file holder
 */
typedef struct CEV_RsrcFileHolder
{
    uint32_t numOfFiles;

    FILE* fSrc;

    CEV_RsrcFileHolderHeader* list;
}
CEV_RsrcFileHolder;




/** \brief Loads ressource file holder.
 *
 * \param fileName : const char* as name of file to load.
 * \param dst : CEV_RsrcFileHolder* to be filled with file informations.
 *
 * \return int : of std function status.
 */
int CEV_rsrcLoad(const char* fileName, CEV_RsrcFileHolder* dst);


/** \brief Clears / frees dst content/
 *
 * \param dst : CEV_RsrcFileHolder* to clear.
 *
 * \return void.
 */
void CEV_rsrcClear(CEV_RsrcFileHolder* dst);


/*----------USER END FUNCTIONS---------*/


/** \brief fetches anything
 *
 * \param id : data id to fetch.
 * \param src : ressource file holder.
 *
 * \return void* on anything that was requested, NULL on error.
 * \return CEV_Capsule* if DEFAULT or DAT file type is spotted.
 */
void* CEV_anyFetch(uint32_t id, CEV_RsrcFileHolder* src);


/** \brief fetches CEV_Capsule from file.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetch(uint32_t id, CEV_RsrcFileHolder* src, CEV_Capsule* dst);


/** \brief loads file into a capsule as it
 *
 * \param caps : CEV_Capsule* to store file.
 * \param fileName : file to be opened and stored.
 *
 * \return any of the function status.
 */
int CEV_capsuleFromFile(CEV_Capsule* caps, const char* fileName);


/** \brief Extracts exploitable data from capsule (texture, gif...)
 *
 * \param caps : CEV_Capsule* to extract file from
 * \param freeData : Destroys capsule content if true.
 *
 * \return void* on resulting object. NULL on error.
 * \note : Capsule content is kept as is if extraction fails.
 * freeData has no effect on data types which needs to keep raw file as is\n
 * (font, wav, music...).\n
 * Resulting pointer will need to be freed / destroyed accordingly to the data type provided.
 * Better to check the capsule's held data type before calling this function.
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


/** \brief fetches pic as SDL_Surface from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetch(uint32_t id, CEV_RsrcFileHolder* src);




            /*---SDL_Textures---*/

/** \brief Loads any picture file from BMP/PNG/JPG.
 *
 * \param fileName : file to open.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureLoad(const char* fileName);


/** \brief fetches pic as SDL_Texture from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetch(uint32_t id, CEV_RsrcFileHolder* src);



          /*---CEV_Text---*/

/** \brief fetches text as CEV_Text from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
CEV_Text* CEV_textFetch(uint32_t id, CEV_RsrcFileHolder* src);


        /*---TTF_Font from compiled file---*/

/** \brief fetches font as CEV_Font from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
CEV_Font* CEV_fontFetch(int32_t id, CEV_RsrcFileHolder* src);


        /*----WAV from compiled file----*/

/** \brief fetches wave as CEV_Chunk from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetch(int32_t id, CEV_RsrcFileHolder* src);


        /*---MUSIC from compiled file---*/

/** \brief fetches music as CEV_Music from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music *CEV_musicFetch(int32_t id, CEV_RsrcFileHolder* src);


        /*---Animations---*/

/** \brief loads animation set from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return SP_AnimList* on success, NULL on error.
 */
SP_AnimList* CEV_animListFetch(int32_t id, CEV_RsrcFileHolder* src);


/** \brief loads gif from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetch(int32_t id, CEV_RsrcFileHolder* src);

        /*---scroll---*/

/** \brief loads text scroller from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText *CEV_scrollFetch(int32_t id, CEV_RsrcFileHolder* src);

        /*---menu---*/

/** \brief Loads menu from from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 *
 */
CEV_Menu *CEV_menuFetch(int32_t id, CEV_RsrcFileHolder* src);

        /*---maps---*/

/** \brief Loads tile map from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 *
 */
CEV_TileMap *CEV_mapFetch(int32_t id, CEV_RsrcFileHolder* src);

        /*---parallax background---*/

/** \brief Loads parallax from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 *
 */
CEV_Parallax* CEV_parallaxFetch(int32_t id, CEV_RsrcFileHolder* src);


/** \brief Loads weather from ressources.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFileHolder* to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather *CEV_weatherFetch(int32_t id, CEV_RsrcFileHolder* src);


/*----- Encapsulation -----*/

/** \brief writes capsule into file.
 *
 * \param src : CEV_Capsule* to be written.
 * \param dst : FILE* to write into at actual position.
 *
 * \return readWriteErr is set on error.
 *
 * \note file is written as it is, it is necessary to place yourself
 * in the file before writing.
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
 * in the file before reading.
 */
void CEV_capsuleTypeRead(FILE *src, CEV_Capsule *dst);


/** \brief writes capsule into virtual file.
 *
 * \param src : CEV_Capsule* to read from.
 * \param dst : SDL_RWops* to write into.
 *
 * \return void.
 *
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


/** \brief Convert id into file type.
 *
 * \param id : uint32_t as id to convert.
 *
 * \return int : as any of FILE_TYPE enum.
 *
 */
int CEV_idTofType(uint32_t id);


/** \brief Select file extension for this id
 *
 * \param id : uint32_t as id to get file extension.
 *
 * \return char* : as extension for a file name.
 * \note : extension is given without '.' separator.
 */
char* CEV_idToExt(uint32_t id);


/** \brief Create base id from file type.
 *
 * \param type : int to create id from.
 *
 * \return uint32_t as id.
 * \note only the 'type' field of id is completed.
 */
uint32_t CEV_ftypeToId(int type);

/** \brief filename to enum type.
 *
 * \param filename : name of file to deduce type from
 *
 * \return any of FILE_TYPE enum
 */
int CEV_fTypeFromName(char* fileName);


/** \brief Attributes file extension from file type.
 *
 * \param type : FILE_TYPE to fetch extension from.
 *
 * \return char* : string with extension.
 * \note : extension is given without '.' separator.
 */
char* CEV_fTypeToExt(int type);

#ifdef __cplusplus
}
#endif

#endif // PIC_LOAD_H_INCLUDED
