//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  25-09-2022   |   1.0    |    creation    **/
//**   CEV    |  15-09-2023   |   1.1    |        #1      **/
//**********************************************************/

/*
#1 - CEV_Animini own spriteMini removed.

*/

#ifndef CEV_SHORTANIM_H_INCLUDED
#define CEV_SHORTANIM_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "CEV_texts.h"

//file IS_ANI (17)

#define ANI_TYPE_ID (IS_ANI<<24)
#define IS_ANI_ID(x) ((x & 0xFF000000) == ANI_TYPE_ID)


/**cahier des charges :
- 2 lignes d'animation max.
- texture en multiple d'une image.
- même largeur / hauteur pour les 2 lignes d'animation.
- stop / play. (arrête et redémarre sur image actuelle).
- play actif à la création.
- changement de ligne au même index d'image.
*/



/** fichier data .ani

U32 : id (type & id)                    0x00
    : id fichier image (embarquée si 0) 0x04
    : delay                             0x08

S32 : time offset                       0x0C
U8  : nombre d'animation                0x10
    : num images animation 0            0x11
    : num images animation 1            0x12

CEV_capsule : fichier image si embarquée
*/


typedef struct S_CEV_AniMini CEV_AniMini;


/** \brief short animation single instance.
 */
typedef struct S_CEV_SpriteMini
{//animation instance

    bool *switchPtr,    /**< enables external switch command, overrides switchAnim */
         *playPtr,      /**< enables external play command, overrides play */
         switchAnim,    /**< enables 2nd line of animation */
         play;          /**< enables animation playing */

    uint8_t frameAct;     /**< active picture index */

    int timeOffset;     /**< time offset to sync */

    SDL_Rect clip;      /**< single frame clip */

    CEV_AniMini *cst;  /**< link to its constants */

}
CEV_SpriteMini;


/** \brief short animation reference constants.
 */
struct S_CEV_AniMini
{//animation constants
    //bool isSync;        /**< animation sync'ed with absolute time */

    uint8_t numOfAnim,  /**< num of anim in animation */
            numOfFrame[2];/**< num of pic per animation */

    uint32_t id,        /**< unique own id */
             delay,     /**< delay between pics (ms) */
             picId;     /**< src picture id if any */

    int timeOffset;     /**< time offset to sync */

    SDL_Rect clip,      /**< single frame clip (top left animation rect)*/
             picDim;    /**< texture dimensions */

    SDL_Texture *pic;    /**< texture with animation */
    // #1 CEV_SpriteMini sprite;  /**< self instance */
};


/** \brief Debug / stress test.
 *
 * \param void
 * \return void
 */
void TEST_shortAnim(void);


/** \brief Dumps CEV_AniMini structure content.
 *
 * \param this : CEV_AniMini* to dump.
 *
 * \return void.
 *
 * \note Result is dumped into stdout.
 */
void CEV_aniMiniDump(CEV_AniMini* this);


/** \brief Dumps CEV_SpriteMini structure content.
 *
 * \param this : CEV_AniMiniC* to dump.
 *
 * \return void.
 *
 * \note Result is dumped into stdout.
 */
void CEV_spriteMiniDump(CEV_SpriteMini* this);


/*** Animation functions ***/

/** \brief Destroys Animini content and itself.
 *
 * \param this : CEV_AniMini* to destroy.
 *
 * \return void.
 *
 * \warning Picture is not freed if referenced with id.
 */
void CEV_aniMiniDestroy(CEV_AniMini *this);


/** \brief Clear / frees structure content.
 *
 * \param this : CEV_AniMini* to clear.
 *
 * \return void.
 *
 * \warning Picture is not freed if referenced with id.
 */
void CEV_aniMiniClear(CEV_AniMini *this);



/** \brief Loads CEV_AniMini from file.
 *
 * \param fileName : const char* as path and name of file to be loaded.
 *
 * \return CEV_AniMini* on success, NULL on failure.
 */
CEV_AniMini* CEV_aniMiniLoad(const char *fileName);


/** \brief Loads CEV_AniMini from RWops.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool to close RWops if true.
 *
 * \return CEV_AniMini* on success, NULL on failure.
 *
 * \note If requested, src is closed weither function succeeds or not.
 */
CEV_AniMini* CEV_aniMiniLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Saves CEV_AniMini into file.
 *
 * \param src : CEV_AniMini* to be saved.
 * \param fileName : const char* as path and file name of resulting file.
 *
 * \return int of standard function status.
 */
int CEV_aniMiniSave(CEV_AniMini *src, const char *fileName);


/** \brief Reads CEV_AniMini from file and fills structure.
 *
 * \param src : FILE* to read from.
 * \param dst : CEV_AniMini* to fill with file content.
 *
 * \return int of standard function status
 *
 * \warning No free operation are performed on dst, thus dst should be cleared before
 beeing filled from file.
 */
int CEV_aniMiniTypeRead(FILE* src, CEV_AniMini* dst);


/** \brief Writes CEV_AniMini into file.
 *
 * \param src : CEV_AniMini* to write into file.
 * \param dst : FILE* to write into.
 *
 * \return int of standard function status.
 *
 * \note readWriteErr is updated by this function.
 */
int CEV_aniMiniTypeWrite(CEV_AniMini *src, FILE* dst);


/** \brief Reads CEV_AniMini from RWops and fills structure.
 *
 * \param src : SDL_RWops*
 * \param dst : CEV_AniMini* to fill with file content.
 * \param freeSrc : bool weither to close RWops or not.
 *
 * \return int of standard function status.
 *
 * \note If requested, src is closed weither function succeeds or not.
 */
