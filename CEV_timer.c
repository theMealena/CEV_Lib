//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |  rev & tested  **/
//**********************************************************/


#include <stdint.h>
#include <SDL.h>
#include "CEV_Timer.h"
#include "CEV_api.h"



void CEV_timerInit(CEV_Timer* in, unsigned int val)
{
    in->run         = 0;
    in->accu        = (uint32_t)val;
    in->preset      = (uint32_t)val;
    in->lst         = SDL_GetTicks();
    in->cmd         = 0;
    in->processing  = 0;
    in->edge.value  = 0;
    in->edge.memo   = 0;
}


char CEV_timerTon(CEV_Timer *in)
{
    unsigned int sinceLast = 0;

    if (in->run && !in->processing && !in->cmd)
	{
        in->accu    = in->preset;
        in->lst     = SDL_GetTicks();
	}

	if(!in->run)
        in->accu = 0;

    in->processing = (in->run && in->accu);

    if (in->processing)
    {
        sinceLast   =  (SDL_GetTicks() - in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
    }

    in->cmd = (in->run && in->accu<=0);
	in->lst = SDL_GetTicks();


	return in->cmd;
}



char CEV_timerTof(CEV_Timer *in)
{

    unsigned int sinceLast = 0;

    if (in->run)
    {
        in->accu    = in->preset;
        in->lst     = SDL_GetTicks();
	}

    in->processing = !in->run && in->accu;

	if(in->processing)
	{
	    sinceLast   = (SDL_GetTicks()-in->lst);
        in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
	}

    in->cmd = in->run || in->processing;
	in->lst = SDL_GetTicks();

	return in->cmd;
}



char CEV_timerDelayOn(CEV_Timer* in)
{

    unsigned int sinceLast = 0;

    in->edge.value = in->run;

	if (CEV_edgeRise(&in->edge) && !in->processing && !in->cmd)
	{
	    in->accu    = in->preset;
	    in->lst     = SDL_GetTicks();
	}

    in->processing = (!in->cmd) && (in->accu>0);

	if(in->processing)
	{
        sinceLast   = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
	}

	in->cmd = in->processing && !in->accu;
	in->lst = SDL_GetTicks();

	return(in->cmd);
}



char CEV_timerDelayOff(CEV_Timer* in)
{

    unsigned int sinceLast = 0;

    in->edge.value = in->run;

	if (CEV_edgeRise(&in->edge) && !in->processing && !in->cmd)
	{
	    in->accu    = in->preset;
	    in->lst     = SDL_GetTicks();
	}

    in->processing = (in->accu != 0);

	if(in->processing)
	{
        sinceLast = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
	}

	in->cmd = (in->accu != 0);
	in->lst = SDL_GetTicks();

	return(in->cmd);
}



char CEV_timerRepeat(CEV_Timer* in)
{
    unsigned int sinceLast = 0;

    if (in->run && !in->processing)
	{
	    in->accu    = in->preset;
	    in->lst     = SDL_GetTicks();
	}

	if(!in->run)
        in->accu = 0;

    in->cmd         = in->run && !in->processing;
    in->processing  = in->run && in->accu;

	if(in->processing)
	{
        sinceLast   = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
	}

	in->lst = SDL_GetTicks();

	return(in->cmd);
}



char CEV_timerRepeatLocked(CEV_Timer* in)
{
    unsigned int sinceLast = 0;

    in->cmd = in->run && !in->processing;

	if (in->run && !in->processing)
	{
	    in->accu    = in->preset;
	    in->lst     = SDL_GetTicks();
	}

	in->processing = in->accu>0;

	if(in->processing)
	{
        sinceLast   = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
	}

	in->lst = SDL_GetTicks();

	return(in->cmd);
}


char CEV_timerEverBlink(CEV_Timer *in)
{
    unsigned int sinceLast = 0;

    if(in->run && !in->processing)
    {
        in->accu    = in->preset;
        in->lst     = SDL_GetTicks();
    }

    if(in->run)
    {
        sinceLast   = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
    }

    in->processing = in->accu && in->run;
    in->cmd =(in->accu>(in->preset/2)) && in->processing;
    in->lst = SDL_GetTicks();

    return(in->cmd);
}


char CEV_timerBlinkLimited(CEV_Timer *in)
{
    unsigned int sinceLast = 0;

    if(in->run && !in->processing)
    {
        in->accu    = in->preset;
        in->lst     = SDL_GetTicks();
        in->run     = 0;
    }

    in->processing = (in->accu>0)? 1: 0;

    if(in->processing)
    {
        sinceLast   = (SDL_GetTicks()-in->lst);
		in->accu    -= (in->accu - sinceLast > 0)? sinceLast: in->accu;
    }

    in->cmd = ((in->accu/200)%2) && in->processing;
    in->lst = SDL_GetTicks();

    return(in->cmd);
}


void CEV_timerReset(CEV_Timer *in)
{
    in->accu = in->preset;
    in->lst  = SDL_GetTicks();
}

