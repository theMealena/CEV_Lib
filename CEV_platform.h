//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  11-2022      |   0.9    |    creation    **/
//**   CEV    |  02-2023      |   1.0    |  validation    **/
//**********************************************************/



/// USES EMBEDDED RESSOURCES ONLY UNTIL REFERENCEMENT IS IMPLEMENTED
/// AMASK UNUSED UNTIL IMPLEMENTED

#ifndef PLATEFORMS_H_INCLUDED
#define PLATEFORMS_H_INCLUDED

#include <stdbool.h>
#include <SDL.h>
#include <CEV_timer.h>
#include <CEV_aniMini.h>

#define CEV_PLATFORM_MAX_POS 5 //max num of platforms' position

#define PLTFRM_TYPE_ID 0x12010000   //object identifier for constantes
#define IS_PLTFRM(x) ((x & 0xFFFF0000) == PLTFRM_TYPE_ID)

#define PTFR_USES_CST 0

/*is part of game objects
file.obj

ID as 0xTTOOIIII

TT = type of file = IS_OBJ (0x12 - 18)
OO = type of object (0x01 - 1)
IIII = ID for this game object
*/


//type of object = 0x02
/* platefrom instance file

    u32le   : Unique ID                     0x00
            : constants' Unique ID          0x04
            : timeset as full travel time   0x08
            : timeSync as time offset to 0  0x0C
            : timePause                     0x10
            : num of floors then            0x14
                for 0 -> num of pos
                    u32le : position X
                    u32le : position Y
    u8      : is elevator
*/



//pause is at end of movement, request is incremented when pause is done
/** \brief Plateform single instance.
 */
typedef struct S_CEV_Platform
{
    //parameters / controls
    uint32_t
            id,         /**< unique id. */
            srcId,      /**< src aniMini id. */
            timeSet,    /**< time to travel full cycle. */
            timeSync,   /**< syncro as offset to 0 ms. */
            timePause,  /**< time to pause when destination is reached. */
            numOfFloors;/**< number of positions available. */

    bool isElevator;    /**< platform is elevator. */

    CEV_ICoord floorPos[CEV_PLATFORM_MAX_POS], /**< floors positions top left of sprite.*/
                actPos,  /**< actual position in world / Top Left of blit / z is background(0) to foreground(x) (local).*/
                vect;    /**< actual velocity vector (local).*/

    //status / locals (not be reached from outside)
    bool isAtPos;       /**< destination is reached (local).*/

    uint32_t
            timeStart,                      /**< begin of sync move on segment (local).*/
            timeStop,                       /**< end of sync move on segment (local).*/
            timeRef,                        /**< memo to detect modulo back to 0 (local).*/
            distFull,                       /**< full distance travel (from pt to pt) (local).*/
            posSync[CEV_PLATFORM_MAX_POS]; /**< time/pos sync values (local).*/

    int posIndexNxt,    /**< position index requested (local).*/
        posIndexAct,    /**< memo to detect changes in request (local).*/
        posIndexReq;    /**< requested index (local or switch button cmd).*/

    SDL_Rect hitbox;    /**< hitbox relative to clip */

    //resources
    CEV_AniMini* anim;  /**< to its constants. */

    //SP_Sprite* sprite;      /**< its sprite if is sprite */
    CEV_SpriteMini sprite;  /**< own mini sprite instance. */
    CEV_Edge  posReached;   /**< reached position RE detection (local).*/
}
CEV_Platform;



/** \brief Platform module tests.
 *
 * \param void.
 *
 * \return void.
 */
void TEST_platform(void);


    /// USER END FUNCTIONS


    /// platform instance functions

/** \brief Dumps platform content.
 *
 * \param this : CEV_Platform* to dump.
 *
 * \return void.
 *
 * \note is dumped into stdout.
 */
void CEV_platformDump(CEV_Platform* this);


/** \brief Updates platform status and displays it.
 *
 * \param this : CEV_Platform* to update.
 * \param now : uint32_t as elapsed time.
 * \param camera : SDL_Rect as camera position in world.
 *
 * \return void
 */
