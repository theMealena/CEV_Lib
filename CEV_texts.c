//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**   CEV    |    11-2017    |   1.0.2  | alloc corrected**/
//**   CEV    |    02-2022    |   1.0.3  | sort added     **/
//**********************************************************/
/*
- CEV 2021_04_06 : modified clear / destroy functions to allow NULL as argument causing crash otherwise
- CEV 2021_05_22 : lines realloc'd if writing greater string : CEV_TEXT_XTRA_ALLOC defined
    - single line alloc'd size is hidden in size_t behind '\0' char of the string
- CEV 2022_07_24 : R/W fonctions revised or added / struct to capsule added / 0 lineNum tolerant / NULL tolerant
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include "project_def.h"
#include "CEV_api.h"
#include "CEV_texts.h"
#include "CEV_types.h"
#include "rwtypes.h"


#define CEV_TEXT_XTRA_ALLOC (sizeof(size_t) + 1) /**< num of bytes of extra alloc after strlen */


/*LOCAL FUNCTIONS DECLARATIONS**/


/** \brief Fills CEV_Text's text lines from virtual file.
 *
 * \param src  : SDL_RWops* to read from.
 * \param dst : CEV_Text* to write into.
 *
 * \return void
 */
static void L_LineFieldRead_RW( SDL_RWops *src, CEV_Text* dst);


/** \brief fills CEV_Text's text lines from data file.
 *
 * \param src : FILE* to read from.
 * \param dst : CEV_Text* to write into.
 *
 * \return void
 * note : readWriteErr updated.
 */
static void L_LineFieldRead(FILE* src, CEV_Text* dst);


/** \brief fills CEV_Text's text lines from text file.
 *
 * \param src : FILE* to read from.
 * \param dst : CEV_Text* to write into.
 *
 * \return int of std function status.
 */
static int L_LineFieldReadTxt(FILE* src, CEV_Text *dst);


/** \brief fills CEV_Text sizing fields from text file.
 *
 * \param text : CEV_Text* to fill with parameters.
 * \param file : FILE* to take measurements from.
 *
 * \return int : of standard function status.
 */
static int L_fileDim(CEV_Text *text, FILE* file);


/** \brief creates suitable 2d array for char to be stored.
 *
 * \param lineNum : size_t as num of line to be allocated.
 * \param lineSize : size_t as max length to store string.
 *
 * \return char** as allocated 2D array.
 */
static char** L_textAlloc(size_t lineNum, size_t lineSize);


/** \brief gets single line alloced capacity.
 *
 * \param src : char* to read allocation size from.
 *
 * \return size_t as allocated size (byte).
 */
static size_t L_lineAllocSizeRead(char* src);


/** \brief hides line's alloced capacity.
 *
 * \param src : char* to write allocation size into.
 * \param size : size_t as allocation size to write.
 *
 * \return void
 */
static void L_lineAllocSizeWrite(char* src, size_t size);


/** \brief Sorting lines AZ.
 *
 * \param q1 const void*
 * \param q2 const void*
 *
 * \return int
 *
 * note : not to be used, is qsort dedicated.
 */
static int L_textCompareAZ(const void* q1, const void* q2);


/** \brief Sorting lines ZA.
 *
 * \param q1 const void*
 * \param q2 const void*
 *
 * \return int
 *
 * note : not to be used, is qsort dedicated.
 */
 static int L_textCompareZA(const void* q1, const void* q2);

 /*
 void TEST_text(void)
 {

    CEV_Text *text = CEV_textTxtLoad("test.txt");

    printf("instance maxline = %u\n", text->lineSize);

    for(int i=0; i< text->linesNum; i++)
    {
        printf("line says : %s\n", CEV_textRead(text, i));
        //printf("this Alloc size is %u\n", L_lineAllocSizeGet(text->line[i]));
    }

    CEV_textWrite(text, 10, "toto part � l'aventure et est tr�s content de lui");
    puts("--- apres ajout----");

    printf("instance maxline = %u\n", text->lineSize);
    for(int i=0; i< text->linesNum; i++)
    {
        printf("line says : %s\n", CEV_textRead(text, i));
        //printf("this Alloc size is %u\n", L_lineAllocSizeGet(text->line[i]));
    }

    CEV_textDestroy(text);
 }
 */


