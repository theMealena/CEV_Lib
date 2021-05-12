//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   2.0    |    SDL2 rev    **/
//**********************************************************/

#ifndef CEV_PAD_H_INCLUDED

#define CEV_PAD_H_INCLUDED

#include <stdbool.h>

#define PAD_ANA_MAX_VAL 0x7FFF  //Max analog Value
#define PAD_ANA_DEADBOUND 1000  /*plage morte par défaut*/
#define PAD_NB_ZONE 3           /*nombre de zone du stick ana*/
#define PAD_X_AXIS 0            /*index de l'axe x*/
#define PAD_Y_AXIS 1            /*index de l'axe y*/
#define PAD_X_ROT  3            /*rotation x*/
#define PAD_Y_ROT  4            /*rotation y*/
#define PAD_Z_AXIS_L 2            /*axe Z left*/
#define PAD_Z_AXIS_R 5
#define PAD_BT_NUM 20
#define PAD_AXIS_NUM 10

#ifndef PI
    #define PI (3.14159265)
#endif // PI

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CEV_Pad
{/**structure pad**/

    bool    isPlugged,          //pad is plugged, will allow to manage warm connect/disconnect
            button[PAD_BT_NUM]; //buttons table

    unsigned char   btNum,          //number of button available
                    axesNum,        //number of analog axes available
                    hat;            //hat value

    int16_t axisCalibrating[PAD_AXIS_NUM][2],/* TODO (drx#1#): à voir pour finition ? */
            axisValue[PAD_AXIS_NUM],        //axis values
            analogDeadRange;                //axis deadrange

    SDL_Haptic   *haptic; //its haptic if available
    SDL_JoystickID  id;   //its id ..?
    SDL_Joystick  *joy;   //itself
}
CEV_Pad;


CEV_Pad *CEV_padInit(int padNum);
/**creates pad table and init all pads
- return a CEV_Pad pointer
- returns NULL on error
*/

char CEV_padInitThis(CEV_Pad* pad);
/**initialize a single pad
- return 1 if haptic detected
- return 0 otherwise
*/

void CEV_padFreeAll();

void CEV_padFreeThis(CEV_Pad *pad);

float CEV_padAngle(Sint16 axe1, Sint16 axe2);

void CEV_padCleanList(CEV_Pad* pad, unsigned char num);

int CEV_padDivide(const CEV_Pad *pad);

void CEV_padClear(CEV_Pad* pad);

void CEV_padDiag(const CEV_Pad *pad);

#ifdef __cplusplus
}
#endif

#endif