void CEV_platformUpdate(CEV_Platform* this, SDL_Rect camera, uint32_t now);


/** \brief Calculates platform position.
 *
 * \param this : CEV_Platform* to move.
 * \param now : uint32_t as elapsed time.
 *
 * \return void
 */
void CEV_platformMove(CEV_Platform* this, uint32_t now);


/** \brief Displays platform onto active renderer.
 *
 * \param this : CEV_Platform* to display.
 * \param cameraPos : SDL_Rect as camera position in world.
 * \param now : uint32_t as elapsed time.
 *
 * \return void
 */
void CEV_platformDisplay(CEV_Platform* this, SDL_Rect cameraPos, uint32_t now);



/** \brief Attaches animation to platform.
 *
 * \param src : CEV_AniMini* to be attached.
 * \param dst : CEV_Platform* to attach to.
 *
 * \return int of std function status.
 */
int CEV_platformAnimAttach(CEV_AniMini* src, CEV_Platform* dst);


/** \brief Performs precalculation according to parameters.
 *
 * \param this : CEV_Platform* to perform calculation on.
 *
 * \return void.
 *
 * \note called on loading from file, call manualy otherwise.
 */
void CEV_platformPrecalc(CEV_Platform* this);


/** \brief Plateform destination reached.
 *
 * \param this : CEV_Platform* to query.
 *
 * \return bool : true if position is reached.
 *
 * \note is true while platform is at its pos & not moving.
 */
bool CEV_platformIsAtPos(CEV_Platform* this);


/** \brief Plateform hitBox position in world.
 *
 * \param this : CEV_Platform* to query.
 *
 * \return SDL_Rect as hitbox position in world
 */
SDL_Rect CEV_platformHitBoxGet(CEV_Platform* this);


/** \brief Frees platform content and itself.
 *
 * \param this : CEV_Platform* to free.
 *
 * \return void
 */
void CEV_platformDestroy(CEV_Platform *this);


/** \brief Clears platform content.
 *
 * \param this : CEV_Platform* to clear.
 *
 * \return void
 */
void CEV_platformClear(CEV_Platform *this);


    /// FILES RELATED FUNCTIONS

    /// Instance structure


/** \brief Saves platform into file.
 *
 * \param src : CEV_Platform* to save.
 * \param fileName : char* as resulting file name.
 *
 * \return int of sdt function satus.
 */
int CEV_platformSave(CEV_Platform* src, char* fileName);


/** \brief Write platform content into file.
 *
 * \param src : CEV_Platform* to be written.
 * \param dst : FILE* to write into.
 *
 * \return int of standard function status.
 */
int CEV_platformTypeWrite(CEV_Platform* src, FILE* dst);


/** \brief Loads platform from file.
 *
 * \param fileName : char* as name of file to load.
 *
 * \return CEV_Platform* on success, NULL on failure.
 */
CEV_Platform* CEV_platformLoad(char* fileName);


/** \brief Loads platform from RWops
 *
 * \param src : SDL_RWops* to read from.
 * \param freeSrc : closes src RWops if true.
 *
 * \return CEV_Platform* on success, NULL on failure.
 *
 * \note if requested, SDL_RWpos is closed weither function succeeds or not.
 */
CEV_Platform* CEV_platformLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Reads platform from RWops
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_Platform* to fill with read data.
 * \param freeSrc : closes src if true.
 *
 * \return int of standard function status.
 *
 * \note src is closed upon request weither the function succeeds or not.
 */
int CEV_platformTypeRead_RW(SDL_RWops* src, CEV_Platform* dst, bool freeSrc);


/** \brief Converts editable text file into data file.
 *
 * \param srcName : const char* as path and file name of source.
 * \param dstName : const char* as path and file name of destination.
 *
 * \return int of standard function status.
 */
int CEV_platformConvertTxtToData(const char* srcName, const char* dstName);

#endif // PLATEFORMS_H_INCLUDED
