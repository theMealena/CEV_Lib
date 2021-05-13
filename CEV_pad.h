//**********************************************************/
//** Done by  |      Date     |  version |    comment      */
//**-------------------------------------------------------*/
//**   CEV    |  14-02-2015   |   0.0    |    creation     */
//**   CEV    |    05-2016    |   2.0    |    SDL2 rev     */
//**   CEV    |    05-2021    |   2.1    |doxy + new func  */
//**********************************************************/



#ifndef CEV_PAD_H_INCLUDED
#define CEV_PAD_H_INCLUDED

#include <stdbool.h>

#define PAD_ANA_MAX_VAL 0x7FFF  /**< Max analog Value (32767) */
#define PAD_ANA_MIN_VAL 0x8000  /**< Min analog Value (-32768) */
#define PAD_ANA_DEADBOUND 0x400 /**< Default dead bound */
#define PAD_ZONE_NUM 3          /**< Default zone num */
#define PAD_X_AXIS 0            /**< X axis index */
#define PAD_Y_AXIS 1            /**< Y axis index */
#define PAD_X_ROT  3            /**< X rotation axis index */
#define PAD_Y_ROT  4            /**< Y rotation axis index */
#define PAD_Z_AXIS_L 2          /**< Z axis index (left trigger) */
#define PAD_Z_AXIS_R 5          /**< Z axis index (right trigger) */
#define PAD_BT_NUM 20           /**< Default pad num of button */
#define PAD_AXIS_NUM 10         /**< Defulat pad num of axis */

#ifndef PI
    #define PI (3.14159265)
#endif // PI

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Analog axis definition.
 */
typedef struct CEV_PadAxis
{//added 20210513

    int16_t calib[2],   /**< Calibrating values min/max */ /* TODO (drx#1#): à implémenter pour finition ? */
            rawValue,   /**< Raw value given by event */
            value,      /**< Scaled value with calib / dead range correction */
            zone;       /**< Zoned value reduction */
}
CEV_PadAxis;



/** \brief Joystick / Pad definition.
 */
typedef struct CEV_Pad
{
    bool    isPlugged,          /**< Pad is plugged, will allow to manage warm connect/disconnect. */
            button[PAD_BT_NUM]; /**< Buttons array. */

    unsigned char   btNum,      /**< Number of button available. */
                    axesNum,    /**< Number of analog axes available. */
                    hat;        /**< Hat value. */

    CEV_PadAxis axis[PAD_AXIS_NUM]; /**< Axis */

    uint16_t analogDeadRange,   /**< Axis dead range. */
             numOfZone;         /**< Zone divider. */


    SDL_Haptic   *haptic; /**< Its haptic instance if available. */
    SDL_JoystickID  id;   /**< Its id. */
    SDL_Joystick  *joy;   /**< Its SDL Instance */
}
CEV_Pad;


/** \brief Creates pads array and init all pads.
 *
 * \param padNum : size of array to allocate.
 *
 * \return CEV_Pad pointer on success / NULL on error.
 */
CEV_Pad* CEV_padCreate(int padNum);


/** \brief Updates pad values / status.
 *
 * \param pad : ptr to pad to be updated.
 *
 * \return N/A.
 */
void CEV_padUpdate(CEV_Pad *pad);


/** \brief Sets pad number of zone.
 *
 * \param numOfZone : how many times to divide analog stick full range.
 * \param which : Pad index to modify, -1 for all pads.
 *
 * \return true if value was applied.
 */
bool CEV_padZoneSet(unsigned numOfZone, int which);

/** \brief Initialize a single pad structure.
 *
 * \param pad : ptr to the Pad to init.
 *
 * \return true if haptic is detected, false otherwise.
 */
bool CEV_padInitThis(CEV_Pad* pad);


/** \brief Resets/NULL/0 a single pad structure.
 *
 * \param pad : ptr to the Pad to clear.
 *
 * \return N/A.
 */
void CEV_padClear(CEV_Pad* pad);


/** \brief Resets/defaults value of analog axis.
 *
 * \param pad : Pad from which to reset analog axis values.
 *
 * \return N/A.
 */
void CEV_padAxisInit(CEV_Pad* pad);


/** \brief Closes / frees all registered pads.
 *
 * \return N/A.
 */
void CEV_padFreeAll(void);


/** \brief Closes / frees single pad.
 *
 * \param pad : ptr to the Pad to free/close.
 *
 * \return N/A.
 */
void CEV_padFreeThis(CEV_Pad *pad);


/** \brief Angle of analog stick.
 *
 * \param axisX : value to be considered as X axis.
 * \param axisY : value to be considered as Y axis.
 *
 * \return Angle as float.
 */
float CEV_padAngle(Sint16 axisX, Sint16 axisY);


/** \brief Analog stick zone divider as circular.
 * \detail analog stick is divided by pad's num of zone.
 *
 * \param pad : Pad to divide value.
 *
 * \return value of range.
 */
int CEV_padCircularZone(const CEV_Pad *pad);


/** \brief Dumps Pad status.
 * \detail Result is dumped to stdout.
 *
 * \param pad : Pad to display status from.
 *
 * \return N/A.
 */
void CEV_padDump(const CEV_Pad *pad);


/** \brief Adds warm plugged joystick.
 *
 * \param index : index of appearing pad
 *
 * \return N/A.
 */
void CEV_padAddWarm(int index);


/** \brief Removes warm unplugged pad from array.
 *
 * \param id : id of disappearing pad
 *
 * \return N/A.
 */
void CEV_padRemoveWarm(int id);


/** \brief Rearange pad array sorting by ID.
 *
 * \param pad : ptr to Pad array.
 * \param num : num of pad un array.
 *
 * \return N/A.
 */
void CEV_padSortByID(CEV_Pad * pad, int num);

#ifdef __cplusplus
}
#endif

#endif
