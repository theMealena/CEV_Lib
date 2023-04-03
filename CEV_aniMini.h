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

//file IS_ANI (17)


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

CEV_capsule : fichier image si embarquée
*/


typedef struct S_CEV_AniMini CEV_AniMini;

/** \brief short animation single instance.
 */
typedef struct S_CEV_SpriteMini
{//animation instance

    bool switchAnim,    /**< enables 2nd line of animation */
         play;          /**< enables animation playing */

    uint8_t picAct;     /**< active picture index */

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
            numOfPic[2];/**< num of pic per animation */

    uint32_t ID,        /**< unique Id */
             delay,     /**< delay between pics (ms) */
             srcID;     /**< src picture ID if any */

    int timeOffset;     /**< time offset to sync */

    SDL_Rect clip,      /**< single frame clip (top left animation rect)*/
             picDim;    /**< texture dimensions */

    SDL_Texture *srcPic;/**< texture with animation */
    CEV_SpriteMini sprite;   /**< self instance */
};


/**dumping / debug */

void TEST_shortAnim(void);


/** \brief Dumps CEV_AniMini structure content.
 *
 * \param in : CEV_AniMini* to dump.
 *
 * \return N/A.
 *
 * note : is dumped into stdin.
 */
void CEV_aniMiniDump(CEV_AniMini* in);


/** \brief Dumps CEV_SpriteMini structure content.
 *
 * \param in : CEV_AniMiniC* to dump.
 *
 * \return N/A.
 *
 * note : is dumped into stdin.
 */
void CEV_spriteMiniDump(CEV_SpriteMini* in);


/*** Animation functions ***/

/** \brief Destroys Animini and content
 *
 * \param src : CEV_AniMini* to destroy
 *
 * \return N/A.
 *
 * \note picture is not freed if referenced.
 */
void CEV_aniMiniDestroy(CEV_AniMini *src);


/** \brief Clear / frees structure content.
 *
 * \param dst : CEV_AniMini* to clear;
 * \param freePic : bool free embedded Texture if true:
 *
 * \return N/A.
 */
void CEV_aniMiniClear(CEV_AniMini *dst, bool freePic);

/*
/** \brief Loads and creates mini animation from file.
 *
 * \param fileName : char*  with name of file to be loaded.
 *
 * \return CEV_AniMini* as result or NULL on failure.
 */
//CEV_AniMini* CEV_aniMiniLoadCreate(char *fileName);


/** \brief Loads mini animation from file.
 *
 * \param fileName : char* with name of file to be loaded.
 *
 * \return CEV_AniMini* on success, NULL if failed.
 */
CEV_AniMini* CEV_aniMiniLoad(char *fileName);


/** \brief Loads mini animation from virtual file.
 *
 * \param src : SDL_RWops* as virtual file to read from.
 * \param freeSrc : bool closes RWops src if true.
 *
 * \return CEV_AniMini* on success, NULL if failed.
 */
CEV_AniMini* CEV_aniMiniLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Saves mini animation to file.
 *
 * \param src CEV_AniMini*
 * \param fileName char*
 * \param embedPic bool
 *
 * \return int of standard function status.
 */
int CEV_aniMiniSave(CEV_AniMini *src, char *fileName, bool embedPic);


int CEV_aniMinConvertTxtToData(char *srcName, char *dstName);


/** \brief Writes CEV_AniMini into file.
 *
 * \param src : CEV_AniMini* to write into file.
 * \param dst : FILE* to write into.
 * \param embedPic :  bool embed texture into file if true:
 *
 * \return int of standard function status.
 */
int CEV_aniMiniTypeWrite(CEV_AniMini *src, FILE* dst, bool embedPic);


/** \brief Sets texture to be used for animation.
 *
 * \param src : SDL_Texture* to be used for animation.
 * \param dst : CEV_AniMini* to hold texture for animation.
 *
 * \return int of standard function status.
 *
 * note : Caculation are performed, thus direct attribution of SDL_Texture
 * into structure may fail.
 * Any existing texture is destroyed and replaced if nor referenced (srcID = nul).
 */
int CEV_aniMiniTextureAttach(SDL_Texture* src, CEV_AniMini *dst);


/** \brief Sets animation paraameters for animation.
 *
 * \param picNum_0 : uint8_t as num of picture in first animation.
 * \param picNum_1 : uint8_t as num of picture in second animation.
 * \param dst : CEV_AniMini* to be parametered.
 *
 * \return int of standard function status.
 *
 * note : Caculation are performed, thus direct attribution of values
 * into structure may fail.
 * Should be called AFTER texture has been set.
 */
int CEV_aniMiniParamSet(uint8_t picNum_0, uint8_t picNum_1, CEV_AniMini* dst);


/** \brief New alloc'd instance for this animation.
 *
 * \param src : CEV_AniMini* to build instance from.
 *
 * \return : CEV_SpriteMini* accordingly filled or NULL on failure.
 */
CEV_SpriteMini* CEV_aniMiniCreateAllocFrom(CEV_AniMini* src);


/** \brief New instance for this animation.
 *
 * \param src : CEV_AniMini* to build instance from.
 *
 * \return : CEV_SpriteMini accordingly filled.
 */
CEV_SpriteMini CEV_spriteMiniFrom(CEV_AniMini* src);


/** \brief Clears / reset structure content.
 *
 * \param src : CEV_SpriteMini* to be cleared.
 *
 * \return void
 */
void CEV_spriteMiniClear(CEV_SpriteMini* src);


/** \brief Updates structure status.
 *
 * \param src : CEV_SpriteMini* to be updated.
 * \param now : uint32_t as reference time (ms).
 *
 * \return SDL_Rect as actual clip for animation.
 *
 */
SDL_Rect CEV_spriteMiniUpdate(CEV_SpriteMini* src, uint32_t now);


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

int CEV_totoConvertTxtToData(char* srcName, char* dstName);*/

#endif // CEV_SHORTANIM_H_INCLUDED
