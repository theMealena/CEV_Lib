
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>
#include "CEV_txtParser.h"
#include "CEV_objects.h"
#include "CEV_platform.h"
#include "CEV_button.h"

void CEV_objectDump(CEV_Object* this)
{//dumps object content

    puts("*** BEGIN CEV_Object ***");

    if(IS_NULL(this))
    {
        fprintf(stderr, "Err at %s / %d : NULL CEV_Object as arg.\n", __FUNCTION__, __LINE__ );
        return ;
    }
    else if (!IS_OBJECT(this->id))
    {
        fprintf(stderr, "Err at %s / %d : not an CEV_Object.\n", __FUNCTION__, __LINE__ );
        return;
    }

    if(IS_SWITCHBTN(this->id))
    {
        puts("This CEV_Object is a switch button :");
        CEV_switchButtonDump(&this->button);

    }
    else if(IS_PLTFRM(this->id))
    {
        puts("This CEV_Object is a platform :\n");
        CEV_platformDump(&this->platform);
    }
    else
    {
        printf("CEV_Object with id %08X is not referenced.\n", this->id);
    }

    puts("****END CEV_Object****");
}


CEV_Object* CEV_objectLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads any object according to its sub type

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


int CEV_objectSave(CEV_Object* src, char* fileName)
{//saves object into file

    int funcSts = FUNC_OK;

    if(IS_PLTFRM(src->id))
    {
        funcSts = CEV_platformSave(&src->platform, fileName);
    }
    else if(IS_SWITCHBTN(src->id))
    {
        funcSts = CEV_switchButtonSave(&src->button, fileName);
    }
    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown.\n", __FUNCTION__, __LINE__, src->id);
        funcSts = FUNC_ERR;
    }

    return funcSts;
}


int CEV_objectTypeRead_RW(SDL_RWops* src, CEV_Object* dst, bool freeSrc)
{//reads object according to its sub type

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
        funcSts = CEV_platformTypeRead_RW(src, &dst->platform, freeSrc);
    }
    else if(IS_SWITCHBTN(objectId))
    {
        funcSts = CEV_switchButtonTypeRead_RW(src, &dst->button, freeSrc);
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
{//destroys object and content if needed

    if (IS_PLTFRM(this->id))
        CEV_platformDestroy(&this->platform);

    else if IS_SWITCHBTN(this->id)
        CEV_switchButtonDestroy(&this->button);

    else
    {
        fprintf(stderr, "Err at %s / %d : Object %08X is unknown, free might not be clean if object embeds allocations.\n", __FUNCTION__, __LINE__, this->id);
        free(this);
    }
}


void CEV_objectUpdate(CEV_Object* this, CEV_Camera* cam, uint32_t now)
{//updates objects
    if(!now)
        now = SDL_GetTicks();

    if (IS_PLTFRM(this->id))
        CEV_platformUpdate(&this->platform, cam, now);

    else if IS_SWITCHBTN(this->id)
        CEV_switchButtonUpdate(&this->button, cam);

    else
    {
        fprintf(stderr, "Err at %s / %d : Object at %p with id %08X is unknown, free might not be clean if object embeds allocations.\n", __FUNCTION__, __LINE__, this, this->id);
        free(this);
    }
}


int CEV_objectConvertToData(const char* srcName, const char* dstName)
{//converts CSV editing file into file

    int funcSts = FUNC_OK;

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {//arg error
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    CEV_Text *src = CEV_textTxtLoad(srcName);

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return FUNC_ERR;
    }

    FILE* dst = fopen(dstName, "wb");

    if (IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto err_1;
    }

    funcSts = CEV_objectConvertTxtToDataFile(src, dst, srcName);

    fclose(dst);

err_1:
    CEV_textDestroy(src);

    return funcSts;
}


int CEV_objectConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName)
{//Writes to data file from CEV_Text.

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;

    uint32_t hex = CEV_txtParseHex32From(src, "type");

    switch(hex)
    {
        case PLTFRM_TYPE_ID :
            funcSts = CEV_platformConvertTxtToDataFile(src, dst, srcName);
        break;

        case SWITCHBTN_TYPE_ID :
            funcSts = CEV_switchButtonConvertTxtToDataFile(src, dst, srcName);
        break;

        default:
            fprintf(stderr, "Err at %s / %d : Not handled ID : %08X.\n", __FUNCTION__, __LINE__, hex);
            funcSts = FUNC_ERR;
        break;
    }

    return funcSts;
}
