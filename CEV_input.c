//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2017    |   1.0    |    SDL2 rev    **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**   CEV    |    03-2020    |   1.0.2  |   behaviour    **/
//**********************************************************/
// - mouse relative corrected 20170416
// - keyrepeat suppressed 20200108
// - updateInput return value supports any event 20200319

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "math.h"
#include "project_def.h"
#include "CEV_mixSystem.h"
#include "CEV_input.h"
#include "CEV_pad.h"
#include "CEV_api.h"


/*LOCAL FUNCTIONS DECLARATION*/

//add a new pad
void L_padAddWarm(int index);

//removes a pad
void L_padRemoveWarm(int id);

//rearrange pad list
void L_padSortByID(CEV_Pad * pad, int num);




bool CEV_inputUpdate(void)
{//updating inputs

    CEV_Input* input = CEV_inputGet();//main struc

    SDL_Event event;    //events
    bool result = false;//any button / quit

    //default values / SDL2->wheel axis to button
    input->mouse.button[SDL_BUTTON_WHEELDOWN]                   = 0;
    input->mouse.button[SDL_BUTTON_WHEELUP]                     = 0;
	input->mouse.button[SDL_BUTTON_WHEELLEFT]                   = 0;
	input->mouse.button[SDL_BUTTON_WHEELRIGHT]                  = 0;
    input->lastKeyVal = input->mouse.rel.x = input->mouse.rel.y = 0;
    input->text = NULL;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            /**keyboard**/
            case SDL_KEYDOWN :
                if(!event.key.repeat)
                {
                    input->key[event.key.keysym.scancode]   = true;
                    input->lastKeyVal                       = event.key.keysym.scancode;
                    input->lastDevice                       = KEYBOARD;
                    result                                  = true;
                }
            break;

            case SDL_KEYUP :
                input->key[event.key.keysym.scancode]   = false;
                input->lastDevice                       = KEYBOARD;
            break;

            case SDL_TEXTINPUT:
                    input->text         = event.text.text;
                    input->lastDevice   = KEYBOARD;
                    result              = true;
            break;

            /**mouse**/
            case SDL_MOUSEBUTTONDOWN :
                    input->mouse.button[event.button.button] = true;
                    input->lastDevice                        = MOUSE;
                    result                                   = true;
            break;

            case SDL_MOUSEBUTTONUP :
                input->mouse.button[event.button.button] = false;
                input->lastDevice = MOUSE;
            break;


            case SDL_MOUSEWHEEL : //SDL2 : axis to button
                if (event.wheel.y >0)
                    input->mouse.button[SDL_BUTTON_WHEELUP]     = true;
                else if (event.wheel.y <0)
                    input->mouse.button[SDL_BUTTON_WHEELDOWN]   = true;

                if (event.wheel.x >0)
                    input->mouse.button[SDL_BUTTON_WHEELRIGHT]  = true;
                else if (event.wheel.x <0)
                    input->mouse.button[SDL_BUTTON_WHEELLEFT]   = true;

                input->lastDevice = MOUSE;
                result            = true;
            break;


            case SDL_MOUSEMOTION :
                input->mouse.pos.x  = event.motion.x;
                input->mouse.pos.y  = event.motion.y;
                input->mouse.rel.x  += event.motion.xrel;
                input->mouse.rel.y  += event.motion.yrel;
                input->lastDevice   = MOUSE;
            break;

            /**pads**/
            case SDL_JOYBUTTONDOWN :
                input->pad[event.jbutton.which].button[event.jbutton.button]    = true;
                input->lastDevice                                               = PAD;
                result                                                          = true;
            break;

            case SDL_JOYBUTTONUP :
                input->pad[event.jbutton.which].button[event.jbutton.button]    = false;
                input->lastDevice                                               = PAD;
            break;

            case SDL_JOYHATMOTION :
                input->pad[event.jhat.which].hat    = event.jhat.value;
                input->lastDevice                   = PAD;
                result                              = true;
            break;

            case SDL_JOYAXISMOTION :
                input->pad[event.jaxis.which].axisValue[event.jaxis.axis]   = event.jaxis.value;
                input->lastDevice                                           = PAD;
                result                                                      = true;
            break;

            case SDL_JOYDEVICEADDED :
                //printf("pad added at index :%d\n", event.jdevice.which);
                L_padAddWarm(event.jdevice.which);  //added by index

            break;

            case SDL_JOYDEVICEREMOVED :
                //printf("pad removed at id = %d\n", event.jdevice.which);
                L_padRemoveWarm(event.jdevice.which);   //removed by Id
            break;

            /**misc**/

            case SDL_QUIT :
                input->window.quitApp = true;
                result                = true;
            break;

            case SDL_WINDOWEVENT :

                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_LOST :
                        input->window.isFocused = false;
                    break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED :
                        input->window.isFocused = true;
                    break;

                    default :
                    break;
                }

            default:
            break;
        }
    }

    return result;
}


