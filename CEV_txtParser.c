//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  03-03-2023   |   1.0    |    creation    **/
//**********************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <CEV_api.h>
#include "CEV_txtParser.h"


//gets value after a '=' char
/** \brief Fetches value in string.
 *
 * \param in : char* to extract value from.
 *
 * \return double as value extracted, 0.0 is default value on error.
 *  Value must be placed after '=' char.
 */
double L_valueFromString(char* in);


//reads multiple values in single line with
/** \brief Fetches multiple values from string
 *
 * \param src : char* from which values must be extracted.
 * \param dst : double array that will be filled with read values.
 * \param maxNum : int as maximum values to read (dst[] size.
 *
 * \return int as num of values that could be read.
 *
 *  Values shall be separated by the ';' char.
 */
int L_valueArrayFromString(char* src, double *dst, int maxNum);


//find parameter's string
/** \brief Find line of parameter "name".
 *
 * \param src : CEV_Text* with parameters set.
 * \param name : char* as parameter's name to be found.
 *
 * \return char* as begin of string, NULL on error or inexistant.
 */
char* L_txtSeek(CEV_Text *src, char* name);


//true if "seek" is found in "in" //start of strings only
/** \brief Check if in starts with seek
 *
 * \param seek : char* to be found.
 * \param in : char* to seek into.
 *
 * \return bool as true if seek is found in in.
 */
bool L_txtMatch(char* seek, char* in);


void TEST_txtParse(void)
{
    CEV_Text *txt = CEV_textTxtLoad("txtParse/text.txt");

    CEV_textDump(txt);

    double dDst[10] = {0};

    printf("toto is : %f\n", CEV_txtParseValueFrom(txt, "toto"));
    printf("tata is : %f\n", CEV_txtParseValueFrom(txt, "tata"));
    printf("hex is : %08X\n", CEV_txtParseHex32From(txt, "hex"));
    printf("hex holds : %s\n", CEV_txtParseTxtFrom(txt, "hex"));
    printf("name is :_%s\n", CEV_txtParseTxtFrom(txt, "name"));

    int num = CEV_txtParseValueArrayFrom(txt, "list", dDst, 10);

    printf("read %d values with : \n", num);

    for(int i=0; i<num; i++)
        printf("value %d is : %f\n", i, dDst[i]);

    for(int i=0; i<2; i++)
    {
        char lTxt[50];
        sprintf(lTxt, "param[%02d]", i);
        printf("param %d is : %s\n", i, CEV_txtParseTxtFrom(txt, lTxt));
    }

    CEV_textSave(txt, "txtParse/text.dtxt");

    CEV_textDestroy(txt);
}



double CEV_txtParseValueFrom(CEV_Text* src, char* name)
{
    return L_valueFromString(L_txtSeek(src, name));
}


uint32_t CEV_txtParseHex32From(CEV_Text* src, char* name)
{
    uint32_t result = 0x0;

    if(IS_NULL(src) || IS_NULL(name))
        goto end;

    char* ptr =  L_txtSeek(src, name);

    if(IS_NULL(ptr))
        goto end;

    //going after '=' or end of ptr
    while(*ptr++ != '=' && *ptr!='\0');

    while(!isalnum(*ptr) && *ptr!='\0')
        ptr++;

    sscanf(ptr, "%x", &result);

end:
    return result;
}


char* CEV_txtParseTxtFrom(CEV_Text* src, char* name)
{
    if(IS_NULL(src) || IS_NULL(name))
        return NULL;

    char* ptr =  L_txtSeek(src, name);

    if(IS_NULL(ptr))
        return NULL;

    //going after '=' or end of ptr
    while(*ptr++ != '=' && *ptr!='\0');

    while(!isalnum(*ptr) && *ptr!='\0')
        ptr++;

    return ptr;
}


int CEV_txtParseValueArrayFrom(CEV_Text *src, char* name, double* dst, int maxNum)
{
    return L_valueArrayFromString(L_txtSeek(src, name), dst, maxNum);
}


int CEV_txtParseIndexGetFrom(CEV_Text *src, char* name)
{
    for(unsigned i = 0; i<src->linesNum; i++)
    {
        char* line = src->line[i];

        if(L_txtMatch(name, line))
        {
            return i;
        }
    }

    return 0;
}


char* L_txtSeek(CEV_Text *src, char* name)
{
    for(unsigned i = 0; i<src->linesNum; i++)
    {
        char* line = src->line[i];

        if(L_txtMatch(name, line))
        {
            return line;
        }
    }

    return NULL;
}


double L_valueFromString(char* in)
{
    if(IS_NULL(in))
        return 0.0;

    char* ptr = in;

    //going after '=' or end of ptr
    while(*ptr++ != '=' && *ptr != '\0');

    double result = atof(ptr);

    return result;
}


int L_valueArrayFromString(char* src, double *dst, int maxNum)
{
    if(IS_NULL(src) || IS_NULL(dst) || !maxNum)
        return 0;

    int numRead = 0;
    char *ptr = src;

    //going to first value
    while(*ptr++ != '=' && *ptr != '\0');

    while(*ptr != '\0')
    {
        dst[numRead++] = atof(ptr);

        if(numRead >= maxNum)
        {
            return numRead;
        }

        while(*ptr++!=';' && *ptr!='\0');
    }

    return numRead;
}


bool L_txtMatch(char* seek, char* in)
{
    if(IS_NULL(seek) || IS_NULL(in))
        return false;

    unsigned i = 0;

    while(seek[i] == in[i])
        i++;

    return (i == strlen(seek));
}

