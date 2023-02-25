//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2023    |   1.0    |    creation    **/
//**********************************************************/

#ifndef CEV_CHRONO_H_INCLUDED
#define CEV_CHRONO_H_INCLUDED

#include <stdint.h>

typedef struct CEV_Chrono
{
    uint32_t timeStart,     /**< as timeAbs when starting */
             timeElapsed,   /**< as effective elapsed time without pause */
             timePause;     /**< abs timestamp when pause requested */
}
CEV_Chrono;


void TEST_cevChrono(void);


/** \brief Sets any chrono for global access.
 *
 * \param in : CEV_Chrono* selected for global access.
 *
 * \return CEV_Chrono* which is recorded as global.
 *
 * \note return recorde if in is NULL.
 */
CEV_Chrono* CEV_chronoSet(CEV_Chrono *in);


/** \brief Fetches chrono recorded for global access.
 *
 * \param void
 *
 * \return CEV_Chrono* which is recorded as global.
 */
CEV_Chrono* CEV_chronoGet(void);


/** \brief Gets relative ticks.
 *
 * \param void
 *
 * \return uint32_t as elapsed time.
 */
uint32_t* CEV_chronoTicks(void);


/** \brief Starts / resets chrono.
 *
 * \param in : CEV_Chrono* to start.
 *
 * \return void
 */
void CEV_chronoStart(CEV_Chrono *in);


/** \brief Updates / runs chrono.
 *
 * \param in  :  CEV_Chrono* to update.
 *
 * \return uint32_t as elapsed time.
 */
uint32_t CEV_chronoUpdate(CEV_Chrono *in);


/** \brief Pauses chrono.
 *
 * \param in : CEV_Chrono* to pause.
 *
 * \return void
 */
void CEV_chronoPause(CEV_Chrono *in);


/** \brief Unpauses chrono.
 *
 * \param in : CEV_Chrono* to unpause.
 *
 * \return void
 */
void CEV_chronoUnpause(CEV_Chrono *in);

#endif // CEV_CHRONO_H_INCLUDED