/*USER END FUNCTIONS*/

CEV_Text* CEV_textCreate(unsigned int lines, unsigned int maxLength)
{//allocate empty text structure

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
    result->line            = lines? L_textAlloc(lines, maxLength) : NULL;

    if(lines && (result->line == NULL))
    {
        free(result);
        result = NULL;
    }

exit:
    return result;
}


int CEV_textWrite(CEV_Text* dst, unsigned int index, const char* src)
{//writes one of the line

    if ((dst == NULL) || (src == NULL))
    {//bad arg
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return ARG_ERR;
    }
    else if(index >= dst->linesNum || IS_NULL(dst->line))
    {//new line
        return CEV_textAppend(dst, src);
    }
    else
    {//writes this line

        size_t alloced  = L_lineAllocSizeRead(dst->line[index]), //alloc allowed
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
{//appends new string line

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
{//reads one line

    if ((src == NULL) || (index >= src->linesNum) || (src->line)== NULL)
    {
        fprintf(stderr, "Err at %s / %d : arg error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    return src->line[index];
}


void CEV_textSortAZ(CEV_Text *src)
{//sorts lines A to Z
    qsort(src->line, src->linesNum, sizeof(char*), L_textCompareAZ);
}


void CEV_textSortZA(CEV_Text *src)
{//sorts lines Z to A
    qsort(src->line, src->linesNum, sizeof(char*), L_textCompareZA);
}


int CEV_textConvertToData(const char *srcName, const char *dstName)
{//convert natural text file into text capsule file .tdat

    int funcSts = FUNC_OK;

    CEV_Text *lText = CEV_textTxtLoad(srcName);

     if(IS_NULL(lText))
    {
        fprintf(stderr, "Err at %s / %d could not load file %s : %s.\n", __FUNCTION__, __LINE__, srcName, strerror(errno));
        return(FUNC_ERR);
    }

    CEV_textSave(lText, dstName);

    if(readWriteErr)
        funcSts = FUNC_ERR;

    CEV_textDestroy(lText);

    return funcSts;
}


CEV_Text* CEV_textLoad(const char *fileName)
{//loads formated cevText file

    FILE* src = fopen(fileName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : unable to open %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return NULL;
    }

    CEV_Text* result = malloc(sizeof(CEV_Text));

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate CEV_Text :%s  .\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    CEV_textTypeRead(src, result);


end:
    fclose(src);

    return result;
}


CEV_Text* CEV_textLoad_RW(SDL_RWops* src, bool freeSrc)
{//loads CEV_Text from virtual file

    CEV_Text *result = NULL;

    if(IS_NULL(src))
        return NULL;

    uint32_t numOfLine  = SDL_ReadLE32(src),
             maxLineLen = SDL_ReadLE32(src);

    result = CEV_textCreate(numOfLine, maxLineLen);

    if(IS_NULL(result))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate CEV_Text : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto end;
    }

    L_LineFieldRead_RW(src, result);


end:
    if(freeSrc)
        SDL_RWclose(src);

    return result;
}


CEV_Text* CEV_textTxtLoad(const char *fileName)
{//loads text file into text structure

    CEV_Text *result = NULL;

    FILE* file = fopen(fileName, "rb");

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d opening %s : %s.\n", __FUNCTION__, __LINE__, fileName, strerror(errno));
        return(NULL);
    }

    rewind(file);

    result = CEV_textCreate(0, 0);//calloc(1, sizeof(CEV_Text));

    if  (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return(NULL);
    }

    /****EXECUTION****/

    //if (L_fileDim(result, file) != FUNC_OK)
        //goto err_1;

    if (L_LineFieldReadTxt(file, result) != FUNC_OK)
        goto err_1;

    fclose(file);

    return result;

err_1:
    free(result);
    return NULL;
}


CEV_Text* CEV_textTxtLoadf(FILE* src)
{//creates text structure from natural text opened file where it is

    CEV_Text *result = NULL;

    if (src == NULL)
    {
        fprintf(stderr, "Err at %s / %d : argument error, file is NULL.\n", __FUNCTION__, __LINE__);
        return(NULL);
    }

    result = CEV_textCreate(0, 0);

    if  (result == NULL)
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        goto err_1;
    }

    //if (L_fileDim(result, src) != FUNC_OK)
        //goto err_2;/

    if (L_LineFieldReadTxt(src, result) != FUNC_OK)
        goto err_2;

    return (result);

err_2:
    free(result);

err_1:
    return NULL;
}


int CEV_textSave(CEV_Text *text, const char* fileName)
{//saves text struc and creates .tdat file

    int funcSts = FUNC_OK;
    readWriteErr = 0;

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


void CEV_textTypeWrite(CEV_Text* text, FILE* dst)
{//saves text struct as text data at file position

    unsigned line = 0, car = 0;
    readWriteErr = 0;

    /**contr�les*/
    if ((dst == NULL) || (text == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        return;
    }

    write_u32le(text->linesNum, dst);
    write_u32le(text->lineSize, dst);

    while(line < text->linesNum)
    {
        char temp = text->line[line][car];
        write_u8(temp, dst);
        car++;

        if (temp == '\0')
        {
            line++;
            car = 0;
        }
    }
}


int CEV_textTypeWrite_RW(CEV_Text* src, SDL_RWops* dst)
{//saves text struct as text data at file position

    unsigned i = 0, j = 0;
    readWriteErr = 0;

    /**contr�les*/
    if ((dst == NULL) || (src == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        return readWriteErr;
    }

    readWriteErr += SDL_WriteLE32(dst, src->linesNum);
    readWriteErr += SDL_WriteLE32(dst, src->lineSize);

    while( i < src->linesNum)
    {
        char temp = src->line[i][j];
        readWriteErr += SDL_WriteU8(dst, temp);
        j++;

        if (temp == '\0')
        {
            i++;
            j = 0;
        }
    }

    return readWriteErr;
}


void CEV_textTypeRead(FILE *src, CEV_Text* dst)
{//read text data file and fills text

    readWriteErr = 0;

    /**contr�les*/
    if ((src == NULL) || (dst == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        readWriteErr++;
        return;
    }

    dst->linesNum = read_u32le(src);
    dst->lineSize = read_u32le(src);

    dst->line = L_textAlloc(dst->linesNum, dst->lineSize);

    L_LineFieldRead(src, dst);

}


void CEV_textTypeRead_RW(SDL_RWops* src, CEV_Text* dst)
{//reads CEV_Text from virtual file

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d :   .\n", __FUNCTION__, __LINE__ );
        readWriteErr++;
        return;
    }

    dst->linesNum = SDL_ReadLE32(src),
    dst->lineSize = SDL_ReadLE32(src);

    L_LineFieldRead_RW(src, dst);

    return;
}


/* not working
int CEV_textToCapsule(CEV_Text* src, CEV_Capsule* dst)
{//creates capsule from CEV_Text

    int funcSts = FUNC_OK;

    //creating virtual file
    SDL_RWops* vFile = SDL_AllocRW();

    if(IS_NULL(vFile))
    {
        fprintf(stderr, "Err at %s / %d : unable to allocate vFile : %s.\n", __FUNCTION__, __LINE__, SDL_GetError());
        return FUNC_ERR;
    }

    if(CEV_textTypeWrite_RW(src, vFile))
    {
        fprintf(stderr, "Err at %s / %d : unable to write into virtual file.\n", __FUNCTION__, __LINE__ );
        goto end;
    }

    size_t size = SDL_RWsize(vFile);
    dst->size = size;
    dst->type = IS_DTX;
    dst->data = malloc(size);

    if(IS_NULL(dst->data))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    SDL_RWread(vFile, dst->data, size, 1);

end:
    SDL_RWclose(vFile);
    SDL_FreeRW(vFile);
    return funcSts;
}
*/


void CEV_textDestroy(CEV_Text* in)
{//frees it all

    if(IS_NULL(in))
        return;

    for(unsigned i = 0; i < in->linesNum ; i++)
        free(in->line[i]);

    free(in->line);

    free(in);
}


void CEV_textClear(CEV_Text* in)
{//frees and init content to 0 / NULL

    if(IS_NULL(in))
        return;

    if(in->line)
    {
        for(unsigned i = 0; i<in->linesNum; i++)
            free(in->line[i]);
    }

    free(in->line);

    in->fileTerminator  = '\0';
    in->lineSize        = 0;
    in->linesNum        = 0;
    in->line            = NULL;
}


void CEV_textDump(CEV_Text* this)
{//dumps structure content

    puts("*** BEGIN CEV_Text ***");

    if (IS_NULL(this))
    {
        puts("This CEV_Text is NULL");
        goto end;
    }

    printf("Instance is at %p\n", this);
    printf("Text holds %u lines\n", this->linesNum);
    printf("of max size %u char\n", this->lineSize);

    for(unsigned i=0; i<this->linesNum; i++)
    {
        char* thisLine = CEV_textRead(this, i);
        printf("Line %d : %s ; (%u allocated)\n", i, thisLine, L_lineAllocSizeRead(thisLine));
    }

end:
    puts("*** END CEV_Text ***");
}


/********LOCAL FUNCTIONS*******/

static void L_LineFieldRead_RW(SDL_RWops* src, CEV_Text* dst)
{//fills texts fields from rwops

    unsigned line = 0,
             car  = 0;

    char lString[FILENAME_MAX] = "*";

    while(line < dst->linesNum)
    {
        char temp ='*';

        while(temp != '\0')
        {
            temp = lString[car] = SDL_ReadU8(src);
            car++;
        }

        CEV_textWrite(dst, line, lString);

        line++; //goto next line
        car = 0;//resets character index
    }
}


static void L_LineFieldRead(FILE* src, CEV_Text* dst)
{//fills texts fields from dataFile

    unsigned line = 0,
             car = 0;

    char lString[FILENAME_MAX] = "*";


    while(line < dst->linesNum)
    {
        char temp ='*';

        while(temp != '\0')
        {
            temp = lString[car] = read_u8(src);
            car++;
        }

        CEV_textWrite(dst, line, lString);

        line++; //goto next line
        car = 0;//resets character index

    }
}


static int L_LineFieldReadTxt(FILE* src, CEV_Text *dst)
{//fills and clean text field from txt file

    char lString[FILENAME_MAX] = "*";

    if(IS_NULL(src) || IS_NULL(dst))
        return ARG_ERR;


    while(!IS_NULL(fgets(lString, sizeof(lString), src)))
    {
        if(lString[0] != '/' && !iscntrl(lString[0]))
        {
            CEV_stringEndFormat(lString);
            CEV_textAppend(dst, lString);
        }
    }

    return FUNC_OK;
}


static int L_fileDim(CEV_Text *text, FILE* file)
{//needs rewind before call to scan full file

    int temp        = '*';   //temporary var

    unsigned
        maxLine     = 0,     //largest line size
        lineNb      = 0;     //number of line in file

    char lastChar   = 0;     //last char of file for count

    long pos        = 0;     //mem to bring cursor back to where it was

    if ((file == NULL) || (text == NULL))
    {
        fprintf(stderr, "Err at %s / %d : argument error.\n", __FUNCTION__, __LINE__);
        return(ARG_ERR);
    }

    pos = ftell(file);//records actual position in file

    //file ending type
    fseek(file, -1, SEEK_END);//LF,CR
    lastChar = fgetc(file);

    fseek(file, pos, SEEK_SET);//go back to file pos

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
    text->lineSize++; //2019 05 14 line added

    return (FUNC_OK);
}


static char** L_textAlloc(size_t lineNum, size_t lineSize)
{//allocs table of char

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


static size_t L_lineAllocSizeRead(char* src)
{//returns single line's allocated size

    size_t* ptr = (uint32_t*)(src + (strlen(src) + 1));

    return *ptr;
}


static void L_lineAllocSizeWrite(char* src, size_t size)
{//writes line's allocated size

    size_t* ptr = (size_t*)(src + (strlen(src)+1));

    *ptr = size;
}


static int L_textCompareAZ(const void* q1, const void* q2)
{//sorting sub func
    return strcmp(*(char**)q1, *(char**)q2);
}


static int L_textCompareZA(const void* q1, const void* q2)
{//sorting sub func
    return strcmp(*(char**)q2, *(char**)q1);
}
