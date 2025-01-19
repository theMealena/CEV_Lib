//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2023    |   1.0    |    creation    **/
//**   CEV    |    09-2024    |   1.0.1  | Added into CEV_system **/
//**********************************************************/

// CEV - 1.0.1 - most function rewritten without Arg and fetches last set timer.


#ifndef CEV_CHRONO_H_INCLUDED
#define CEV_CHRONO_H_INCLUDED

#include <stdint.h>

typedef struct CEV_Chrono
{
    uint32_t timeStart,     /**< as timeAbs when starting */
             timeElapsed,   /**< as effective elapsed time without pause */
             timePause;     /**< abs timestamp when pause requested */

    bool    isRunning,      /**< is active */
            isPaused;       /**< is paused */
}
CEV_Chrono;


void TEST_CEV_chrono(void);


/** \brief Sets any chrono for global access.
 *
 * \param in : CEV_Chrono* selected for global access.
 *
 * \return CEV_Chrono* which is recorded as global.
 *
 * \note returns recorded value if in is NULL.
 */
CEV_Chrono* CEV_chronoSet(CEV_Chrono *in);


/** \brief Fetches chrono recorded for global access.
 *
 * \param void.
 *
 * \return CEV_Chrono* which is recorded as global.
 */
CEV_Chrono* CEV_chronoGet(void);


/** \brief Gets relative ticks.
 *
 * \param void.
 *
 * \return uint32_t as elapsed time.
 */
uint32_t CEV_chronoTicks(void);


/** \brief Gets ptr to relative ticks.
 *
 * \param void.
 *
 * \return uint32_t* as elapsed time.
 */
uint32_t* CEV_chronoTicksPtr(void);


/** \brief Gets running status.
 *
 * \param void
 *
 * \return bool : true if chrono was started, false otherwise.
 */
bool CEV_chronoIsRunning(void);


/** \brief Gets pause status.
 *
 * \param void
 *
 * \return bool : true if chrono is pause, false otherwise.
 */
bool CEV_chronoIsPaused(void);


/** \brief Starts / resets chrono.
 *
 * \param void
 *
 * \return void.
 */
void CEV_chronoStart(void);


/** \brief Updates / runs chrono.
 *
 * \param void
 *
 * \return uint32_t as elapsed time.
 */
uint32_t CEV_chronoUpdate(void);


/** \brief Pauses chrono.
 *
 * \param void
 *
 * \return void.
 */
void CEV_chronoPause(void);


/** \brief Unpauses chrono.
 *
 * \param void
 *
 * \return void.
 */
void CEV_chronoUnpause(void);

#endif // CEV_CHRONO_H_INCLUDED
