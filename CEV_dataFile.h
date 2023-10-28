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
 * \brief   Multi file in one resources.
 *
 * \details
 */

#ifndef CEV_FILE_LOAD_H_INCLUDED
#define CEV_FILE_LOAD_H_INCLUDED

#include <stdint.h>
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
typedef struct CEV_RsrcFileHeader
{
    uint32_t id;        /**< id for this capsule.*/
    size_t   offset;    /**< offset position in file of this capsule.*/
}
CEV_RsrcFileHeader;


/** \brief Ressource file holder.
 */
typedef struct CEV_RsrcFile
{
    uint32_t numOfFiles;    /**< Num of files embedded in this file. */

    FILE* fSrc;             /**< File holding resources files. */

    CEV_RsrcFileHeader* list; /**< Array of Ids and corresponding offsets */
}
CEV_RsrcFile;




/** \brief Loads resource file holder.
 *
 * \param fileName : const char* as name of file to load.
 * \param dst : CEV_RsrcFile* to be filled with file informations.
 *
 * \return int : of std function status.
 */
int CEV_rsrcLoad(const char* fileName, CEV_RsrcFile* dst);


/** \brief Clears / frees dst content.
 *
 * \param dst : CEV_RsrcFile* to clear.
 *
 * \return void.
 */
void CEV_rsrcClear(CEV_RsrcFile* dst);


/*----------USER END FUNCTIONS---------*/


/** \brief Fetches anything based on id.
 *
 * \param id : data id to fetch.
 * \param src : resource file holder.
 *
 * \return void* on anything that was requested, NULL on error.
 * \note Returns CEV_Capsule* if DEFAULT or DAT file type is spotted.
 */
void* CEV_anyIdFetch(uint32_t id, CEV_RsrcFile* src);


/** \brief Fetches anything based on index.
 *
 * \param index : data index to fetch.
 * \param src : resource file holder.
 *
 * \return void* on anything that was requested, NULL on error.
 * \note Returns CEV_Capsule* if DEFAULT or DAT file type is spotted.
 */
void* CEV_anyFetchByIndex(uint32_t index, CEV_RsrcFile* src);


/** \brief Fetches anything from file based on id.
 *
 * \param id : uint32_t data id to fetch.
 * \param fileName : const char* as file name to extract from.
 *
 * \return void* on anything that was requested, NULL on error.
 * \note Returns CEV_Capsule* if DEFAULT or DAT file type is spotted.
 */
void* CEV_anyIdFetchFromFile(uint32_t id, const char* fileName);


/** \brief Fetches anything from file based on index.
 *
 * \param index : uint32_t data index to fetch.
 * \param fileName : const char* as file name to extract from.
 *
 * \return void* on anything that was requested, NULL on error.
 * \note Returns CEV_Capsule* if DEFAULT or DAT file type is spotted.
 */
void* CEV_anyFetchByIndexFromFile(uint32_t index, const char* fileName);


/** \brief Fetches CEV_Capsule from resource file holder based on Id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetchById(uint32_t id, CEV_RsrcFile* src, CEV_Capsule* dst);


/** \brief Fetches CEV_Capsule from resource file holder based on index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetchByIndex(uint32_t index, CEV_RsrcFile* src, CEV_Capsule* dst);


/** \brief Fetches CEV_Capsule from file based on id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetchByIdFromFile(uint32_t id, const char* fileName, CEV_Capsule* dst);


/** \brief Fetches CEV_Capsule from file based on index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 * \param dst : CEV_Capsule* to receive raw memory.
 *
 * \return any of the functions status.
 */
int CEV_capsuleFetchByIndexFromFile(uint32_t index, const char* fileName, CEV_Capsule* dst);


/** \brief loads file into a capsule as it
 *
 * \param caps : CEV_Capsule* to store file.
 * \param fileName : file to be opened and stored.
 *
 * \return any of the function status.
 */
int CEV_capsuleFromFile(CEV_Capsule* caps, const char* fileName);


/** \brief Extracts exploitable data from capsule (texture, gif...).
 *
 * \param caps : CEV_Capsule* to extract file from.
 * \param freeData : Destroys capsule content if true.
 *
 * \return void* on resulting object. NULL on error.
 *
 * \note Capsule content is kept as is if extraction fails.
  freeData has no effect on data types which needs to keep raw file as is\n
  (font, wav, music...).\n
  Resulting pointer will need to be freed / destroyed accordingly to the data type provided.\n
  Better to check the capsule's held data type before calling this function.\n
 */
