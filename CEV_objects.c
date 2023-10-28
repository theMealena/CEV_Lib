
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include "CEV_objects.h"


CEV_Object* CEV_objectLoad_RW(SDL_RWops* src, bool freeSrc)
{
    if (IS_NULL(src))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : Received NULL arg.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    uint32_t objectId = SDL_ReadLE32(src);
    SDL_RWseek(src, 0, RW_SEEK_SET);

    CEV_Object* result = NULL;

    if(IS_PLTFRM_CST(objectId))
    {//platform constant definition
        result = (CEV_Object*) CEV_platformCstLoad_RW(src, freeSrc);
    }
    else if(IS_PLTFRM_INST(objectId))
    {
        result = (CEV_Object*) CEV_platformLoad_RW(src, freeSrc);
    }
    else if(IS_SWITCHBTN(objectId))
    {
        result = (CEV_Object*) CEV_switchButtonLoad_RW(src, freeSrc);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown.\n", __FUNCTION__, __LINE__, objectId);
        if(freeSrc)
            SDL_RWclose(src);
    }

    if (IS_NULL(result))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : returning NULL result.\n", __FUNCTION__, __LINE__);
    }



    return result;
}
