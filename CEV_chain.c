//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |  05-2016      |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**   CEV    |  11-2017      |   1.0.1  | diag improved  **/
//**********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "CEV_Chain.h"

static CEV_ChainLink * L_seekForward(CEV_ChainHandle* handle, unsigned int index);
static CEV_ChainLink * L_seekBackward(CEV_ChainHandle* handle, unsigned int index);


CEV_ChainHandle *CEV_chainCreate(void)
{//creates chain handler

    /*---DECLARATIONS---*/

    CEV_ChainHandle* newHandle = NULL;

    /*---EXECUTION---*/

    newHandle = malloc(sizeof(*newHandle));

    if (newHandle == NULL)  //on error
        fprintf(stderr, "Err at %s / %d : unable to allocate newHandle : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
    else
    {//reseting fields
        newHandle->first  = NULL;
        newHandle->last   = NULL;
        newHandle->count  = 0;
    }

    /*---POST---*/

    return(newHandle);
}


void *CEV_chainGetDataIndex(CEV_ChainHandle* handle, unsigned int index)
{//gets data field of the Nth link

    return CEV_chainGetIndex(handle, index)->data;
}


CEV_ChainLink *CEV_chainGetIndex(CEV_ChainHandle* handle, unsigned int index)
{//returns the Nth link by shortest way

    CEV_ChainLink *actual;

    if(   (handle == NULL)          //NULL handle
       || (handle->first == NULL)     //unexisting links
       || (index>=handle->count))   //index out of range
    {//set error
        fprintf(stderr, "Err at %s / %d : Arg Error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    if (index <= (handle->count-1)/2)
        actual = L_seekForward(handle, index);
    else
        actual = L_seekBackward(handle, index);

    return actual;
}


CEV_ChainLink* CEV_chainInsert(CEV_ChainHandle* handle, void* data, unsigned int index)
{//creates new inserted link

    /*---DECLARATIONS---*/

    CEV_ChainLink *newLink = NULL,
                  *actual  = handle->first;

    /*---PRL---*/

    if(   (handle == NULL)          //NULL handle
       || (index>=handle->count))   //index out of range
    {/*set error*/
        fprintf(stderr, "Err at %s / %d : Arg Error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    /*---EXECUTION---*/

    if (!actual)
    {//if chain is empty, just append...
        newLink = CEV_chainAppend(handle, data);

        if(!newLink)
            fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
    }
    else if (!index)
    {//if not index, just prepend
        newLink = CEV_chainPrepend(handle, data);

        if(!newLink)
            fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
    }
    else
    {
        actual = CEV_chainGetIndex(handle, index);

        newLink = malloc(sizeof(*newLink));

        if(!newLink)
        {
            fprintf(stderr, "Err at %s / %d : %s.\n", __FUNCTION__, __LINE__, strerror(errno));
            return NULL;
        }

        newLink->prev       = actual->prev;
        actual->prev->next  = newLink;
        actual->prev        = newLink;
        newLink->next       = actual;
        newLink->data       = data;
        handle->count++;
    }

    return newLink;
}


CEV_ChainLink* CEV_chainAppend(CEV_ChainHandle* handle, void* data)
{//creates and append new link

    /*---DECLARATIONS---*/

    CEV_ChainLink *newLink = NULL;

    /*---EXECUTION---*/

    newLink = malloc(sizeof(*newLink));

    if (newLink != NULL)
    {//alloc ok
        /**new link treatment**/
        newLink->next = NULL;           //last link, no next
        newLink->prev = handle->last;   //old last becomes new's previous
        newLink->data = data;           //content is data

        /**master update**/
        if (handle->first == NULL)      //if chain's empty
            handle->first = newLink;    //become first

        if (handle->last != NULL)           //if any last
            handle->last->next = newLink;   //new become the next of the old last

        handle->last = newLink; //new becomes last

        handle->count++;
    }
    else//alloc err
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));

    return (newLink);
}


CEV_ChainLink* CEV_chainPrepend(CEV_ChainHandle* handle, void *data)
{//creates and append new link

    /*---DECLARATIONS---*/

    CEV_ChainLink *newLink = NULL;

    /*---EXECUTION---*/

    newLink = malloc(sizeof(*newLink));

    if (newLink != NULL)
    {//if alloc ok
        /**new link treatment**/
        newLink->next = handle->first;  //first link becomes new's next
        newLink->prev = NULL;           //none previous
        newLink->data = data;           //content is data

        /**master update**/
        handle->first = newLink;    //becomes first

        if (newLink->next)                  //if has next
            newLink->next->prev = newLink;  //new becomes its next's previous

        if (handle->last == NULL)           //if no last
            handle->last = newLink;         //new becomes last


        handle->count++;
    }
    else//err alloc
        fprintf(stderr, "Err at %s / %d : %s\n", __FUNCTION__, __LINE__, strerror(errno));

    return (newLink);
}


void CEV_chainRemoveIndex(CEV_ChainHandle* handle, unsigned int index, char freeData)
{//removes link Nth

    CEV_chainRemove(handle, CEV_chainGetIndex(handle, index), freeData);
}


void CEV_chainRemove(CEV_ChainHandle* handle, CEV_ChainLink* link, char freeData)
{//destroy link from chain

    /*---EXECUTION---*/

    if (link == handle->first)          //if first link
    {
        if(link->next != NULL)          //if not last
            link->next->prev = NULL;    //next has no previous

        handle->first = link->next;     //link's next becomes first
    }
    else
        link->prev->next = link->next;  //links previous to next

    if (link == handle->last)   //if last link
    {
        handle->last = link->prev;  //previous becomes last

        if (link->prev != NULL)     //if not first
            link->prev->next = NULL;// previous' next is NULL
    }
    else
        link->next->prev = link->prev; //links next to previous

    /*now off the chain, can be destroyed*/

    if (freeData)
        free(link->data);

    free(link);

    handle->count--;
}


void CEV_chainDestroy(CEV_ChainHandle* handle, char freeData)
{//destroy everything

    /*---DECLARATIONS---*/

    CEV_ChainLink *actual   = NULL, //temp
                  *memo     = NULL;

    /*---EXECUTION---*/

    if (handle != NULL)
    {//if handle

        actual = handle->first;

        while (actual)
        {//scans & destroy
            if(freeData)
                free(actual->data);

            memo = actual;
            actual = actual->next;

            free(memo);
        }

        free (handle);
    }
}


static CEV_ChainLink* L_seekForward(CEV_ChainHandle* handle, unsigned int index)
{//scans forward

    /*---DECLARATIONS---*/

    CEV_ChainLink *actual = handle->first;  //get first link

    /*---EXECUTION---*/

    while (index)
    {//goin thru chain
        actual = actual->next;
        index--;
    }

    return actual;
}


static CEV_ChainLink* L_seekBackward(CEV_ChainHandle* handle, unsigned int index)
{//scans backward

    /*---DECLARATIONS---*/
    CEV_ChainLink   *actual = handle->last;//get first link
    unsigned int    loc     = handle->count - 1 - index;

    /*---EXECUTION---*/

    while (loc)
    {//goin thru chain
        actual = actual->prev;
        loc--;
    }

    return actual;
}
