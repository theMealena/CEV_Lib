//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   1.0    |    SDL2 rev    **/
//**   CEV    |    11-2017    |   1.0.1  |  diag improved **/
//**********************************************************/


#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "errno.h"
#include "string.h"
#include <SDL.h>
#include "project_def.h"
#include "CEV_pad.h"
#include "CEV_input.h"



CEV_Pad *CEV_padInit(int padNum)
{/**crée une table de pointeurs sur les pads*/

    /*****DECLARATION*****/

    CEV_Pad* padTab = NULL;              /*result*/

    /*****EXECUTION*****/

    if (padNum)/*if any pad*/
    {
        padTab = calloc(padNum,  sizeof(*padTab));

        if (!padTab)
            fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));

        else
        {
            for (int i=0; i<padNum; i++)
            {
                padTab[i].joy   = SDL_JoystickOpen(i);
                CEV_padInitThis(&padTab[i]);
            }
        }
    }

    return (padTab);
}


/**single pad init*/
char CEV_padInitThis(CEV_Pad* pad)
{
    pad->isPlugged = (pad->joy != NULL);

    if (pad->isPlugged)
    {
        pad->analogDeadRange    = PAD_ANA_DEADBOUND;
        pad->btNum              = SDL_JoystickNumButtons(pad->joy);
        pad->axesNum            = SDL_JoystickNumAxes(pad->joy);
        pad->hat                = SDL_HAT_CENTERED;

        pad->id = SDL_JoystickInstanceID(pad->joy);/*get id*/

        if(SDL_JoystickIsHaptic(pad->joy)>0) /*haptic ?*/
        {
            pad->haptic = SDL_HapticOpenFromJoystick(pad->joy);
        }
        else
        {
            pad->haptic = NULL;
        }
    }

#if VERBOSE
    CEV_padDiag(pad);
#endif // VERB

    return (pad->haptic != 0);
}


void CEV_padClear(CEV_Pad* pad)
{/**everything to 0 / NULL*/

    pad->analogDeadRange = 0;
    pad->axesNum         = 0;
    pad->btNum           = 0;
    pad->haptic          = NULL;
    pad->hat             = SDL_HAT_CENTERED;
    pad->id              = -1;
    pad->isPlugged       = false;
    pad->joy             = NULL;


    for(int i =0; i<PAD_BT_NUM; i++)
        pad->button[i] = false;

    for(int i =0; i<PAD_AXIS_NUM; i++)
    {
        pad->axisValue[i] = 0;

        for(int j=0; j<2; j++)
            pad->axisCalibrating[i][j] = 0;
    }
}


void CEV_padFreeAll()
{/**frees all opened pad*/

    CEV_Input *input = CEV_inputGet();

    for (int i=0; i<input->padNum; i++)
        CEV_padFreeThis(&input->pad[i]);

	free(input->pad);
}


void CEV_padFreeThis(CEV_Pad *pad)
{/*frees one pad*/

    if (pad)
    {
        if(pad->haptic != NULL)
            SDL_HapticClose(pad->haptic);

        if((pad->joy!=NULL) && SDL_JoystickGetAttached(pad->joy))
            SDL_JoystickClose(pad->joy);
    }
}


float CEV_padAngle(Sint16 axe1, Sint16 axe2)
{/**défini l'angle du stick ana, nécessite math.h*/
/**la valeur de retour est exploitable par le rotozoom*/

    float val =0;/*resultat de calcul*/

    val = (180*(atan((float)axe2/(float)axe1))/PI);

    if (axe1<0)
        return(180.0 + val);
    else if (axe2<0)
        return(360.0 + val);
    else
        return(val);
}


int CEV_padDivide(const CEV_Pad *pad)
{/**filtre le stick ana du pad et divise en plages*/

    /*****DECLARATIONS*****/

    int value = 0;
    CEV_FCoord pos,
            padPos;

    Sint16 plage =(PAD_ANA_MAX_VAL - pad->analogDeadRange)/PAD_NB_ZONE;/*partage de la zone active en PAD_NB_ZONE plages*/

    /*****PRELIMINAIRE*****/

    pos.x       = pos.y  =0;
    padPos.x    = pad->axisValue[PAD_X_AXIS];
    padPos.y    = pad->axisValue[PAD_Y_AXIS];

    /*****EXECUTION*****/

    value = CEV_fcoordDist(pos, padPos);

    /*****POST*****/

    return(value/plage);
}



void CEV_padDiag(const CEV_Pad *pad)
{
    printf(" Pad name : %s\n", SDL_JoystickName(pad->joy));
    printf(" Pad id : %d \n", pad->id);
    printf(" Pad is haptic : %s\n", (pad->haptic == NULL)? "no" : "yes");
    printf(" Pad button : %d\n",pad->btNum);
    printf(" Pad axis : %d\n",pad->axesNum);
}
