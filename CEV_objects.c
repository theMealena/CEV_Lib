
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include "CEV_objects.h"
#include "CEV_platform.h"
#include "CEV_button.h"


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

    if(IS_PLTFRM(objectId))
    {
        result = (CEV_Object*)CEV_platformLoad_RW(src, freeSrc);
    }
    else if(IS_SWITCHBTN(objectId))
    {
        result = (CEV_Object*)CEV_switchButtonLoad_RW(src, freeSrc);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown.\n", __FUNCTION__, __LINE__, objectId);
        if(freeSrc)
            SDL_RWclose(src);
    }

    if (IS_NULL(result))
    {//on error
        fprintf(stderr, "Err at %s / %d : returning NULL result.\n", __FUNCTION__, __LINE__);
    }

    return result;
}


int CEV_objectTypeRead_RW(SDL_RWops* src, CEV_Object* dst, bool freeSrc)
{

    if (IS_NULL(src) || IS_NULL(dst))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : Received NULL arg.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    uint32_t objectId = SDL_ReadLE32(src);
    SDL_RWseek(src, -(sizeof(uint32_t)), RW_SEEK_CUR);

    if(IS_PLTFRM(objectId))
    {
        funcSts = CEV_platformTypeRead_RW(src, (CEV_Platform*)dst, freeSrc);
    }
    else if(IS_SWITCHBTN(objectId))
    {
        funcSts = CEV_switchButtonTypeRead_RW(src, (CEV_SwitchButton*)dst, freeSrc);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown.\n", __FUNCTION__, __LINE__, objectId);
        if(freeSrc)
            SDL_RWclose(src);

        funcSts = FUNC_ERR;
    }

    return funcSts;
}


void CEV_objectDestroy(CEV_Object* this)
{
    if (IS_PLTFRM(this->id))
        CEV_platformDestroy(this);

    else if IS_SWITCHBTN(this->id)
        CEV_switchButtonDestroy(this);

    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown, free might not be clean if object embeds allocations.\n", __FUNCTION__, __LINE__, this->id);
        free(this);
    }
}