void* CEV_capsuleExtract(CEV_Capsule* caps, bool freeData);



            /*---SDL_Surfaces---*/


/** \brief Loads SDL_Surface from BMP/PNG/JPG.
 *
 * \param fileName : file to open.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceLoad(const char* fileName);


/** \brief Fetches SDL_Surface from resources based on id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetchById(uint32_t id, CEV_RsrcFile* src);


/** \brief Fetches SDL_Surface from resources based on index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetchIndex(uint32_t index, CEV_RsrcFile* src);


/** \brief Fetches SDL_Surface from file based on id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetchByIdFromFile(uint32_t id, const char* fileName);


/** \brief Fetches SDL_Surface from resources based on index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SDL_Surface* on success, NULL on error.
 */
SDL_Surface* CEV_surfaceFetchByIndexFromFile(uint32_t index, const char* fileName);


/** \brief Loads picture as SDL_Surface from CEV_Capsule.
 *
 * \param src : CEV_Capsule* to extract from.
 * \param freeSrc : bool that frees src content if true.
 *
 * \return SDL_Surface* on success, NULL on failure.
 * \note If freeSrc is true, src is cleared weither function succeed or not.
 */
SDL_Surface* CEV_surfaceFromCapsule(CEV_Capsule* src, bool freeSrc);

            /*---SDL_Textures---*/

/** \brief Loads SDL_Texture from BMP/PNG/JPG.
 *
 * \param fileName : file to open.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureLoad(const char* fileName);


/** \brief Fetches SDL_Texture from resources based on id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetchById(uint32_t id, CEV_RsrcFile* src);


/** \brief Fetches SDL_Texture from resources based on index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetchByIndex(uint32_t index, CEV_RsrcFile* src);


/** \brief Fetches SDL_Texture from file based on id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetchByIdFromFile(uint32_t id, const char* fileName);


/** \brief Fetches SDL_Texture from file based on index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SDL_Texture* on success, NULL on error.
 */
SDL_Texture* CEV_textureFetchByIndexFromFile(uint32_t index, const char* fileName);


          /*---CEV_Text---*/

/** \brief Fetches CEV_Text from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
CEV_Text* CEV_textFetchById(uint32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Text from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
CEV_Text* CEV_textFetchByIndex(uint32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Text from file based on id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
CEV_Text* CEV_textFetchByIdFromFile(uint32_t id, const char* fileName);


/** \brief Fetches CEV_Text from file based on index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Text* on success, NULL on error.
 */
 CEV_Text* CEV_textFetchByIndexFromFile(uint32_t index, const char* fileName);


        /*---TTF_Font from compiled file---*/

/** \brief Fetches CEV_Font from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
CEV_Font* CEV_fontFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Font from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
CEV_Font* CEV_fontFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Font from file based on id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
 CEV_Font* CEV_fontFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_Font from file based on index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Font* on success, NULL on error.
 */
 CEV_Font* CEV_fontFetchByIndexFromFile(int32_t index,  const char* fileName);


        /*----WAV from compiled file----*/

/** \brief Fetches CEV_Chunk from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Chunk from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Chunk from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetchByIdFromFile(int32_t id, const char* fileName);


/** \brief Fetches CEV_Chunk from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Chunk* on success, NULL on error.
 */
CEV_Chunk* CEV_waveFetchByIndexFromFile(int32_t index, const char* fileName);


        /*---MUSIC from compiled file---*/

/** \brief Fetches CEV_Music from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music *CEV_musicFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Music from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music* CEV_musicFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Music from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music* CEV_musicFetchByIdFromFile(int32_t id, const char* fileName);


/** \brief FetchesCEV_Music from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Music* on success, NULL on error.
 */
CEV_Music* CEV_musicFetchByIndexFromFile(int32_t index, const char* fileName);

        /*---Animations---*/

/** \brief Fetches SP_Anim from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SP_Anim* on success, NULL on error.
 */
SP_Anim* CEV_animFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches SP_Anim from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return SP_Anim* on success, NULL on error.
 */
SP_Anim* CEV_animFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches SP_Anim from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SP_Anim* on success, NULL on error.
 */
SP_Anim* CEV_animFetchByIdFromFile(int32_t id, const char* fileName);


/** \brief Fetches SP_Anim from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return SP_Anim* on success, NULL on error.
 */
SP_Anim* CEV_animFetchByIndexFromFile(int32_t index, const char* fileName);


/*----gif animation from compiled file----*/


