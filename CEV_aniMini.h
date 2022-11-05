//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  25-09-2022   |   1.0    |    creation    **/
//**********************************************************/

#ifndef CEV_SHORTANIM_H_INCLUDED
#define CEV_SHORTANIM_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>


/**cahier des charges :
- 2 lignes d'animation max.
- texture en multiple d'une image.
- même largeur / hauteur pour les 2 lignes d'animation.
- stop / play. (arrête et redémarre sur image actuelle).
- play actif à la création.
- changement de ligne au même index d'image.
*/



/** fichier data .ani

U32 : ID (type & id)
    : ID fichier image (embarquée si 0)
    : delay

S32 : time offset
U8  : nombre d'animation
    : num images animation 0
    : num images animation 1

CEV_capsule : fichier image
*/


typedef struct CEV_AniMiniCst CEV_AniMiniCst;


/** \brief short animation single instance.
 */
typedef struct CEV_aniMini
{//animation instance

    bool switchAnim,    /**< enables 2nd line of animation */
         play;          /**< enables animation playing */

    uint32_t picAct,     /**< active picture index */
            cstID;       /**< its constants ID */

    int timeOffset;     /**< time offset to sync */

    SDL_Rect clip,      /**< single frame clip */
             blit;      /**< optional own blit position */

    CEV_AniMiniCst *cst;/**< link to its constants */

}
CEV_AniMini;


/** \brief short animation reference constants.
 *
 * note : holds a mini animation instance.
 */
struct CEV_AniMiniCst
{//animation constants
    //bool isSync;        /**< animation sync'ed with absolute time */

    uint32_t ID,        /**< unique Id */
             delay,     /**< delay between pics (ms) */
             srcID;     /**< src picture ID if any */

    uint8_t numOfAnim,  /**< num of anim in animation */
            numOfPic[2];/**< num of pic per animation */

    int timeOffset;     /**< time offset to sync */

    SDL_Rect clip,      /**< single frame clip (top left animation rect)*/
             picDim;    /**< texture dimensions */

    SDL_Texture *srcPic;/**< texture with animation */
    CEV_AniMini anim;   /**< self instance */
};


/**dumping / debug */

void TEST_shortAnim(void);


/** \brief Dumps CEV_AniMiniCst structure content.
 *
 * \param in : CEV_AniMiniCst* to dump.
 *
 * \return N/A.
 *
 * note : is dumped into stdin.
 */
void CEV_aniMiniCstDump(CEV_AniMiniCst* in);


/** \brief Dumps CEV_AniMini structure content.
 *
 * \param in : CEV_AniMiniC* to dump.
 *
 * \return N/A.
 *
 * note : is dumped into stdin.
 */
void CEV_aniMiniDump(CEV_AniMini* in);


/** \brief Destroys constant content and itself.
 *
 * \param this CEV_AniMiniCst*
 *
 * \return void
 */
void CEV_aniMiniCstDestroy(CEV_AniMiniCst *this);


/** \brief Clear / frees structure content.
 *
 * \param this : CEV_AniMiniCst* to clear;
 *
 * \return N/A.
 */
void CEV_aniMiniCstClear(CEV_AniMiniCst *this);


/** \brief Loads mini animation cst from file.
 *
 * \param fileName : char* with name of file to be loaded.
 *
 * \return CEV_AniMiniCst* on success, NULL on error.
 */
CEV_AniMiniCst* CEV_aniMiniLoad(char *fileName);


/** \brief Loads mini animation from virtual file.
 *
 * \param src : SDL_RWops* as virtual file to read from.
 * \param freeSrc : bool closes RWops src if true.
 *
 * \return CEV_AniMiniCst* on success, NULL on error.
 */
CEV_AniMiniCst* CEV_aniMiniLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Saves mini animation to file.
 *
 * \param src CEV_AniMiniCst*
 * \param fileName char*
 * \param embedPic bool
 *
 * \return int of standard function status.
 */
int CEV_aniMiniCstSave(CEV_AniMiniCst *src, char *fileName, bool embedPic);


/** \brief Writes CEV_AniMiniCst into file.
 *
 * \param src : CEV_AniMiniCst* to write into file.
 * \param dst : FILE* to write into.
 * \param embedPic :  bool embed texture into file if true:
 *
 * \return int of standard function status.
 */
int CEV_aniMiniCstTypeWrite(CEV_AniMiniCst *src, FILE* dst, bool embedPic);


/** \brief Sets texture to be used for animation.
 *
 * \param src : SDL_Texture* to be used for animation.
 * \param dst : CEV_AniMiniCst* to hold texture for animation.
 *
 * \return int of standard function status.
 *
 * note : Caculation are performed, thus direct attribution of SDL_Texture
 * into structure may fail.
 */
int CEV_aniMiniSetTexture(SDL_Texture* src, CEV_AniMiniCst *dst);


/** \brief Sets animation parameters for animation.
 *
 * \param picNum_0 : uint8_t as num of picture in first animation.
 * \param picNum_1 : uint8_t as num of picture in second animation.
 * \param dst : CEV_AniMiniCst* to be parametered.
 *
 * \return int of standard function status.
 *
 * note : Caculation are performed, thus direct attribution of values
 * into structure may fail.
 * Should be called AFTER texture has been set.
 */
int CEV_aniMiniSetParam(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMiniCst* dst);


/** \brief Force picture display
 *
 * \param dst : CEV_aniMini* to force view from.
 * \param picIndex : int as which picture index to display.
 * \param switchView : bool to show alternate view.
 *
 * \return void
 *
 * note will need CEV_aniMiniPlay to restart.
 */
void CEV_aniMiniPicForce(CEV_aniMini* dst, int picIndex, bool switchView);


/** \brief New instance for this animation.
 *
 * \param src : CEV_AniMiniCst* to build instance from.
 *
 * \return : CEV_AniMini accordingly filled.
 */
CEV_AniMini CEV_aniMiniCreateFrom(CEV_AniMiniCst* src);


/** \brief Clears / reset structure content.
 *
 * \param this : CEV_AniMini* to be cleared.
 *
 * \return void
 */
void CEV_aniMiniClear(CEV_AniMini* this);


/** \brief Updates structure status.
 *
 * \param src : CEV_AniMini* to be updated.
 * \param now : uint32_t as reference time (ms).
 *
 * \return SDL_Rect as actual clip for animation.
 *
 */
SDL_Rect CEV_aniMiniUpdate(CEV_AniMini* src, uint32_t now);


/** \brief Enables / stops playing.
 *
 * \param src : CEV_AniMini* to control.
 * \param play : true enbales playing / false stops.
 *
 * \return bool as playing status.
 */
bool CEV_aniMiniPlay(CEV_AniMini* src, bool play);


/*
Toto* CEV_totoCreate(void);

void CEV_totoDestroy(Toto* in);//NULL compliant

void CEV_totoClear(Toto* in);

Toto* CEV_totoLoad(char* fileName);

Toto* CEV_totoLoad_RW(SDL_RWops* src, bool freeSrc);//free success or failure

int CEV_totoSave(Toto* src, char fileName);

int CEV_totoTypeRead(FILE* src, Toto* dst);

int CEV_totoTypeWrite(Toto* src, FILE* dst);

int CEV_totoTypeRead_RW(SDL_RWops* src, Toto* dst, bool freeSrc);//free success or failure

int CEV_totoTypeWrite_RW(Toto* src, SDL_RWops* dst);

int CEV_totoToCapsule(Toto* src, CEV_Capsule *dst);

int CEV_convertTotoCSVToData(char* srcName, char* dstName);*/

#endif // CEV_SHORTANIM_H_INCLUDED
