//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |    11-2017    |   1.0.1  | diag improved  **/
//**********************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "project_def.h"
#include "CEV_table.h"
#include "CEV_api.h"



int CEV_tabSizeExtend(CEV_DynamicArray *table)
{/*extend array by 1*/

    void* tempPtr = table->data;

    tempPtr = realloc(table->data, (table->capacity+1)*table->elemSize);   /*new size*/

    if (tempPtr == NULL)
    {/*on error*/
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return(FUNC_ERR);
    }
    /*else*/
    table->capacity++;  /*1 more space available*/
    table->data = tempPtr;

    return(FUNC_OK);
}


int CEV_tabSizeDouble(CEV_DynamicArray *table)
{/*extend array by twice*/

    void* tempPtr = table->data;

    tempPtr = realloc(table->data, 2*table->capacity * table->elemSize);   /*twice bigger*/

    if (tempPtr == NULL)
    {/*si echec de realloc*/
        fprintf(stderr, "Err at %s / %d : unable to allocate : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
        return(FUNC_ERR);
    }
    /*else*/
    table->data = tempPtr;
    table->capacity *= 2;   /*twice the space available*/

    return(FUNC_OK);
}


int CEV_tabAddValue(CEV_DynamicArray *table, void* value)
{/*add a value in the table*/

    int  sts  = FUNC_OK;/*enum FUNC_OK =0*/

    if (table->inUse + 1 > table->capacity)
        sts += CEV_tabSizeExtend(table);

    if (!sts)
    {
        /*no NULL can occur, we just extendend the table if empty*/
        memcpy(CEV_tabIndexGet(table, table->inUse), value, table->elemSize);
        table->inUse++;
    }

    return sts;
}


void CEV_tabRemoveIndex(CEV_DynamicArray *table, unsigned int index)
{/*copies last value into index*/

    if(index >= table->inUse) /*index off limits*/
        return;

    else if(table->inUse>=2) /*at least 2 values to copy*/
    {
        char  *temp = table->data, /*ptr to array*/
              *src  = temp + ((table->inUse-1) * table->elemSize),  /*ptr to last value*/
              *dst  = temp + (index * table->elemSize);   /*ptr to the one to remove*/

        memcpy(dst, src, table->elemSize);  /*copy last one into the one to be removed*/
    }

    table->inUse--; /*one less in use*/
}


int CEV_tabInit(CEV_DynamicArray *table, unsigned int num, size_t size)
{/*structure init*/

    table->inUse    = 0;
    table->capacity = 0;
    table->elemSize = size;

    if(num)
        table->data = calloc(size, num);
    else
        table->data = NULL;

    if (table->data == NULL)
        return(FUNC_ERR);
    /*else*/
        table->capacity = num;

    return FUNC_OK;
}


void CEV_tabFree(CEV_DynamicArray *table)
{/*destroy everything*/

    free(table->data);
    free(table);
}


void CEV_tabClear(CEV_DynamicArray *table)
{/*destroy content*/

    free(table->data);
    table->data     = NULL;
    table->capacity = 0;
    table->elemSize = 0;
    table->inUse    = 0;
}


void *CEV_tabIndexGet(CEV_DynamicArray *table, unsigned int index)
{
    char *ptr = (char*) table->data;

    /*checkin index is available*/
    if (index > table->inUse)
    {
        fprintf(stderr, "Err at %s / %d : index is higher than allowed : %d.\n", __FUNCTION__, __LINE__, table->inUse-1 );
        return NULL;
    }

    ptr += table->elemSize * index;

    return (void*)ptr;
}


void* CEV_tabFreeSlotGet(CEV_DynamicArray *table)
{
    char *ptr = NULL;

    if (table->inUse < table->capacity)
        ptr = (char*)table->data + table->elemSize * table->inUse;

    return (void*)ptr;
}


unsigned int CEV_tabSizeGet(CEV_DynamicArray *table)
{
    return table->inUse;
}