/** \brief Fetches CEV_GifAnim from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_GifAnim from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_GifAnim from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetchByIdFromFile(int32_t id, const char* fileName);


/** \brief Fetches CEV_GifAnim from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_GifAnim* on success, NULL on error.
 */
CEV_GifAnim* CEV_gifFetchByIndexFromFile(int32_t index, const char* fileName);

        /*---scroll---*/

/** \brief Fetches CEV_ScrollText from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText *CEV_scrollFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_ScrollText from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
CEV_ScrollText* CEV_scrollFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_ScrollText from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
 CEV_ScrollText* CEV_scrollFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_ScrollText from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_ScrollText* on success, NULL on error.
 */
 CEV_ScrollText* CEV_scrollFetchByIndexFromFile(int32_t index, const char* fileName);

        /*---menu---*/

/** \brief Fetches CEV_Menu from from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 *
 */
CEV_Menu *CEV_menuFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Menu from from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 *
 */
CEV_Menu* CEV_menuFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Menu from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 */
 CEV_Menu* CEV_menuFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_Menu from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Menu* on success, NULL on error.
 */
 CEV_Menu* CEV_menuFetchByIndexFromFile(int32_t index, const char* fileName);


        /*---maps---*/

/** \brief Fetches CEV_TileMap from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 */
CEV_TileMap *CEV_mapFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_TileMap from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 */
 CEV_TileMap* CEV_mapFetchByIndex(int32_t index, CEV_RsrcFile* src);


 /** \brief Fetches CEV_TileMap from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 */
 CEV_TileMap* CEV_mapFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_TileMap from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_TileMap* on success, NULL on error.
 */
 CEV_TileMap* CEV_mapFetchByIndexFromFile(int32_t index, const char* fileName);


        /*---parallax background---*/

/** \brief Fetches CEV_Parallax from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 */
CEV_Parallax* CEV_parallaxFetchById(int32_t id, CEV_RsrcFile* src);


/** \brief Fetches CEV_Parallax from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 */
CEV_Parallax* CEV_parallaxFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Parallax from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 */
CEV_Parallax* CEV_parallaxFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_Parallax from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Parallax* on success, NULL on error.
 */
CEV_Parallax* CEV_parallaxFetchByIndexFromFile(int32_t index, const char* fileName);

    /*---- Weather ----*/


/** \brief Fetches CEV_Weather from resources by id.
 *
 * \param id : data id to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather *CEV_weatherFetchById(int32_t id, CEV_RsrcFile* src);



/** \brief Fetches CEV_Weather from resources by index.
 *
 * \param index : data index to fetch.
 * \param src : CEV_RsrcFile* to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather* CEV_weatherFetchByIndex(int32_t index, CEV_RsrcFile* src);


/** \brief Fetches CEV_Weather from file by id.
 *
 * \param id : data id to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather* CEV_weatherFetchByIdFromFile(int32_t id, const char* fileName);


 /** \brief Fetches CEV_Weather from file by index.
 *
 * \param index : data index to fetch.
 * \param fileName : name of file to fetch from.
 *
 * \return CEV_Weather* on success, NULL on error.
 */
CEV_Weather* CEV_weatherFetchByIndexFromFile(int32_t index, const char* fileName);

/*----- Encapsulation -----*/

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


/** \brief Convert id into file type.
 *
 * \param id : uint32_t as id to convert.
 *
 * \return int : as any of FILE_TYPE enum.
 */
int CEV_idTofType(uint32_t id);


/** \brief Select file extension for this id
 *
 * \param id : uint32_t as id to get file extension.
 *
 * \return char* : as extension for a file name.
 *
 * \note extension is given without '.' separator.
 */
char* CEV_idToExt(uint32_t id);


/** \brief Create base id from file type.
 *
 * \param type : int to create id from.
 *
 * \return uint32_t as id.*
 *
 * \note only the 'type' field of id is completed.
 */
uint32_t CEV_ftypeToId(int type);


/** \brief filename to enum type.
 *
 * \param filename : name of file to deduce type from
 *
 * \return any of FILE_TYPE enum
 */
int CEV_fTypeFromName(const char* fileName);


/** \brief Attributes file extension from file type.
 *
 * \param type : FILE_TYPE to fetch extension from.
 *
 * \return char* : string with extension.
 * \note extension is given without '.' separator.
 */
char* CEV_fTypeToExt(int type);

#ifdef __cplusplus
}
#endif

#endif // PIC_LOAD_H_INCLUDED
