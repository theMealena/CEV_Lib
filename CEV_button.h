//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2022      |   0.9    |    creation    **/
//**********************************************************/
//CEV 2024/01/07 display position uses camera insted of SDL_Rect

#ifndef CEV_BUTTON_H_INCLUDED
#define CEV_BUTTON_H_INCLUDED

#include <SDL.h>
#include <CEV_api.h>
#include <CEV_timer.h>
#include <CEV_types.h>
#include <CEV_aniMini.h>
#include <CEV_camera.h>


#define SWITCHBTN_NUMOF_CMD 3   /**< 3 types of command, num of texts here under */

//literal enum for file conversion
#define SWITCHBTN_CMD_MODES {"SWITCHBTN_CMD_NUMERIC", "SWITCHBTN_CMD_LOGIC", "SWITCHBTN_CMD_ALL"}

//button id mask
#define SWITCHBTN_TYPE_ID 0x12030000
//macro to identify button from id
#define IS_SWITCHBTN(x) ((x & 0xFFFF0000) == SWITCHBTN_TYPE_ID)

enum
{
    SWITCHBTN_CMD_NUMERIC = 0x01,    /**< button is numerical command only */
    SWITCHBTN_CMD_LOGIC   = 0x02,    /**< button is logical command only */
    SWITCHBTN_CMD_ALL	  = 0x03     /**< button is both */
};

/*is part of game objects
file.obj

id as 0xTTOOIIII

TT = type of file = IS_OBJ (0x12 - 18)
OO = type of object = 03
IIII = id for this game object
*/

/* FILE FORMAT
u32 this buton id
u32 piloted object id
u32 animation id
u32 its num value
u32 its mode
u32 x pos in world
u32 y pos in world
u32 x,y,w,h hitbox
u8 is reversible
CEV_AniMini if animId <> 0 (not encapsuled)
*/


// TODO (drx#1#): ajouter l'accessMode dans le fichier.

/*
la int value est transmise à chaque changement de front
la bool value vaut l'état de l'interrupteur si reversible
sinon bool reste vrai une fois activé.
*/

/** \brief CEV_SwitchButton structure defnition
 */
typedef struct S_CEV_SwitchButton
{
    uint32_t id,            /**< this object id */
             ctrlId,        /**< id of controlled object */
             animId,        /**< animation resource id */
             chunkId,       /**< sound id */
             accessMode;    /**< access mode to dst (numerical / logic)*/

    int value,              /**< switch own value */
        *nDst;              /**< link to object's numerical command value */

    bool *bDst,             /**< link to object's bool command */
          isActivated,      /**< switch is activated (local) */
          isReversible;     /**< can be turned off (bool only) */

    CEV_Timer aniCtrl;      /**< timer to take control of animation localy */
    CEV_AniMini *anim;      /**< its animation constant */
    CEV_SpriteMini sprite;  /**< its mini sprite */

    CEV_ICoord pos;         /**< position in world top left / z used for plan pos bckgrd(0), foregrnd(n) */
    SDL_Rect blit,          /**< blit Pos */
             hitBox;        /**< hitbox relative to sprite's clip */

    CEV_Edge reActive;      /**< re edge for single commmand send */
    CEV_Chunk* sound;       /**< Sound emitted when activated */
}
CEV_SwitchButton;


//dumps content
/** \brief Dumps structure content into stdout.
 *
 * \param in  : CEV_SwitchButton* to dump.
 *
 * \return void.
 */
void CEV_switchButtonDump(CEV_SwitchButton* in);


/** \brief Creates new allocated instance.
 *
 * \param void
 *
 * \return CEV_SwitchButton* on success, NULL on error.
 */
CEV_SwitchButton* CEV_switchButtonCreate(void);


/** \brief Frees content and itself.
 *
 * \param in CEV_SwitchButton* to destroy.
 *
 * \return void
 */
void CEV_switchButtonDestroy(CEV_SwitchButton* in);//NULL compliant


/** \brief Frees / sets to nul structure content.
 *
 * \param in : CEV_SwitchButton* to clear.
 *
 * \return void
 */
void CEV_switchButtonClear(CEV_SwitchButton* in);



