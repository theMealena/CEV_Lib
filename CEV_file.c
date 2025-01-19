//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   1.0    |    creation    **/
//**********************************************************/


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "CEV_file.h"
#include "CEV_api.h"
#include "rwtypes.h"


int CEV_fileNumOfLine(FILE *file)
{//num of line in file (text mode / csv)

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int temp        = '*',   //temporary var
        lineNb      = 0,     //number of line in file
        start       = ftell(file),
        lastChar    = 0;     //last char of file for count

    //file ending type
    fseek(file, -1, SEEK_END);
    lastChar = fgetc(file);

    rewind(file);//rewind file

    do
    {
        temp = fgetc(file);

        if (temp == '\n')
            lineNb++;
    }
    while(temp != EOF);

    if(lastChar != '\n')
        lineNb++;

    //restores cursor position
    fseek(file, start, SEEK_SET);

    return (lineNb);
}


bool CEV_fileGotoLine(int line, FILE* file)
{//goes to line, (text mode / csv) true if found, false if not found / err

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return false;
    }
    int cpt = 0;

    rewind(file);

    while (cpt != line)
    {
        int temp = fgetc(file);

        if (temp == '\n')
            cpt++;
        else if (temp == EOF)
            return false;

    }

    return true;
}


bool CEV_fileGotoColumn(int num, FILE *file, int separator)
{//reaches begin of column (text mode / CSV)

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    int count = 0;

    while (count < num)
    {
        int temp = fgetc(file);

        if (temp == separator)
            count++;
        else if (temp == '\n')
            return false;
    }

    return true;
}


bool CEV_fileGotoNextLine(FILE *file)
{//reaches begin of next line (text mode)

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    int temp = '*';

    while((temp!='\n') && (temp!=EOF))
        temp = fgetc(file);

    if(temp == EOF)
        return false;

    return true;
}


size_t CEV_fileSize(FILE* file)
{//size of a file as byte

    if(IS_NULL(file))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return 0;
    }

    long pos = ftell(file), //recording pos
         size = 0;          //resulting size

    fseek(file, 0, SEEK_END); //reaching EOF

    size = ftell(file); //telling pos in file

    fseek(file, pos, SEEK_SET); //back to where it was

    return(size);
}


bool CEV_fileFileNameGet(const char* src, char* dst)
{//dst is filled with "name" from "folder/name"

    if(IS_NULL(src) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    int len = strlen(src);

    for(int i = len-1; i >= 0; i--)
    {
      if((src[i] == '/') || (src[i] == '\\'))
      {
        strcpy(dst, &src[i+1]);
        return true;
      }
    }

    return false;
}


bool CEV_fileFolderNameGet(const char *src, char *dst)
{//dst is filled with folder from folder/name, name provided with '/' or '\\'
//returns true if folder found, dst is filled with empty char '\0' otherwise

    if(IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return false;
    }

    bool funcSts = false;

    int i = 0;

    if (NOT_NULL(src))
    {
        int srcLen = strlen(src);

        for (i = srcLen; i >= 0; i--)
        {
            if ((src[i] =='/') || (src[i]=='\\'))
            {
                funcSts = true;
                break;
            }
        }
    }

    if (funcSts)
    {
        dst[i+1] = '\0';        //setting end of string
        for(int j = i; j >= 0; j--) //copying folder
            dst[j]= src[j];
    }
    else
        dst[0] = '\0';//empty string

    return funcSts;
}


bool CEV_fileFolderUp(char* str)
{// parent folder of folder/folder

    if(IS_NULL(str))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return 0;
    }

    bool funcSts = false;
    int srcLen   = strlen(str),
        i;

    for (i = srcLen; i >= 0; i--)
    {
        if ((str[i] =='/') || (str[i]=='\\'))
        {
            str[i] = '\0';
            funcSts = true;
            break;
        }
    }

    return funcSts;
}


void CEV_stringEndFormat(char* in)
{//replace CR/LF with nul char

    if(IS_NULL(in))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }

    int maxScan = strlen(in) - 1; /*'\n' and '\r' are included in strlen in binary mode*/

    for(int i = maxScan; i > maxScan-2; i--)
    {
        if (in[i]=='\r' || in[i]=='\n')
        {
            in[i] = '\0';
            //break;
        }
    }
}


void CEV_stringGroup(char *src, unsigned int group)
{//groupement de chiffres

    if(IS_NULL(src) || !group)
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return;
    }

    char *srcPtr, *dstPtr;       //nos pointeurs source et destination

    int length      = strlen(src);//taille de src pour savoir trouver la fin
    unsigned count  = 0;             //on va compter par group

    srcPtr = src + length; //on pointe la fin actuelle de src
    dstPtr = srcPtr + length/group - (length%group == 0); //on pointe la nouvelle fin supposée de src
    *(dstPtr+1) = '\0';                     //on met sa nouvelle fin de chaîne

    while (srcPtr >= src) //tant qu'on n'est pas au moins au début de src
    {
        *dstPtr = *srcPtr; //on recopie le char
        dstPtr--; //on passe au char précédent dans dst
        srcPtr--; //pareil pour src

        if (count == group) //si on est à group char copiés
        {
            *dstPtr =' ';   //on met un espace
            dstPtr--;       //on recule dans dst
            count = 0;      //on reset count
        }

        count++;  //incrément du nombre de boucles
    }
}


int CEV_fileStrSearch(FILE* file, char* src)
{//seeks string in file and returns line index

    if(IS_NULL(file) || IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

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


int CEV_fileCopy(char *srcName, char *dstName)
{//copies file

    if(IS_NULL(srcName) || IS_NULL(dstName))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    int funcSts = FUNC_OK;
    readWriteErr = 0;

    FILE *srcFile = fopen(srcName, "rb"),
         *dstFile = fopen(dstName, "wb");

    if(IS_NULL(srcFile) || IS_NULL(dstFile))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        funcSts = FUNC_ERR;
        goto end;
    }

    uint32_t fileSize = CEV_fileSize(srcFile);

    for(unsigned i=0; i<fileSize; i++)
    {
        write_u8(read_u8(srcFile), dstFile);
    }

    funcSts = readWriteErr? FUNC_ERR : FUNC_OK;

end:

    fclose(srcFile);
    fclose(dstFile);

    return funcSts;
}


int CEV_fileInsert(char* srcName, FILE* dst)
{//inserts srcName file into dst file
    int funcSts = FUNC_OK;

    if(IS_NULL(srcName) || IS_NULL(dst))
    {
        fprintf(stderr, "Err at %s / %d : NULL arg provided.\n", __FUNCTION__, __LINE__ );
        return ARG_ERR;
    }

    FILE* src = fopen(srcName, "rb");

    if(IS_NULL(src))
    {
        fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return FUNC_ERR;
    }

    uint32_t fileSize = CEV_fileSize(src);

    for(unsigned i=0; i<fileSize; i++)
    {
        write_u8(read_u8(src), dst);
    }

    fclose(src);

    funcSts = readWriteErr? FUNC_ERR : FUNC_OK;

    return funcSts;
}
