//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    05-2016    |   1.0    |  creation/SDL2 **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**********************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <SDL.h>
#include "CEV_gif.h"
#include "CEV_gifDeflate.h"
#include "CEV_gifToSurface.h"


/***Local Structures declarations****/

typedef struct L_GifDicoEntry
{/*dictionnary element*/
    int16_t prev;
    uint8_t value;
}
L_GifDicoEntry;//local


typedef struct L_GifDico
{/*full lzw dictionnary*/
    L_GifDicoEntry  entry[4096];
    uint16_t        actSize;
}
L_GifDico;//local


/***Local functions declaration****/

/*allocates and fills new structure*/
static L_GifFile *L_gifCreate(void);//local

/*Header file init*/
static void L_gifHeaderInit(L_GifHeader*);//local

/*Graphic Control Extension file init*/
static void L_gifGceInit(L_GifGCE*);//local

/*Logical Screen Descriptor file init*/
static void L_gifLsdInit(L_GifLSD*);//local

/*Comment Extension file init*/
static void L_gifComExtInit(L_GifComExt*);//local

/*allocates and fills image information*/
static void L_gifImgNew_RW(SDL_RWops*, L_GifFile*);//local

/*reads raw data sublock and send it thru lzw decompression*/
static void L_gifDataRead_RW(SDL_RWops*, L_GifFile*);//local

/*gets header from file*/
static void L_gifHeaderRead_RW(SDL_RWops*, L_GifHeader*);//local

/*gets misc extension blocks*/
static int L_gifExtRead_RW(SDL_RWops*, L_GifFile*);//local

/*gets Comment Extension*/
static void L_gifComExtRead_RW(SDL_RWops*, L_GifComExt*);//local

/*gets Logical Screen Descriptor*/
static void L_gifLsdRead_RW(SDL_RWops*, L_GifLSD*);//local

/*gets Logical Screen Descriptor byte Pack*/
static void L_gifLsdPackRead_RW(SDL_RWops*, L_GifLSDpack*);//local

/*gets color table*/
static void L_gifColorTabRead_RW(unsigned int, SDL_RWops*, L_GifColorTable*);//local

/*gets Graphic Control Extension*/
static void L_gifGceRead_RW(SDL_RWops*, L_GifGCE*);//local

/*gets Graphic Control Extension byte Pack*/
static void L_gifGcePackRead_RW(SDL_RWops*, L_GifGCEPack*);//local

/*gets Image Descriptor*/
static void L_gifIdRead_RW(SDL_RWops*, L_GifID*);//local

/*gets Image Descriptor byte Pack*/
static void L_gifIdPackRead_RW(SDL_RWops*, L_GifIDpack*);//local

/*skips useless blocks*/
static void L_gifBlockSkipRW(SDL_RWops*);//local

/*Performs LZW decompression*/
static void L_gifLzw(void*, L_GifFile*, unsigned int);//local

/*low level 16 bit extraction*/
static uint16_t L_gifGetBitFieldValue16(void*, unsigned int*, size_t);//local

/*dictionnary initialisation*/
static void L_gifDicoInit(L_GifDico*, unsigned int);//local

/*dictionnary sub functions*/
static uint8_t L_gifDicoGetFirstOfString(L_GifDico*, L_GifDicoEntry);//local
static void L_gifDicoStringOutput(L_GifDico*, L_GifDicoEntry, uint8_t*, unsigned int*, unsigned int);//local
static void L_gifDicoOutputRepeat(L_GifDico*, int16_t, uint8_t*, unsigned int*, unsigned int);//local
static void L_gifStreamValueOutput(uint8_t, uint8_t*, unsigned int*, unsigned int);//local


/***Functions Implementation***/


/**user's**/

L_GifFile *GIFL_gifLoad_RW(SDL_RWops* file)
{/*create gif handler*/

    L_GifFile *gif = NULL;

    unsigned char
                endOfFile=0;

    CEV_gifReadWriteErr = 0;

    /*creating new gif structure*/
    gif = L_gifCreate();

    if (gif == NULL)
    {/*on error*/
        fprintf(stderr,"Err at %s / %d : unable to create gif structure.\n", __FUNCTION__, __LINE__);
        goto err1;
    }

    L_gifHeaderRead_RW(file, &gif->header);  /*Mandatory / function checked*/
    L_gifLsdRead_RW(file, &gif->lsd);        /*Mandatory / function checked*/

    /*Global color table reading*/
    if (gif->lsd.packField.usesGlobalColor)
        L_gifColorTabRead_RW(gif->lsd.packField.numOfColors, file, &gif->globalColor);    /*function checked*/

    while(!endOfFile && !CEV_gifReadWriteErr)
    {
        uint8_t dataType = SDL_ReadU8(file);

        switch(dataType)
        {/*switch next data type*/

            case 0x21 : /*file extensions*/
                L_gifExtRead_RW(file, gif);
            break;

            case 0x2C : /*Image Descriptor*/
                L_gifImgNew_RW(file, gif);
            break;

            case 0x3B :/*EOF*/
                endOfFile = 1;
            break;

            default :
                fprintf(stderr, "Err at %s / %d : Unexpected value :%d\n", __FUNCTION__, __LINE__, dataType);
                CEV_gifReadWriteErr++;
            break;
        }
    }

err1:

    return gif;
}


