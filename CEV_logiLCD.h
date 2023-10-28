#ifndef REV_LOGILCD_H_INCLUDED
#define REV_LOGILCD_H_INCLUDED

#include <stdbool.h>
#include <SDL.h>

#define LCD_SPECS_NUM 6
#define LCD_SPECS_NAMES {L"Viper Mark II", L"Viper Mark VII", L"X-Wing", L"Y-Wing", L"Gryphon", L"Valkyrie"}
#define LCD_GAME 1

#ifndef DATA_FILE
 #define DATA_FILE "data/data.dat"
#endif


/** \brief lcd system structure
 */
typedef struct REV_SystemLcd
{
    bool isInit;            /**< has LCD */

    unsigned int lcdType;   /**< COLOR / BW */

    SDL_Surface *srcBckgd,  /**< neat background */
                *result;    /**< display result */
}
REV_SystemLcd;


/** \brief init LCD system
 *
 * \param sys : REV_SystemLcd* to init.
 *
 * \return bool : true if any compatible keyboard found.
 */
bool REV_systemLcdInit(REV_SystemLcd *sys);


/** \brief closes LCD lib
 *
 * \return void.
 */
void REV_systemLcdClose(void);


/** \brief fetches LCD system
 *
 * \return REV_SystemLcd* that was recorded.
 */
REV_SystemLcd* REV_systemLcdGet(void);


/** \brief set LCD background
 *
 * \param index : int in resource file.
 *
 * \return void.
 */
void REV_lcdBackgroundSet(int index);


void REV_lcdTitleSet(const char *src);

/** \brief updates LCD display.
 *
 * \return void.
 */
void REV_lcdShow(void);


void REV_lcdLinesClear(void);





#endif // REV_LOGILCD_H_INCLUDED
