//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2023    |   1.0    |    creation    **/
//**   CEV    |    09-2024    |   1.0.1  | Added into CEV_system **/
//**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL.h>
#include "CEV_api.h"
#include "CEV_mixSystem.h"
#include "CEV_chrono.h"





void TEST_CEV_chrono(void)
{//functions stress and testing


    CEV_Input* input = CEV_inputGet();
    CEV_Edge pauseBt;

    bool quit = false;

    CEV_Chrono *chrono = CEV_chronoGet();
    CEV_chronoStart();
    uint32_t *time = CEV_chronoTicksPtr();

    while(!quit)
    {
        CEV_inputUpdate();

        quit = input->key[SDL_SCANCODE_ESCAPE];

        pauseBt.value = input->key[SDL_SCANCODE_SPACE];

        CEV_edgeUpdate(&pauseBt);

        CEV_chronoUpdate();

        if(pauseBt.re)
        {
            if (CEV_chronoIsPaused())
                CEV_chronoUnpause();
            else
                CEV_chronoPause();
        }

        printf("%u\n", *time);
    }
}


void CEV_chronoInit(CEV_Chrono *in)
{
    if(IS_NULL(in))
    {
        fprintf(stderr, "Warn at %s / %d : Arg provided is NULL.\n", __FUNCTION__, __LINE__ );
        return;
    }

    in->isPaused    = true;
    in->isRunning   = false;
}


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


uint32_t CEV_chronoTicks(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
        return this->timeElapsed;
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
        return false;
    }
}


uint32_t* CEV_chronoTicksPtr(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
        return &this->timeElapsed;
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
        return false;
    }
}


bool CEV_chronoIsRunning(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
        return this->isRunning;
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
        return false;
    }
}


bool CEV_chronoIsPaused(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
        return this->isPaused;
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
        return false;
    }
}


void CEV_chronoStart(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
    {
        this->timeStart   = SDL_GetTicks();
        this->timeElapsed = this->timePause = 0;
        this->isRunning   = true;
        this->isPaused    = false;
    }
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
    }
}


uint32_t CEV_chronoUpdate(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
    {

        return (this->timeElapsed = this->isPaused? this->timeElapsed : (SDL_GetTicks() - this->timeStart));
    }
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
    }
}


void CEV_chronoPause(void)
{
    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
    {
        this->timePause   = SDL_GetTicks();
        this->isPaused    = true;
    }
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
    }
}


void CEV_chronoUnpause(void)
{
    uint32_t pause = 0;

    CEV_Chrono *this = CEV_chronoGet();

    if(NOT_NULL(this))
    {
        pause = SDL_GetTicks() - this->timePause;
        this->timeStart  += pause;
        this->isPaused    = false;
        CEV_chronoUpdate();
    }
    else
    {
        fprintf(stderr, "Warn at %s / %d : Requested CEV_Chrono is NULL, use CEV_chronoSet to set.\n", __FUNCTION__, __LINE__ );
    }
}
