//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2023    |   1.0    |    creation    **/
//**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL.h>
#include "CEV_api.h"
#include "CEV_chrono.h"




/*
void TEST_cevChrono(void)
{//functions stress and testing

#include <CEV_mixSystem.h>
    CEV_Chrono chrono;
    CEV_Input* input = CEV_inputGet();
    CEV_Edge pauseBt;

    bool quit = false;

    CEV_chronoSet(&chrono);

    while(!quit)
    {
        CEV_inputUpdate();

        quit = input->key[SDL_SCANCODE_ESCAPE];

        pauseBt.value = input->key[SDL_SCANCODE_SPACE];

        CEV_edgeUpdate(&pauseBt);

        CEV_chronoUpdate(&chrono);


        if(pauseBt.re)
        {
            CEV_edgeUpdate(&pauseBt);
            CEV_chronoPause(&chrono);

            while(!pauseBt.re)
            {
                CEV_inputUpdate();
                pauseBt.value = input->key[SDL_SCANCODE_SPACE];
                CEV_edgeUpdate(&pauseBt);
            }

            CEV_chronoUnpause(&chrono);
        }

        printf("%u\n", CEV_chronoTicks());
    }
}
*/


CEV_Chrono* CEV_chronoSet(CEV_Chrono *in)
{
    static CEV_Chrono *this = NULL;

    if(NOT_NULL(in))
    {
        this = in;
    }

    return this;
}


CEV_Chrono* CEV_chronoGet(void)
{
    return CEV_chronoSet(NULL);
}


uint32_t* CEV_chronoTicks(void)
{
    return &CEV_chronoSet(NULL)->timeElapsed;
}


void CEV_chronoStart(CEV_Chrono *in)
{
    in->timeStart   = SDL_GetTicks();
    in->timeElapsed = in->timePause = 0;
}


uint32_t CEV_chronoUpdate(CEV_Chrono *in)
{
    return in->timeElapsed = SDL_GetTicks() - in->timeStart;
}


void CEV_chronoPause(CEV_Chrono *in)
{
    in->timePause   = SDL_GetTicks();
}


void CEV_chronoUnpause(CEV_Chrono *in)
{
    uint32_t pause = SDL_GetTicks() - in->timePause;
    in->timeStart += pause;

    CEV_chronoUpdate(in);

}