int CEV_aniMiniTypeRead_RW(SDL_RWops* src, CEV_AniMini* dst, bool freeSrc);//freed weither succeeds or fails if requested


/** \brief Write CEV_AniMini structure into RWops.
 *
 * \param src : CEV_AniMini* to be written.
 * \param dst : SDL_RWops* to write into.
 *
 * \return int of standard function status.
 */
int CEV_aniMiniTypeWrite_RW(CEV_AniMini* src, SDL_RWops* dst);


/** \brief Converts editable text file into data file.
 *
 * \param srcName : const char* as path and file name of source file.
 * \param dstName : const char* as path and file name of resulting file.
 *
 * \return int of standard function status.
 *
 * \note file extension should be ".ani" to ensure recognition on auto-load.
 */
int CEV_aniMiniConvertToData(const char *srcName, const char *dstName);


/** \brief Writes to data file from CEV_Text.
 *
 * \param src : CEV_Text* build from txt file.
 * \param dst : FILE* as destination file.
 * \param srcName : char* as name of file of src.
 *
 * \return int of standard function status.
 *
 * \note src & dst are not freed in this function.
 */
int CEV_aniMiniConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName);


/** \brief Export aniMini as editable file.
 *
 * \param src : AniMini* to export.
 * \param dstName : const char* as path and resulting fileName.
 *
 * \return int  as sdt funcSts.
 *
 * \note picture is produced  next to resulting file
 * random name given to embedded picture if any.
 */
int CEV_aniMiniExport(CEV_AniMini *src, const char *dstName);


/** \brief Sets texture to be used for animation.
 *
 * \param src : SDL_Texture* to be used for animation.
 * \param dst : CEV_AniMini* to hold texture for animation.
 *
 * \return int of standard function status.
 *
 * \note Caculation are performed, thus direct attribution of SDL_Texture
 into structure may fail.
 *
* \warning Any src without ID will be freed upon attachment / clear / destroy operation.
 Id'd resources will have to be freed at upper level.
 */
int CEV_aniMiniAttachTexture(SDL_Texture* src, CEV_AniMini *dst);


/** \brief Sets animation paraameters for animation.
 *
 * \param picNum_0 : uint8_t as num of picture in first animation.
 * \param picNum_1 : uint8_t as num of picture in second animation.
 * \param dst : CEV_AniMini* to be parametered.
 *
 * \return int of standard function status.
 *
 * \note Caculation are performed, thus direct attribution of values
 into structure may fail.\n
    Should be called AFTER texture has been set.
 */
int CEV_aniMiniParamSet(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMini* dst);


    /** sprite mini **/

/** \brief New alloc'd instance for this animation.
 *
 * \param src : CEV_AniMini* to build instance from.
 *
 * \return : CEV_SpriteMini* accordingly filled or NULL on failure.
 */
CEV_SpriteMini* CEV_spriteMiniCreateFrom(CEV_AniMini* src);


/** \brief Destroys SpriteMini content and itself.
 *
 * \param this : CEV_SpriteMini* to destroy.
 *
 * \return void.
 */
void CEV_spriteMiniDestroy(CEV_SpriteMini* this);


/** \brief New instance for this animation.
 *
 * \param src : CEV_AniMini* to build instance from.
 * \param dst : CEV_SpriteMini* as new sprite instance.
 *
 * \return : CEV_SpriteMini accordingly filled.
 */
int CEV_spriteMiniFrom(CEV_AniMini* src, CEV_SpriteMini* dst);


/** \brief Clears / reset structure content.
 *
 * \param src : CEV_SpriteMini* to be cleared.
 *
 * \return void
 */
void CEV_spriteMiniClear(CEV_SpriteMini* src);


/** \brief Updates structure status.
 *
 * \param this : CEV_SpriteMini* to be updated.
 * \param now : uint32_t as reference time (ms).
 *
 * \return SDL_Rect as actual clip for animation.
 */
SDL_Rect CEV_spriteMiniUpdate(CEV_SpriteMini* this, uint32_t now);


/** \brief Displays / updates mini sprite
 *
 * \param this : CEV_SpriteMini* to display
 * \param dstRect : SDL_Rect as blit position.
 * \param now : uint32_t as reference time (ms).
 *
 * \return void
 *
 * \note this function preforms update, thus CEV_spriteMiniUpdate() is not necessary if used.
 */
void CEV_spriteMiniBlit(CEV_SpriteMini* this, SDL_Rect dstRect, uint32_t now);


/** \brief Enable sprite to be animated.
 *
 * \param this : CEV_SpriteMini* to animate.
 *
 * \return void.
 *
 * \note Animation may restart at any frame of anuimation.
 */
void CEV_spriteMiniPlay(CEV_SpriteMini* this);


/** \brief Stops sprite animation.
 *
 * \param this : CEV_SpriteMini* to stop playing.
 *
 * \return void.
 *
 * \note Stopped animation will show frame 0 - switching animation is disabled..
 */
void CEV_spriteMiniStop(CEV_SpriteMini* this);


/** \brief Attaches external playing command.
 *
 * \param playCmd : bool* as external command.
 * \param dst : CEV_SpriteMini* to attach to.
 *
 * \return void.
 *
 * \note any external command overrides internal state.
 */
void CEV_spriteMiniAttachPlayCmd(bool* playCmd, CEV_SpriteMini* dst);


/** \brief Attaches external animation switch command.
 *
 * \param switchCmd : bool* as external command.
 * \param dst : CEV_SpriteMini* to attach to.
 *
 * \return void
 *
 * \note any external command overrides internal state.
 */
void CEV_spriteMiniAttachSwitchCmd(bool* switchCmd, CEV_SpriteMini* dst);

#endif // CEV_SHORTANIM_H_INCLUDED
