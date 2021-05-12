//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**   CEV    |    11-2017    |   1.0.2  | alloc corrected**/
//**   CEV    |    04-2020    |   1.0.3  | free corrected **/
//**********************************************************/

/**LOG**/
//06/04/200 CEV : modified free functions to allow NULL as argument causing crash otherwise


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <SDL.h>
#include "CEV_api.h"
#include "CEV_texts.h"
#include "rwtypes.h"



/*LOCAL FUNCTIONS DECLARATIONS**/

/*fills text field from rwops*/
static void L_rwopsLineFieldRead(CEV_Text* text, SDL_RWops *ops);

/*fills text field from data file**/
static void L_dataLineFieldRead(FILE* src, CEV_Text* dst);

/*fills text field from natural file*/
static int L_fileLineFieldRead(CEV_Text *text, FILE* file);

/*num of line and max line size in natural file*/
static int L_fileDim(CEV_Text *text, FILE* file);

/*creates suitable 2d table for char to be stored*/
static char** L_textAlloc(size_t lineNum, size_t lineSize);




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

    if ((dst == NULL) || (index >= dst->linesNum) || (src == NULL))
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    if  (strlen(src)+1 >= (dst->lineSize) )
    {
        fprintf(stderr, "Err at %s / %d : src exceeds dst size.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }

    strcpy(dst->line[index], src);

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

    puts("preparing to convert text");

    CEV_Text *lText = CEV_textLoad(srcName);

    if(IS_NULL(lText))
    {
        fprintf(stderr, "Err at %s / %d could not load file.\n", __FUNCTION__, __LINE__);
        return(FUNC_ERR);
    }

    CEV_textToData(lText, dstName);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    CEV_textFree(lText);

    return funcSts;
}


CEV_Text* CEV_textLoad(const char *fileName)
{/*loads natural text file into text structure*/

    CEV_Text *result = NULL;

    printf("opening %s...", fileName);
    FILE* file = fopen(fileName, "rb");

    if(IS_NULL(file))
    {
        puts("nok");
        fprintf(stderr, "Err at %s / %d opening %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return(NULL);
    }
    puts("ok");

    rewind(file);

    printf("converting file...");
    result = CEV_textLoadf(file);
    printf("%s\n", result? "ok":"nok");
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

    int i = 0,j = 0;
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

    printf("creating %s...", fileName);
    FILE* file = fopen(fileName, "wb");

    if (file == NULL)
    {
        fprintf(stderr, "Err at %s / %d : Unable to open file %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        funcSts = FUNC_ERR;
        goto exit;
    }
    puts("ok");

    printf("writting...");
    CEV_textTypeWrite(text, file);

    if (readWriteErr)
        funcSts = FUNC_ERR;

    printf("%s\n", readWriteErr? "nok" : "ok");

    fclose(file);

exit :
    return funcSts;
}


void CEV_textFree(CEV_Text* in)
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




int CEV_fileStrSearch(FILE* file, char* src)
{/*seeks string in file and return line index*/

    long            pos         = ftell(file);
    char            line[256];
    unsigned int    i           = 0;
    int result                  = -1;

    rewind(file);
    while (fgets(line, sizeof(line), file))
    {
        CEV_stringEndFormat(line);

        if(!strcmp(line, src))
        {
            result = i;
            break;
        }
        i++;
    }

    fseek(file, pos, SEEK_SET);

    return result;
}




/********LOCAL FUNCTIONS*******/

static void L_rwopsLineFieldRead(CEV_Text* text, SDL_RWops *ops)
{/*fills texts fields from rwops*/

    int i = 0,
        j = 0;

    while(i < text->linesNum)
    {
        uint8_t temp = SDL_ReadU8(ops);
        text->line[i][j] = temp;
        j++;

        if(temp == '\0')
        {
            i++;
            j = 0;
        }
    }
}


static void L_dataLineFieldRead(FILE* src, CEV_Text* dst)
{/*fills texts fields from dataFile*/

    int i = 0,
        j = 0;

    while(i < dst->linesNum)
    {
        uint8_t temp = read_u8(src);
        dst->line[i][j] = temp;
        j++;

        if(temp == '\0')
        {
            i++;
            j = 0;
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
        fgets(result[i], text->lineSize+1, file);/*fgets reads num-1 char, asshole...*/
        CEV_stringEndFormat(result[i]);
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
    text->lineSize++;//20190514 line added

    return (FUNC_OK);
}


static char** L_textAlloc(size_t lineNum, size_t lineSize)
{/*allocs table of char**/

    char** result = NULL;/*return value*/

    result = calloc(lineNum, sizeof(char*));/*dim 1*/

    if (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to alloc : %s\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    for (int i=0; i<lineNum; i++)/*dim 2*/
    {
        result[i] = malloc(lineSize*sizeof(char));

        if (result[i] == NULL)
        {
            fprintf(stderr, "Err at %s / %d : unable to alloc line %d :%s\n", __FUNCTION__, __LINE__, i, strerror(errno));
            goto err_2;
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



