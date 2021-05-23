//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**   CEV    |    11-2017    |   1.0.2  | alloc corrected**/
//**********************************************************/
/*
- CEV 20210406 : modified clear / destroy functions to allow NULL as argument causing crash otherwise
- CEV 20210522 : lines realloc'd if writing greater string.
    - single line alloc'd size is hidden in size_t behind '\0' char of the string

  */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <SDL.h>
#include "project_def.h"
#include "CEV_api.h"
#include "CEV_texts.h"
#include "rwtypes.h"



/*LOCAL FUNCTIONS DECLARATIONS**/

/*fills text structure from rwops*/
static void L_rwopsLineFieldRead(CEV_Text* text, SDL_RWops *ops);

/*fills text field from data file**/
static void L_dataLineFieldRead(FILE* src, CEV_Text* dst);

/*fills text field from natural file*/
static int L_fileLineFieldRead(CEV_Text *text, FILE* file);

/*num of line and max line size in natural file*/
static int L_fileDim(CEV_Text *text, FILE* file);

/*creates suitable 2d array for char to be stored*/
static char** L_textAlloc(size_t lineNum, size_t lineSize);

/*gets single line alloced capacity*/
static size_t L_lineAllocSizeGet(char* src);

/*hides line's alloces capacity*/
static void L_lineAllocSizeWrite(char* src, size_t size);


/*USER END FUNCTIONS*/

CEV_Text* CEV_textCreate(unsigned int lines, unsigned int maxLength)
{/*allocate empty text structure*/

    CEV_Text *result = NULL;

    result = malloc(sizeof(CEV_Text));

    if(result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto exit;
    }

    result->fileTerminator  = '\0';
    result->linesNum        = lines;
    result->lineSize        = maxLength;
    result->line            = L_textAlloc(lines, maxLength);

    if(result->line == NULL)
    {
        free(result);
        result = NULL;
    }

exit:
    return result;
}


int CEV_textWrite(CEV_Text* dst, unsigned int index, const char* src)
{/*writes one of the line*/

    if ((dst == NULL) || (src == NULL))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }
    else if(index >= dst->linesNum)
    {//new line
        return CEV_textAppend(dst, src);
    }
    else
    {//writes this line

        size_t alloced  = L_lineAllocSizeGet(dst->line[index]), //alloc allowed
                len     = strlen(src),
                lenRequested = len + CEV_TEXT_XTRA_ALLOC; //alloc needed

        if(lenRequested > alloced)
        {//needs realloc ?

            char* temp = realloc(dst->line[index], lenRequested);

            if(IS_NULL(temp))
            {
                fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
                return FUNC_ERR;
            }

            dst->line[index] = temp;
            alloced = lenRequested;
        }

        strcpy(dst->line[index], src);
        L_lineAllocSizeWrite(dst->line[index], alloced);

        dst->lineSize = MAX(dst->lineSize, len);
    }

    return FUNC_OK;
}


int CEV_textAppend(CEV_Text* dst, const char* src)
{/*appends new string line*/

    char**memo = realloc(dst->line, (dst->linesNum+1)*sizeof(char*));

    if(IS_NULL(memo))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }
    dst->line = memo;

    size_t  len = strlen(src),
            lineAlloc = len + CEV_TEXT_XTRA_ALLOC;

    dst->line[dst->linesNum] = malloc(lineAlloc);

    if(IS_NULL(dst->line[dst->linesNum]))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    strcpy(dst->line[dst->linesNum], src);
    L_lineAllocSizeWrite(dst->line[dst->linesNum], lineAlloc);
    dst->linesNum++;
    dst->lineSize = MAX(dst->lineSize, len);

    return FUNC_OK;
}


char* CEV_textRead(CEV_Text *src, unsigned int index)
{/*reads one line*/

    if ((src == NULL) || (index >= src->linesNum))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    return src->line[index];
}


int CEV_convertTextTxtToData(const char *srcName, const char *dstName)
{/*convert natural text file into text capsule file .tdat*/

    int funcSts = FUNC_OK;

    CEV_Text *lText = CEV_textLoad(srcName);

     if(IS_NULL(lText))
    {
        fprintf(stderr, "Err at %s / %d could not load file %s : %s.\n", __FUNCTION__, __LINE__, srcName, strerror(errno));
        return(FUNC_ERR);
    }

    CEV_textToData(lText, dstName);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    CEV_textDestroy(lText);

    return funcSts;
}


CEV_Text* CEV_textLoad(const char *fileName)
{/*loads natural text file into text structure*/

    CEV_Text *result = NULL;

    FILE* file = fopen(fileName, "rb");

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d opening %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return(NULL);
    }

    rewind(file);

    result = CEV_textLoadf(file);

    fclose(file);

    return result;
}