void GIFL_gifFileFree(L_GifFile *gif)
{/*free all gif structure allocations*/

    if(gif == NULL)
        return;

    for(int i=0; i<gif->imgNum; i++)
    {
        free(gif->image[i].localColor.table);
        free(gif->image[i].imageData);
    }

    free(gif->globalColor.table);
    free(gif->image);
    free(gif);
}


/** locals **/

static int L_gifExtRead_RW(SDL_RWops* src, L_GifFile *dst)
{/*fills misc extension blocks*/

    switch(SDL_ReadU8(src))
    {/*switch src extension type*/

        case 0xf9 : /*Graphics Control Extension*/
            L_gifGceRead_RW(src, &dst->gce);
        break;

        /* NOTE (drx#1#): eventually to be taken into consideration but why ? */
        case 0x01 :/*Plain Text extension*/
        case 0xff :/*Application Extension*/
            L_gifBlockSkipRW(src);
        break;

        case 0xfe :/*Comment Extension*/
            L_gifComExtRead_RW(src, &dst->comExt);
        break;

        default :
            fprintf(stderr, "Err at %s / %d : Unknow file extension type has occured\n", __FUNCTION__, __LINE__);
			CEV_gifReadWriteErr++;
        break;
    }


    if(CEV_gifReadWriteErr)
        fprintf(stderr,"Warn at %s / %d : W/R error occured.\n", __FUNCTION__, __LINE__);

    return (CEV_gifReadWriteErr)? GIF_ERR : GIF_OK;
}


static L_GifFile *L_gifCreate()
{/*allocates and fills new structure*/

    L_GifFile *newgif = NULL;

    newgif = malloc(sizeof(*newgif));

    if(newgif == NULL)
    {
        fprintf(stderr,"Err at %s / %d : Unable to malloc : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return NULL;
    }

    /*initializing values & pointers*/
    newgif->imgNum                  = 0;
    newgif->image                   = NULL;
    newgif->globalColor.numOfColors = 0;
    newgif->globalColor.table       = NULL;

    /*init header*/
    L_gifHeaderInit(&newgif->header);

    /*init graphic control extension*/
    L_gifGceInit(&newgif->gce);

    /*init lsd*/
    L_gifLsdInit(&newgif->lsd);

    /*init comment extension*/
    L_gifComExtInit(&newgif->comExt);

    return newgif;
}


static void L_gifHeaderInit(L_GifHeader *header)
{/*header file init*/

    memset(header->signature, 0, sizeof(header->signature));
    memset(header->version, 0, sizeof(header->version));
}


static void L_gifGceInit(L_GifGCE *gce)
{/*gce file init*/

    gce->alphaColorIndex = 0;
    gce->byteSize        = 0;
    gce->delayTime       = 0;
    gce->used            = 0;

    gce->packField.alphaFlag      = 0;
    gce->packField.disposalMethod = 0;
    gce->packField.res            = 0;
    gce->packField.userInput      = 0;
}


static void L_gifLsdInit (L_GifLSD *lsd)
{/*lsd file init*/

    lsd->bckgrdColorIndex = 0;
    lsd->height           = 0;
    lsd->pxlAspectRatio   = 0;
    lsd->width            = 0;

    lsd->packField.colorRes         = 0;
    lsd->packField.numOfColors      = 0;
    lsd->packField.sorted           = 0;
    lsd->packField.usesGlobalColor  = 0;
}


static void L_gifComExtInit(L_GifComExt* comment)
{/*comment extension init*/

    comment->numOfBlocks = 0;
    comment->text        = NULL;
}


static void L_gifImgNew_RW( SDL_RWops* src, L_GifFile *dst)
{/*allocate and fills image information*/

    int frameIndex = dst->imgNum;/*rw purpose*/

    /*temporay image data*/
    L_GifImage *temp = NULL;

    /*one more frame to come*/
    dst->imgNum ++;

    temp = realloc(dst->image, dst->imgNum * sizeof(L_GifImage));

    if (temp != NULL)
        dst->image = temp;
    else
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to allocate temp :%s\n", __FUNCTION__, __LINE__, strerror(errno));
        dst->imgNum --;
        return;
    }

    if(dst->gce.used)
    {/*if graphic Control Extension was read before*/
        dst->image[frameIndex].control = dst->gce; /*store into new picture*/
    }

    /*fills Image Descriptor*/
    L_gifIdRead_RW(src, &dst->image[frameIndex].descriptor);

    if (dst->image[frameIndex].descriptor.imgPack.usesLocalColor)
    {/*if there is a local color table*/
        L_gifColorTabRead_RW(dst->image[frameIndex].descriptor.imgPack.colorTabSize, src, &dst->image[frameIndex].localColor);
    }
    else
    {/*default values*/
        dst->image[frameIndex].localColor.numOfColors    = 0;
        dst->image[frameIndex].localColor.table          = NULL;
    }

    /*restores raw data into pixels table*/
    L_gifDataRead_RW(src, dst);
}


