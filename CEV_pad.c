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

/*axis value scaling added 20210513*/
int16_t L_padAxisScale(int16_t realVal, int16_t realMin, int16_t realMax, int16_t retMin, int16_t retMax);


CEV_Pad *CEV_padCreate(int padNum)
{/*Creates pad array*/

    CEV_Pad* padTab = NULL; //result

    if (padNum)//if any pad
    {
        padTab = calloc(padNum,  sizeof(*padTab));

        if (!padTab)
        {
            fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        }
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


//added 20210513
void CEV_padUpdate(CEV_Pad *pad)
{/*Updates pd values */

    if(pad)
    {
        int16_t zoneLen = PAD_ANA_MAX_VAL / pad->numOfZone;

        for(int i=0; i<PAD_AXIS_NUM; i++)
        {
            if(pad->axis[i].rawValue < 0)
                pad->axis[i].value = L_padAxisScale(pad->axis[i].rawValue,
                                                        pad->axis[i].calib[0],
                                                        -pad->analogDeadRange,
                                                        PAD_ANA_MIN_VAL,
                                                        0);
            else
                pad->axis[i].value = L_padAxisScale(pad->axis[i].rawValue,
                                                        pad->analogDeadRange,
                                                        pad->axis[i].calib[1],
                                                        0,
                                                        PAD_ANA_MAX_VAL);

            pad->axis[i].zone = pad->axis[i].value / zoneLen;
        }
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Argument is NULL.\n", __FUNCTION__, __LINE__ );
    }
}


//added 20210513
bool CEV_padZoneSet(unsigned numOfZone, int which)
{/*Zoning value setting*/

    if(numOfZone)
    {
        CEV_Input* input = CEV_inputGet();

        if(which<0)
        {
            for(int i=0; i<input->padNum; i++)
            {
                input->pad[i].numOfZone = numOfZone;
            }
            return true;
        }
        else if (which < input->padNum)
        {
            input->pad[which].numOfZone = numOfZone;
            return true;
        }
    }

    return false;
}


bool CEV_padInitThis(CEV_Pad* pad)
{/*Single pad init*/

    if(pad)
    {
        pad->isPlugged = (pad->joy != NULL);

        if (pad->isPlugged)
        {
            pad->analogDeadRange    = PAD_ANA_DEADBOUND;
            pad->btNum              = SDL_JoystickNumButtons(pad->joy);
            pad->axesNum            = SDL_JoystickNumAxes(pad->joy);
            pad->hat                = SDL_HAT_CENTERED;
            pad->numOfZone          = PAD_ZONE_NUM;

            CEV_padAxisInit(pad);

            pad->id = SDL_JoystickInstanceID(pad->joy);//gets id

            if(SDL_JoystickIsHaptic(pad->joy)>0) //is haptic ?
            {
                pad->haptic = SDL_HapticOpenFromJoystick(pad->joy);
            }
            else
            {
                pad->haptic = NULL;
            }
        }

    #if VERBOSE
        CEV_padDump(pad);
    #endif // VERB

        return (pad->haptic != 0);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Argument is NULL.\n", __FUNCTION__, __LINE__ );
    }

    return false;
}


void CEV_padClear(CEV_Pad* pad)
{/*Everything to 0 / NULL*/

    if(pad)
    {

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

        CEV_padAxisInit(pad);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Argument is NULL.\n", __FUNCTION__, __LINE__ );
    }
}


//added 20210513
void CEV_padAxisInit(CEV_Pad* pad)
{/*Inits axis default values */

    if(pad)
    {
        for(int i=0; i<PAD_AXIS_NUM; i++)
        {
            pad->axis[i].rawValue = 0;
            pad->axis[i].value = 0;
            pad->axis[i].zone = 0;
            pad->axis[i].calib[0] = PAD_ANA_MIN_VAL;
            pad->axis[i].calib[1] = PAD_ANA_MAX_VAL;
        }
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Argument is NULL.\n", __FUNCTION__, __LINE__ );
    }
}


void CEV_padFreeAll(void)
{/*Frees all opened pad*/

    CEV_Input *input = CEV_inputGet();

    for (int i=0; i<input->padNum; i++)
        CEV_padFreeThis(&input->pad[i]);

	free(input->pad);
}


void CEV_padFreeThis(CEV_Pad *pad)
{/*Frees one pad*/

    if (pad)
    {
        if(pad->haptic != NULL)
            SDL_HapticClose(pad->haptic);

        if((pad->joy != NULL) && SDL_JoystickGetAttached(pad->joy))
            SDL_JoystickClose(pad->joy);
    }
}


float CEV_padAngle(Sint16 axisX, Sint16 axisY)
{/*Analog stick angle*/
/**la valeur de retour est exploitable par le rotozoom*/

    float val = 0;/*resultat de calcul*/

    val = (180*(atan((float)axisY/(float)axisX))/PI);

    if (axisX<0)
        return(180.0 + val);
    else if (axisY<0)
        return(360.0 + val);
    else
        return(val);
}


int CEV_padCircularZone(const CEV_Pad *pad)
{/*Circular range divider**/

    int value = 0;
    CEV_ICoord pos,
            padPos;

    Sint16 plage =(PAD_ANA_MAX_VAL - pad->analogDeadRange)/pad->numOfZone;/*partage de la zone active en PAD_ZONE_NUM plages*/

    pos.x       = pos.y  =0;
    padPos.x    = pad->axis[PAD_X_AXIS].value;
    padPos.y    = pad->axis[PAD_Y_AXIS].value;

    value = CEV_icoordDist(pos, padPos);

    return(value/plage);
}


void CEV_padDump(const CEV_Pad *pad)
{/*Dumps pad sts to stdout**/

    printf(" Pad name : %s\n", SDL_JoystickName(pad->joy));
    printf(" Pad id : %d \n", pad->id);
    printf(" Pad is haptic : %s\n", (pad->haptic == NULL)? "no" : "yes");
    printf(" Pad button : %d\n",pad->btNum);
    printf(" Pad axis : %d\n",pad->axesNum);
}


void CEV_padAddWarm(int index)
{/*Adds warm plugged joystick*/

    fprintf(stdout, "Adding new pad index %d.\n", index);

    CEV_Input * input = CEV_inputGet();
    CEV_Pad newPad;
    CEV_Pad* temp = NULL;


    CEV_padClear(&newPad);//everything to 0

    newPad.joy = SDL_JoystickOpen(index);

    if (newPad.joy == NULL)
        return;

    temp = realloc(input->pad, (input->padNum+1)* sizeof(*temp));//extending table

    if(temp == NULL)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to allocate new added joystick : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    CEV_padInitThis(&newPad);

    input->pad                = temp;
    input->pad[input->padNum] = newPad;
    input->padNum++;

    CEV_padSortByID(input->pad, input->padNum);

   return;

err_1 :
    SDL_JoystickClose(newPad.joy);

    return;
}


void CEV_padRemoveWarm(int id)
{/*Removes warm unplugged pad from list*/

    CEV_Input *input = CEV_inputGet();

    fprintf(stderr, "Removing pad #%d\n", id);

    input->pad[id].isPlugged = false;
    //input->padNum --;

    CEV_padSortByID(input->pad, input->padNum);

}


void CEV_padSortByID(CEV_Pad *pad, int num)
{/*Sorts pad by id**/

    CEV_Pad temp;

    if (num<2)
        return;

    for (int i=0; i < num-1; i++)
    {
        for (int j=0; j < num-1; j++)
        {
            if (pad[j].id > pad[j+1].id)
            {
                temp     = pad[j];
                pad[j]   = pad[j+1];
                pad[j+1] = temp;
            }
        }
    }
}


    /*---------- Locals functions ---------*/

//added 20210513
int16_t L_padAxisScale(int16_t realVal, int16_t realMin, int16_t realMax, int16_t retMin, int16_t retMax)
{/*Axis scaling*/

    int16_t result;

    if(((realMax-realMin) + retMin)!= 0)
        result = ((retMax-retMin)*(realVal-realMin)/(realMax-realMin) + retMin);
    else
        return 0;

    if(result<retMin)
        result = retMin;
    else if (result>retMax)
        result = retMax;

    return result;
}