int CEV_inputInit()
{/**initialisation de la structure des entrées***valide***/

    /*---DECLARATIONS---*/
    CEV_Input* input = NULL;

    /*---EXECUTION---*/

    input = &CEV_systemGet()->input;

    if(!input)
        return FATAL;

    CEV_inputSet(input);/*création du lien vers la structure*/
    CEV_inputClear();

    return FUNC_OK;
}


void CEV_inputFree()
{/**destruction**/
    CEV_Input *input = CEV_inputGet();

    if (input)
        CEV_padFreeAll();

    //free(input);
}


void CEV_inputClear()
{/**resets all input values*/

    CEV_Input *input = CEV_inputGet();

    if (input)
    {
        input->lastKeyVal   = 0;
        //input->lastDevice   = KEYBOARD;

        for(int i=0; i<MSE_NB_BT; i++)
            input->mouse.button[i] = false;


        for(int i=0; i<SDL_NUM_SCANCODES; i++)
            input->key[i] = false;


        for(int i=0; i<input->padNum; i++)
        {
            input->pad[i].hat = 0;

            for(int j=0; j<input->pad[i].btNum; j++)
            {
                input->pad[i].button[j] = false;
                /*
                for(int k=0; k<input->pad[i].axesNum; k++)
                    input->pad[i].axisValue[k] = 0;*/
            }
        }


        input->window.isFocused = 1;
        input->window.quitApp = false;
    }
}


CEV_Input* CEV_inputSet(CEV_Input *ptr)
{/**record inputs ptr**/

    static CEV_Input* funcSts = NULL;

    if (ptr != NULL)
        funcSts = ptr;

    return funcSts;
}


CEV_Input* CEV_inputGet()
{/**fetch inputs**/
    return CEV_inputSet(NULL);
}


void CEV_inputValue(int *val)
{
    bool anyValue = false;
    int lValue = 0;
    CEV_Input *input = CEV_inputGet();

    for(int i = SDL_SCANCODE_KP_1; i<=SDL_SCANCODE_KP_0; i++)
    {//layer display / input value
        if(input->key[i])
        {
            lValue = (i-SDL_SCANCODE_KP_1+1) %10;
            input->key[i] = false;
            anyValue = true;
        }
    }

    for(int i = SDL_SCANCODE_1; i<=SDL_SCANCODE_0; i++)
    {//option display / input value

        if(input->key[i])
        {
            lValue = (i-SDL_SCANCODE_1+1) %10;
            input->key[i] = false;
            anyValue = true;
        }
    }

    if (anyValue)
    {
        *val *= 10;
        *val += lValue;
    }
}

int CEV_mouseBoxPtr(SDL_Rect** box ,int num)
{/**mouse box with ptr table**/

    SDL_Point *msePos = &CEV_inputGet()->mouse.pos;

    for (int i=0; i<num; i++)
    {
        if (CEV_pointIsInRect(*msePos,*box[i]))
            return i;
    }

    return -1;
}


int CEV_mouseBox(SDL_Rect* box ,int num)
{/**mouse box**/

    SDL_Point *msePos = &CEV_inputGet()->mouse.pos;

    for (int i=0; i<num; i++)
    {
        if (CEV_pointIsInRect(*msePos, box[i]))
            return i;
    }

    return -1;
}



/*LOACAL FUNCTION IMPLEMENTATION*/

void L_padAddWarm(int index)
{//adds a joystick

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

    L_padSortByID(input->pad, input->padNum);

   return;

err_1 :
    SDL_JoystickClose(newPad.joy);

    return;
}


void L_padRemoveWarm(int id)
{/**remove a pad from list*/

    CEV_Input *input = CEV_inputGet();

    fprintf(stderr, "Removing pad #%d\n", id);

    input->pad[id].isPlugged = false;
    //input->padNum --;

    L_padSortByID(input->pad, input->padNum);

}


void L_padSortByID(CEV_Pad * pad, int num)
{/*sorts pad by id*/

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