static void L_gifDataRead_RW(SDL_RWops* src, L_GifFile *dst)
{/*reads raw data sublock and send it thru lzw decompression*/

    uint8_t
            *rawData    = NULL, /*data field only*/
            *temp       = NULL; /*temporary*/

    unsigned char
                LZWminiCodeSize = 0, /*min code size*/
                subBlockSize    = 0; /*size of data sub block*/

    size_t rawDataSize = 0; /*raw data full size in bytes*/

    LZWminiCodeSize = SDL_ReadU8(src);

    while((subBlockSize = SDL_ReadU8(src))) /*a size of 0 means no data to follow*/
    {/*fill rawData with subBlocks datas*/

        temp = realloc(rawData, rawDataSize + subBlockSize);/*alloc/realloc volume for new datas*/

        if (temp != NULL)/*realloc is ok*/
            rawData = temp;
        else
        {/*on error*/
            fprintf(stderr, "Err at %s / %d : realloc error. %s\n", __FUNCTION__, __LINE__, strerror(errno));
            goto err;
        }

        /*fetch datas from SDL_RWops*/
        SDL_RWread(src, rawData+rawDataSize, sizeof(uint8_t), subBlockSize);

        rawDataSize +=  subBlockSize;   /*increase datasize by blocksize*/
    }

    /*pass data through lzw deflate*/
    L_gifLzw(rawData, dst, LZWminiCodeSize);

err :
    free(rawData);
}


static void L_gifHeaderRead_RW(SDL_RWops* src, L_GifHeader* dst)
{/*fills header**/

    int i;

    for(i=0; i<3; i++)
        dst->signature[i] = SDL_ReadU8(src);

    for(i=0; i<3; i++)
        dst->version[i] = SDL_ReadU8(src);


    dst->signature[3] = dst->version[3] = '\0';
}


static void L_gifComExtRead_RW(SDL_RWops* src, L_GifComExt *dst)
{/*fills comment extension*/

    uint8_t temp;

    temp = SDL_ReadU8(src);
    temp+= 1;

    dst->text = malloc(temp * sizeof(char));
    if(dst->text == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to alloc : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        CEV_gifReadWriteErr++;
        return;
    }

    if(SDL_RWread(src, dst->text, 1, temp) != temp)
        CEV_gifReadWriteErr++;

    /*debug supprimé ici*/
    /*dst->text[temp-1]='\0';*/

    if(CEV_gifReadWriteErr)
        fprintf(stderr,"Err at %s / %d : R/W error while reading Comment Extension.\n", __FUNCTION__, __LINE__);
}


static void L_gifLsdRead_RW(SDL_RWops* src, L_GifLSD* dst)
{/*fills Logical Screen Descriptor*/

    uint8_t temp;

    dst->width  = SDL_ReadLE16(src);
    dst->height = SDL_ReadLE16(src);

    L_gifLsdPackRead_RW(src, &dst->packField);

/* NOTE (drx#1#): background color unused ? W8&C if it ever becomes a problem */
    dst->bckgrdColorIndex   = SDL_ReadU8(src);
    temp                    = SDL_ReadU8(src);/*useless info*/
    dst->pxlAspectRatio     = (temp + 15) / 64;
}


