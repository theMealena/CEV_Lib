#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "CEV_file.h"


int CEV_fileNumOfLine(FILE *file)
{/**détermine le nombre de ligne dans le fichier txt*/

    /****DECLARATIONS****/

    int temp        = '*',   /*temporary var*/
        lineNb      = 0,     /*number of line in file*/
        start       = ftell(file),
        lastChar    = 0;     /*last char of file for count*/

    /***prl**/
    /*file ending type*/
    fseek(file, -1, SEEK_END);
    lastChar = fgetc(file);

    /****EXECUTION****/

    rewind(file);/*rewind file*/

    do
    {
        temp = fgetc(file);

        if (temp == '\n')
            lineNb++;
    }
    while(temp != EOF);

    if(lastChar != '\n')
        lineNb++;

    /*restores cursor position*/
    fseek(file, start, SEEK_SET);

    return (lineNb);
}


bool CEV_fileGotoLine(int line, FILE* file)
{/*goes to line, return true if reached*/

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
{
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
{
    int temp = '*';

    while((temp!='\n') && (temp!=EOF))
        temp = fgetc(file);

    if(temp == EOF)
        return false;

    return true;
}


size_t CEV_fileSize(FILE* file)
{/*size of a file as byte*/

    long pos = ftell(file),
         size = 0;

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    fseek(file, pos, SEEK_SET);

    return(size);
}


bool CEV_fileFileNameGet(const char* src, char* dst)
{
    int len = strlen(src);

    for(int i = len-1; i>=0; i--)
    {
      if(src[i] == '/' || src[i]=='\\')
      {
        strcpy(dst, &src[i+1]);
        return true;
      }
    }

    return false;
}


bool CEV_fileFolderNameGet(const char *src, char *dst)
{
    bool funcSts = false;
    int srcLen   = strlen(src),
        i;

    for (i = srcLen; i>=0; i--)
    {
        if (src[i] =='/' || src[i]=='\\')
        {
            funcSts = true;
            break;
        }
    }

    if (funcSts)
    {
        dst[i+1] ='\0';
        for(int j = i; j>=0; j--)
            dst[j]= src[j];
    }

    return funcSts;
}


void CEV_stringEndFormat(char* in)
{/*replace CR/LF with nul char*/

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
{/*groupement de chiffres*/

    char *srcPtr, *dstPtr;       //nos pointeurs source et destination

    int length = strlen(src),//taille de src pour savoir trouver la fin
        count = 0;             //on va compter par group

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