CEV_Text* CEV_textLoadf(FILE* src)
{/*create text structure from natural text opened file where it is*/

    /****DECLARATIONS****/

    CEV_Text *result = NULL;

    /**contrôles*/
    if (src == NULL)
    {
        fprintf(stderr, "Err at %s / %d : argument error, file is NULL.\n", __FUNCTION__, __LINE__);
        return(NULL);
    }

    /***prl**/
    result = calloc(1, sizeof(CEV_Text));

    if  (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    /****EXECUTION****/

    if (L_fileDim(result, src) != FUNC_OK)
        goto err_2;

    if (L_fileLineFieldRead(result, src) != FUNC_OK)
        goto err_2;

    return (result);

err_2:
    free(result);

err_1:
    return NULL;
}


void CEV_textTypeWrite(CEV_Text* text, FILE* dst)
{/*saves text struct as text data at file position*/

    int i = 0, j = 0;
    readWriteErr = 0;

    /**contrôles*/
    if ((dst == NULL) || (text == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        return;
    }

    write_u32le(text->linesNum, dst);
    write_u32le(text->lineSize, dst);

    while( i < text->linesNum)
    {
        char temp = text->line[i][j];
        write_u8(temp, dst);
        j++;

        if (temp == '\0')
        {
            i++;
            j = 0;
        }
    }
}


void CEV_textTypeRead(FILE *src, CEV_Text* dst)
{/*read text data file and fills text*/

    readWriteErr = 0;

    /**contrôles*/
    if ((src == NULL) || (dst == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        return;
    }

    dst->linesNum = read_s32le(src);
    dst->lineSize = read_s32le(src);

    dst->line = L_textAlloc(dst->linesNum, dst->lineSize);

    L_dataLineFieldRead(src, dst);

}



CEV_Text* CEV_textLoad_RW(SDL_RWops* ops, char freeData)
{/*loads text rwops into text structure*/

    CEV_Text *result = NULL;

    /**contrôles*/
    if (ops == NULL)
    {
        fprintf(stderr, "Err at %s / %d : argument error, ops is NULL.\n", __FUNCTION__, __LINE__);
        return(NULL);
    }

    /***prl**/
    result = calloc(1, sizeof(CEV_Text));

    if  (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : allocation : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    result->linesNum = SDL_ReadLE32(ops);
    result->lineSize = SDL_ReadLE32(ops);

    result->line = L_textAlloc(result->linesNum, result->lineSize);

    L_rwopsLineFieldRead(result, ops);

    if(freeData)
        SDL_RWclose(ops);

    return (result);


err_1:
    return NULL;

}


int CEV_textToData(CEV_Text *text, const char* fileName)
{/*saves text struc and creates .tdat file*/

    int funcSts = FUNC_OK;

    if((fileName == NULL) || (text == NULL))
    {
        fprintf(stderr, "Err at %s / %d : Argument Error.\n", __FUNCTION__, __LINE__);
        funcSts = ARG_ERR;
        goto exit;
    }

    FILE* file = fopen(fileName, "wb");

    if (file == NULL)
    {
        fprintf(stderr, "Err at %s / %d : Unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        funcSts = FUNC_ERR;
        goto exit;
    }

    CEV_textTypeWrite(text, file);

    if (readWriteErr)
        funcSts = FUNC_ERR;

    fclose(file);

exit :
    return funcSts;
}


void CEV_textDestroy(CEV_Text* in)
{/*frees it all*/

    if(IS_NULL(in))
        return;

    for(int i = 0; i < in->linesNum ; i++)
        free(in->line[i]);

    free(in->line);

    free(in);
}


void CEV_textClear(CEV_Text* in)
{/*frees and init content to 0 / NULL*/

    if(IS_NULL(in))
        return;

     for(int i = 0; i<in->linesNum; i++)
        free(in->line[i]);

    free(in->line);

    in->fileTerminator  = '\0';
    in->lineSize        = 0;
    in->linesNum        = 0;
    in->line            = NULL;
}



/********LOCAL FUNCTIONS*******/

static void L_rwopsLineFieldRead(CEV_Text* text, SDL_RWops *ops)
{/*fills texts fields from rwops*/

    int line = 0,
        car = 0;

    size_t alloced = L_lineAllocSizeGet(text->line[line]);

    while(line < text->linesNum)
    {
        uint8_t temp = SDL_ReadU8(ops);
        text->line[line][car] = temp;
        car++;

        if(temp == '\0')
        {
            //storing alloc size
            L_lineAllocSizeWrite(text->line[line], alloced);

            line++; //goto next line

            if(line < text->linesNum)
                alloced = L_lineAllocSizeGet(text->line[line]);

            car = 0;//resets character index
        }
    }
}


static void L_dataLineFieldRead(FILE* src, CEV_Text* dst)
{/*fills texts fields from dataFile*/

    int line = 0,
        car = 0;

    size_t allocSize = L_lineAllocSizeGet(dst->line[line]);//reading alloc'd size for line

    while(line < dst->linesNum)
    {
        uint8_t temp = read_u8(src);
        dst->line[line][car] = temp;
        car++;

        if(temp == '\0')
        {
            //storing alloc size
            L_lineAllocSizeWrite(dst->line[line], allocSize);
            line++;

            if(line < dst->linesNum)
                allocSize = L_lineAllocSizeGet(dst->line[line]);

            car = 0;
        }
    }
}


static int L_fileLineFieldRead(CEV_Text *text, FILE* file)
{/*fills and clean text field*/

    /**DECLARATIONS**/
    char** result;

    /**contrôles*/
    if(!file)
        return FUNC_ERR;

    /**EXECUTION**/
    result = L_textAlloc(text->linesNum, text->lineSize);

    if (!result)
        goto exit;

    for(int i=0; i < text->linesNum; i++)
    {
        size_t allocSize = L_lineAllocSizeGet(result[i]);//reading alloc'd size for line
        fgets(result[i], text->lineSize+1, file);/*fgets reads num-1 char, asshole...*/
        CEV_stringEndFormat(result[i]);
        L_lineAllocSizeWrite(result[i], allocSize);//writing alloc'd size for line
    }

    text->line = result;

exit:

    return FUNC_OK;
}


static int L_fileDim(CEV_Text *text, FILE* file)
{/*needs rewind before call to scan full file*/

    /****DECLARATIONS****/

    int temp        = '*',   /*temporary var*/
        maxLine     = 0,     /*largest line size*/
        lineNb      = 0;     /*number of line in file*/
    char lastChar   = 0;     /*last char of file for count*/
    long pos        = 0;     /*mem to bring cursor back to where it was*/

    /**PRL*/

    if ((file == NULL) || (text == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return(ARG_ERR);
    }

    pos = ftell(file);/*records actual position in file*/
    /*file ending type*/
    fseek(file, -1, SEEK_END);/*LF,CR*/
    lastChar = fgetc(file);

    /****EXECUTION****/

    fseek(file, pos, SEEK_SET);/*go back to file pos*/

    do
    {
        temp = fgetc(file);
        maxLine++;

        if (maxLine > text->lineSize)
                text->lineSize = maxLine;

        if (temp=='\n')
        {
            lineNb++;
            maxLine = 0;
        }
    }
    while(temp != EOF);

    if(lastChar != '\n')
    {
        lineNb++;
        if (maxLine > text->lineSize)
                text->lineSize = maxLine;
    }

    fseek(file, pos, SEEK_SET);/*restores file's pos*/

    text->linesNum         = lineNb;
    text->fileTerminator   = lastChar;
    text->lineSize++; //20190514 line added

    return (FUNC_OK);
}


static char** L_textAlloc(size_t lineNum, size_t lineSize)
{/*allocs table of char**/

    char** result = NULL;/*return value*/
    lineSize += CEV_TEXT_XTRA_ALLOC;

    result = calloc(lineNum, sizeof(char*));/*dim 1*/

    if (IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to alloc : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    for (int i=0; i < lineNum; i++)/*dim 2*/
    {
        result[i] = malloc(lineSize * sizeof(char)); //a bit more for extra datas

        if (IS_NULL(result[i]))
        {
            fprintf(stderr, "Err at %s / %d : unable to alloc line %d :%s\n", __FUNCTION__, __LINE__, i, strerror(errno));
            goto err_2;
        }
        else
        {//storing alloc size behind '\0' char
            result[i][0] = '\0';
            L_lineAllocSizeWrite(result[i], lineSize);
        }
    }

    return result;

err_2:

    for(int i =0; i<lineNum; i++)
    {
        if (result[i])
            free(result[i]);
    }
    free(result);

err_1:
    return NULL;
}


static size_t L_lineAllocSizeGet(char* src)
{//returns line's allocated size

    size_t* ptr = (uint32_t*)(src + (strlen(src) + 1));

    return *ptr;
}


static void L_lineAllocSizeWrite(char* src, size_t size)
{//writes line's allocated size

    size_t* ptr = (size_t*)(src + (strlen(src)+1));

    *ptr = size;
}