static void L_gifLsdPackRead_RW(SDL_RWops* src, L_GifLSDpack* dst)
{/*extract LSD's packed flags*/

    uint8_t temp = SDL_ReadU8(src);

    dst->usesGlobalColor   = (temp & 0x80)? 1 :0 ;
    dst->colorRes          = ((temp & 0x70)>>4) + 1;
    dst->sorted            = (temp & 0x08)? 1: 0;
    dst->numOfColors       = 1 << ((temp & 0x07)+1);
}


static void L_gifColorTabRead_RW(unsigned int numOfColor, SDL_RWops* src, L_GifColorTable *dst)
{/*fills color table*/

    int i;

    dst->table = malloc(numOfColor*sizeof(L_GifColor));

    if (dst->table == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate memory :%s\n", __FUNCTION__, __LINE__, strerror(errno));
        CEV_gifReadWriteErr++;
        return;
    }

    dst->numOfColors = numOfColor;

    for(i= 0; i<numOfColor; i++)
    {
        dst->table[i].r = SDL_ReadU8(src);
        dst->table[i].g = SDL_ReadU8(src);
        dst->table[i].b = SDL_ReadU8(src);
        dst->table[i].a = 0xff;
    }
}


static void L_gifGceRead_RW(SDL_RWops* src, L_GifGCE* dst)
{/*fills Graphic Control Extension*/

    dst->byteSize         = SDL_ReadU8(src);
    L_gifGcePackRead_RW(src, &dst->packField);
    dst->delayTime        = SDL_ReadLE16(src);
    dst->alphaColorIndex  = SDL_ReadU8(src);
    SDL_ReadU8(src); /*read garbage byte*/
    dst->used = 1;
}


static void L_gifGcePackRead_RW(SDL_RWops* src, L_GifGCEPack* dst)
{/*extract Graphic Control Extension's packed flags*/

    uint8_t temp;

    temp = SDL_ReadU8(src);

    dst->res               = (temp & 0xE0)>>5;
    dst->disposalMethod    = (temp & 0x1C)>>2;
    dst->userInput         = (temp & 0x02)>>1;
    dst->alphaFlag         = temp & 0x01;
}


static void L_gifIdRead_RW(SDL_RWops* src, L_GifID* dst)
{/*fills Image Descriptor*/

    dst->leftPos   = SDL_ReadLE16(src);
    dst->topPos    = SDL_ReadLE16(src);
    dst->width     = SDL_ReadLE16(src);
    dst->height    = SDL_ReadLE16(src);

    L_gifIdPackRead_RW(src, &dst->imgPack);
}


static void L_gifIdPackRead_RW(SDL_RWops* src, L_GifIDpack* dst)
{/*extract Image Descriptor's packed flag*/

    uint8_t temp;

    temp = SDL_ReadU8(src);

    dst->usesLocalColor  = (temp & 0x80)? 1 : 0;
    dst->interlace       = (temp & 0x40)? 1 : 0;
    dst->sorted          = (temp & 0x20)? 1 : 0;
    dst->res             = (temp & 0x18)? 1 : 0;
    dst->colorTabSize    = 1 << ((temp & 0x07)+1);
}


static void L_gifBlockSkipRW(SDL_RWops* src)
{/*skip useless subBlock in SDL_RWops*/

    uint8_t temp;

    while ((temp = (SDL_ReadU8(src))))
        SDL_RWseek(src, temp, RW_SEEK_CUR);
}