/** \brief Attaches bool value to button.
 *
 * \param src : bool* to be attached.
 * \param dst : CEV_SwitchButton* to attach to.
 *
 * \return void.
 */
void CEV_switchButtonAttachBool(bool* src, CEV_SwitchButton* dst);


/** \brief Attaches value to button.
 *
 * \param src : int* to be attached.
 * \param dst : CEV_SwitchButton* to attach to.
 *
 * \return void.
 */
void CEV_switchButtonAttachValue(int* src, CEV_SwitchButton* dst);


/** \brief Sets aniMini to be used.
 *
 * \param src : CEV_AniMini* to be used as Animini.
 * \param dst : CEV_SwitchButton* to link aniMini to.
 *
 * \return void.
 *
 * \note content is unchanged on arg error.
 */
void CEV_switchButtonAttachAnim(CEV_AniMini* src, CEV_SwitchButton* dst);


/** \brief Loads structure from file.
 *
 * \param fileName : char* as name of file to load.
 *
 * \return CEV_SwitchButton* on success, NULL on error.
 */
CEV_SwitchButton* CEV_switchButtonLoad(char* fileName);


/** \brief Loads structure from RWops virtual file.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool closes RWops if true / keep opened otherwise.
 *
 * \return CEV_SwitchButton* on success, NULL on error.
 *
 * \note freeSrc is applied weither function succeed or not.
 */
CEV_SwitchButton* CEV_switchButtonLoad_RW(SDL_RWops* src, bool freeSrc);//freed weither succeeds or fails


/** \brief Saves structure into file.
 *
 * \param src : CEV_SwitchButton* to be saved.
 * \param fileName : char* as name of file.
 *
 * \return int of std CEV function status.
 */
int CEV_switchButtonSave(CEV_SwitchButton* src, char* fileName);


/** \brief Writes structure into opened file.
 *
 * \param src : CEV_SwitchButton* to be written.
 * \param dst : FILE* to write into.
 *
 * \return int of std CEV function status.
 */
int CEV_switchButtonTypeWrite(CEV_SwitchButton* src, FILE* dst);


/** \brief Reads structure from opened RWops virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_SwitchButton* to be filled with vFile content.
 * \param freeSrc : bool closes RWops if true / keep opened otherwise.
 *
 * \return int of std CEV function status.
 *
 * \note freeSrc is applied weither function succeed or not.
 */
int CEV_switchButtonTypeRead_RW(SDL_RWops* src, CEV_SwitchButton* dst, bool freeSrc);


/** \brief Complete updates structure's instance (sts + display).
 *
 * \param this : CEV_SwitchButton* to update.
 * \param camera : CEV_Camera* as camera position in world.
 *
 * \return void.
 *
 * \note Updates content / animates & displays only if on camera.
 */
void CEV_switchButtonUpdate(CEV_SwitchButton* this, CEV_Camera* camera);


/** \brief Updates status & anim / sends command.
 *
 * \param this : CEV_SwitchButton* to move.
 *
 * \return void.
 */
void CEV_switchButtonMove(CEV_SwitchButton* this);


/** \brief Displays button on screen.
 *
 * \param this : CEV_SwitchButton* to display.
 * \param camera : CEV_Camera* as camera position in world.
 *
 * \return void.
 */
void CEV_switchButtonDisplay(CEV_SwitchButton* this, CEV_Camera* camera);



/** \brief Insert switch button into CEV_Capsule
 *
 * \param src : CEV_SwitchButton* as source to encapsule.
 * \param dst : CEV_Capsule* as destination to write into.
 *
 * \return int of std return values.
 *
 * \note UNUSED.
 */
int CEV_switchButtonToCapsule(CEV_SwitchButton* src, CEV_Capsule *dst);


/** \brief Convert definition text file into button typed file.
 *
 * \param srcName : const char* as path and file name to read from.
 * \param dstName : const char* as path and file name of resulting file.
 *
 * \return int of std return values.
 *
 * \note Resulting file extension should be ".obj" to enable recognition upon auto-loading.
 */
int CEV_switchButtonConvertToData(const char* srcName, const char* dstName);


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
int CEV_switchButtonConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName);


#endif // CEV_BUTTON_H_INCLUDED
