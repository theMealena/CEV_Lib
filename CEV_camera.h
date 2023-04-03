//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  2022/03/06   |   1.0    | rev & test / added to CEV_lib
//**********************************************************/

#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <SDL.h>

#include "CEV_timer.h"
#include "CEV_api.h"



/*moving direction*/
#define CAMERA_LEFT     0x1 //0001
#define CAMERA_RIGHT    0x2 //0010
#define CAMERA_UP       0x4 //0100
#define CAMERA_DOWN     0x8 //1000
#define CAMERA_ALL      0xF
#define CAMERA_NONE     0x0
#define CAMERA_NEGATIVE      0x1 //going right, up in world
#define CAMERA_POSITIVE      0x2 //going left, down in world

/*open field is layed out in front of target point to follow
- is automatic by default, can be disabled with autoReverse = false.
open field will be kept as is until call to CEV_cameraOpenFieldSet() is called.

- direction lock is shifted 2 bytes for Y axis
*/



/** \brief Camera mode behaviour
 *
 * note : enable to show more or less field of view before point to follow.
 */
typedef enum S_CameraMode
{
    CAMERA_MED = 2,     /**< follow point is kept centered */
    CAMERA_THIRD = 3,   /**< follow point is kept at 1/3 */
    CAMERA_QUART = 4    /**< follow point is kept at 1/4 */
}
CEV_CameraMode;


typedef struct S_CameraParam
{
    int velMax,         /**< max pxl/frame vel */
        dirAutoScroll,  /**< autoScroll direction */
        dirAct,         /**< active direction */
        dirCalc,        /**< recalculated camera direction */
        dirLock,        /**< direction lock */
        camDim,         /**< display (render) dimension */
        posCalc,        /**< calculated camera position */
        *posAct,        /**< camera real position  */
        autoScrollVel; /**< autoscroll pxl/frame vel*/

    bool autoReverse;   /**< enables direction change */

    float *followThis,  /**< follow this position */
           followPrev;  /**< where was I before ? */

    CEV_CameraMode mode; /**< camera display mode */

    CEV_Timer timer;/**< direction change TON*/
}
CEV_CameraParam;


typedef struct S_Camera
{
    CEV_CameraParam param[2]; /**< parameters x, y */

    SDL_Rect scrollActPos, /**< camera position in world pxl */
             constraint; /**< world dimension pxl */
}
CEV_Camera;


/** \brief Inits new camera / sets parameters for camera.
 *
 * \param in : CEV_Camera* to init.
 * \param followPt : CEV_FCoord* to be followed by camera.
 * \param constraint : SDL_Rect in which camera should be constraint (world).
 * \param changeTime : unsigned int as delay (ms) for camera to react to direction change.
 * \param mode : CEV_CameraMode as follow mode.
 *
 * \return void
 * note : Is to use with a declared camera VS camera loaded from file.
 */
void CEV_cameraInit(CEV_Camera *in, CEV_FCoord* followPt, SDL_Rect constraint, unsigned int changeTime, CEV_CameraMode mode);


/** \brief Laods camera from file.
 *
 * \param fileName : char* as path and file name to load.
 *
 * \return CEV_Camera* on success, NULL on failure.
 *
 */
CEV_Camera *CEV_cameraLoad(char *fileName);


/** \brief Saves camera into file.
 *
 * \param src : CEV_Camera* to be saved.
 * \param fileName : const char* as filename to save as.
 *
 * \return int : any of std return value.
 */
int CEV_cameraSave(CEV_Camera *src, const char* fileName);


/** \brief Loads camera from vrtual file.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool set to true closes virtual file.
 *
 * \return CEV_Camera* as loaded from Vfile, NULL on error.
 *
 * note : src will be treated as asked wether function is successful or not.
 */
CEV_Camera *CEV_cameraLoad_RW(SDL_RWops *src, bool freeSrc);


/** \brief Automatic camera update.
 *
 * \param in : CEV_Camera* to update.
 *
 * \return int : One direction if follow point is off camera, 0 otherwise.
 *
 * note : If follow point was to be off camera at bottom, shall return CAMERA_DOWN
 */
int CEV_cameraUpdate(CEV_Camera *in);


/** \brief Brings camera back on follow point
 *
 * \param in : CEV_Camera* which camera.
 *
 * \return void
 */
void CEV_cameraReset(CEV_Camera *in);


/** \brief Sets camera Dimensions.
 *
 * \param src : CEV_Camera* to set dimension for
 * \param w : int as width dimension (pxl).
 * \param h : int as height dimension (pxl).
 *
 * \return void
 *
 * note : setting values of 0 or negative will set demension to be render dimension
 */
void CEV_cameraDimensionSet(CEV_Camera *src, int w, int h);


/** \brief Enables automatic open field.
 *
 * \param in : CEV_Camera* to set.
 * \param time : unsigned int as ms before open field changes.
 * \param axis : int which axis to modify CEV_X, CEV_Y.
 *
 * \return void
 *
 */
void CEV_cameraOpenFieldAuto(CEV_Camera *in, unsigned int time, int axis);


/** \brief Sets/forces camera open field direction.
 *
 * \param in : CEV_Camera* to set.
 * \param direction : int as new direction.
 *
 * \return void
 *
 * note : Does not override automatic direction switch.
 */
void CEV_cameraOpenFieldSet(CEV_Camera *in, int direction);


/** \brief Locks camera direction.
 *
 * \param in : CEV_Camera* to lock.
 * \param direction : int as wich direction to lock OR'd together.
 *
 * \return void
 *
 * note : Can be used to disable backtracking.
 */
void CEV_cameraDirectionLock(CEV_Camera *in, int direction);


/** \brief Sets camera maximum velocity (pxl/frame)
 *
 * \param in : CEV_Camera* to set.
 * \param velMax : int as maximum velocity that can be reached as pxl/frame.
 * \param direction : int as wich axis to apply this parameter, Or'd together.
 *
 * \return void
 *
 * note : any of CAMERA_RIGHT / CAMERA_LEFT sets horizontal maximum velocity.
 * same for UP/DOWN.
 */
void CEV_cameraMaxVelocitySet(CEV_Camera* in, int velMax, int direction);


/** \brief Set camera following mode.
 *
 * \param in : CEV_Camera* which camera to set.
 * \param mode : CEV_CameraMode following mode.
 * \param axis : int which axis to set (CEV_X, CEV_Y).
 *
 * \return void
 */
void CEV_cameraFollowModeSet(CEV_Camera *in, CEV_CameraMode mode, int axis);


/** \brief Activate/deactivate camera auto scrolling
 *
 * \param in : CEV_Camera* to parameter.
 * \param autoScroll : bool true to activate, false to deactivate.
 * \param direction : int as which direction to auto scroll.
 * \param velocity : int as auto scroll velocity in pxl/frame.
 *
 * \return void
 *
 */
void CEV_cameraScrollSet(CEV_Camera* in, bool autoScroll, int direction, int velocity);














#endif // CAMERA_H_INCLUDED