static void L_gifLzw(void *codeStream, L_GifFile *gif, unsigned int lzwMinCode)
{/*LZW Decompression*/

    /*** DECLARATIONS ***/

    unsigned int bitPtr     = 0,   /*actual bit position in codestream*/
                 count      = 0,   /*count of index output*/
                 indexNum   = gif->image[gif->imgNum-1].descriptor.width * gif->image[gif->imgNum-1].descriptor.height;

    uint8_t *indexStream    =  NULL; /*result*/

    size_t maskSize = lzwMinCode +1; /*size of mask used to read bitstream in code Stream*/

    uint16_t
            actCodeValue    = 0,/*actual code value*/
            prevCodeValue   = 0;/*previous code value*/

    const uint16_t
            resetCode       = 1<<lzwMinCode,
            EOICode         = resetCode + 1;

    L_GifDico codeTable;
    L_GifDicoEntry prevString;

    /*** PRL ***/

    indexStream = malloc(indexNum *sizeof(uint8_t));

    if(indexStream == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate indexStream : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return;
    }

    /*** EXECUTION ***/

    while(count<indexNum)
    {/*starts loop*/

        /*read value in code stream*/
        actCodeValue  = L_gifGetBitFieldValue16(codeStream, &bitPtr, maskSize);

        if(actCodeValue == resetCode)
        {/*reinit dico if reset code*/

            L_gifDicoInit(&codeTable, resetCode); /*CodeTable init*/
            maskSize    = lzwMinCode +1;            /*maskSize init*/

            actCodeValue =  L_gifGetBitFieldValue16(codeStream, &bitPtr, maskSize);

            prevString = codeTable.entry[actCodeValue];
            prevCodeValue = actCodeValue;
            L_gifStreamValueOutput(actCodeValue, indexStream, &count, indexNum);

            continue;
        }

        if(actCodeValue == EOICode)
        {
            fprintf(stderr,"Err at %s / %d : received EOI at %d; should be %d\n", __FUNCTION__, __LINE__, count, indexNum);
            break;
        }

        if(actCodeValue < codeTable.actSize)
        {/**is CODE in the code table? Yes:**/

            uint8_t string0 = L_gifDicoGetFirstOfString(&codeTable, codeTable.entry[actCodeValue]);
            /*output {CODE} to index stream */
            L_gifDicoStringOutput(&codeTable, codeTable.entry[actCodeValue], indexStream, &count, indexNum);

            codeTable.entry[codeTable.actSize].prev  = prevCodeValue;
            codeTable.entry[codeTable.actSize].value = string0;

            prevCodeValue = actCodeValue;

            prevString = codeTable.entry[actCodeValue];
            codeTable.actSize++;
        }
        else
        {/**is CODE in the code table? No:**/

            uint8_t prevString0 = L_gifDicoGetFirstOfString(&codeTable, prevString);

            L_gifDicoStringOutput(&codeTable, prevString, indexStream, &count, indexNum);
            L_gifStreamValueOutput(prevString0, indexStream, &count, indexNum);

            codeTable.entry[codeTable.actSize].prev = prevCodeValue;
            codeTable.entry[codeTable.actSize].value = prevString0;
            prevCodeValue   = codeTable.actSize;
            prevString      = codeTable.entry[codeTable.actSize];
            codeTable.actSize++;
        }

        if (codeTable.actSize == (1<<(maskSize)))
        {/*if index reaches max value for actMask*/

            maskSize++; /*maskSize increase*/

            if (maskSize>=13) /*max gif mask is 12 bits by spec*/
                maskSize = 12;/*limited to 12*/
        }
    }

    /** POST **/

    /*image data =  index stream*/
    gif->image[gif->imgNum-1].imageData = indexStream;
}


static uint16_t L_gifGetBitFieldValue16(void *data, unsigned int *bitStart, size_t maskSize)
{/*16 bits value extraction*/

    uint32_t value  = 0,
             mask   = (1<<maskSize) - 1;

    uint8_t* ptr    = data;

    if (maskSize > 16 || !data)
        return 0;

    ptr +=(*bitStart) / CHAR_BIT;

    value = ((*(uint32_t*)ptr) >> ((*bitStart)%CHAR_BIT)) & mask;

    (*bitStart) += maskSize ;

    return (uint16_t)value;
}


static void L_gifDicoInit(L_GifDico * dico, unsigned int code)
{/*init dictionnary*/
    int i;

    dico->actSize = code+2;

    for(i=0; i< code; i++)
    {
        dico->entry[i].prev  = -1;
        dico->entry[i].value = (uint8_t)i;
    }
}


static uint8_t L_gifDicoGetFirstOfString(L_GifDico *dico, L_GifDicoEntry entry)
{
    int16_t index = entry.prev ;

    if (index == -1)
        return entry.value;

    while(dico->entry[index].prev != -1)
        index = dico->entry[index].prev;

    return dico->entry[index].value;
}


static void L_gifDicoStringOutput(L_GifDico *dico, L_GifDicoEntry string, uint8_t *out, unsigned int *count, unsigned int maxCount)
{
    L_gifDicoOutputRepeat(dico, string.prev, out, count, maxCount);
    L_gifStreamValueOutput(string.value, out, count, maxCount);
}


static void L_gifDicoOutputRepeat(L_GifDico *dico, int16_t index, uint8_t *out, unsigned int *count, unsigned int maxCount)
{
    if(index==-1)
        return ;

    L_gifDicoOutputRepeat(dico, dico->entry[index].prev, out, count, maxCount);
    L_gifStreamValueOutput(dico->entry[index].value, out, count, maxCount);
    return ;
}


static void L_gifStreamValueOutput(uint8_t value, uint8_t *out, unsigned int *count, unsigned int maxCount)
{
    if (*count == maxCount)
        return;

    out[(*count)++] = value;

    return;
}
