//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |  rev & tested  **/
//**********************************************************/

#ifndef CEV_TIMER_H_INCLUDED
#define CEV_TIMER_H_INCLUDED

#include <stdint.h>
#include "CEV_api.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  \brief Timer instance.
 */
typedef struct CEV_Timer
{
	uint8_t     run,        /*run order*/
                processing, /*counting down*/
                cmd;        /*command*/

    int64_t     accu;   /*accumulator*/

	uint32_t     preset,/*preset value*/
                 lst;   /*previous absolute time*/

    CEV_Edge    edge;   /*edge*/
}
CEV_Timer;



/**
 *  \brief Initialize timer.
 *  \param Timer to set as ptr.
 *  \param Time time in ms.
 *  \return N/A.
 */
void CEV_timerInit(CEV_Timer* in, unsigned int val);


/**
 *  \brief TON timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true, 0 otherwise
 *  note : cmd = run && preset ms elapsed
 */
char CEV_timerTon(CEV_Timer *in);


/**
 *  \brief TOF timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : cmd = run || (!run && processing)
 */
char CEV_timerTof(CEV_Timer *in);


/**
 *  \brief Delay On timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : RE cmd = RE run + preset ms elapsed
 */
char CEV_timerDelayOn(CEV_Timer* in);


/**
 *  \brief Pulse timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : cmd = RE run || processing
 */
char CEV_timerPulse(CEV_Timer* in);


/**
 *  \brief repeat timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : RE cmd = RE run || (run && preset elapsed)
 *  note : runs like autofire loop
 */
char CEV_timerRepeat(CEV_Timer* in);


/**
 *  \brief locked repeat timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : RE cmd = run && !processing
 */
char CEV_timerRepeatLocked(CEV_Timer* in);


/**
 *  \brief blinking timer.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : cmd = square signal of period = preset while run
 */
char CEV_timerEverBlink(CEV_Timer *in);


/**
 *  \brief blinking timer limited.
 *  \param Timer to control as ptr.
 *  \return 1 When cmd is true 0 otherwise
 *  note : cmd = RE run && square signal of period 200 while processing
 *  note : can repeat while run
 */
char CEV_timerBlinkLimited(CEV_Timer *in);
/*clignote pendant preset ms sur RE run*/


/**
 *  \brief reset timer.
 *  \param Timer to control as ptr.
 *  \return N/A
 */
void CEV_timerReset(CEV_Timer *in);

#ifdef __cplusplus
}
#endif

#endif // CEV_TIMER_H_INCLUDED
