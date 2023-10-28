//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  04-2020      |   1.0    |    creation    **/
//**   CEV    |  03-2023      |   1.1    | CEV_lib style  **/
//**********************************************************/

//log
//  2023-03 - CEV :
//  seqFree() renamed as seqDestroy(); Memory leak solved.
//  Doxy comments added.

/** \file   CEV_sequencer.c
 * \author  CEV
 * \version 1.1
 * \date    March 2023
 * \brief   Sequencer creation and handling.
 *
 * \details Originaly extracted from stringman project.
 *  Modified / adapted to fit general purpose.
 *
 * attributing functions to the sequencer :
 * with myFunction(void* in);
 *
 * mySquencer->execute[TYPE][ACT_STATE][TO/FROM] = myFunction;
 *
 *[SEQ_RE][ACT STATE][PREV STATE] : [executed when arriving] in [ACT STATE] from [PREV STATE]\n
 *[SEQ_ACTIVE][ACT STATE][ACT STATE] : [executed while] [ACT STATE] is active\n
 *[SEQ_TEST][ACT STATE][NEXT STATE] : [test condition] from [ACT STATE] to [NEXT STATE]\n
 *[SEQ_FE][ACT STATE][NEXT STATE] : [executed when leaving] [ACT STATE] to [NEXT STATE]\n
 *\n
 *the SEQ_RE and SEQ_ACTIVE functions return values are ignored.\n
 *the SEQ_TEST function must return true to enable transition from act to next state.\n
 *\n
 *if the SEQ_FE function returns false,
 *    the next status is instantly executed,
 *    thus to be able to reach some stable state.
 *    On true, the function is close and actions left to next call.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <project_def.h>
#include <CEV_api.h>
#include <CEV_display.h>
#include "CEV_sequencer.h"


#define SEQ_DEBUG 0


CEV_Sequencer *CEV_seqCreate(unsigned int numOfState)
{//new sequencer

    CEV_Sequencer *result = calloc(1, sizeof(*result));
    int iErr = 0,
        jErr = 0;

    if(IS_NULL(result))
        return NULL;


    result->execute = calloc(SEQ_TYPE_LAST, sizeof(*result->execute));

    for(int i=0; i<SEQ_TYPE_LAST; i++)
    {
        iErr = i;
        result->execute[i] = calloc(numOfState, sizeof(*result->execute[i]));

        if(IS_NULL(result->execute[i]))
        {
            fprintf(stderr, "Err at %s / %d : Unable to alloc : %s.\n ",__FUNCTION__,  __LINE__, strerror(errno));
            goto err_i;
        }

        for(unsigned j=0; j<numOfState; j++)
        {
            jErr = j;

            result->execute[i][j] = calloc(numOfState, sizeof(*(result->execute[i][j])));

            if(IS_NULL(result->execute[i][j]))
        {
            fprintf(stderr, "Err at %s / %d : Unable to alloc : %s.\n ",__FUNCTION__,  __LINE__, strerror(errno));
            goto err_j;
        }

        }
    }

    result->stsAct = 0;
    result->stsPrev = 0;
    result->numOfState = numOfState;

    return result;

err_j :

    for(int i = jErr-1; i>=0; i--)
        free(result->execute[iErr][i]);

err_i :

    for(int i = iErr-1; i>=0; i--)
        for(unsigned j=0; j<numOfState; j++)
            free(result->execute[i][j]);

    return NULL;
}


void CEV_seqDestroy(CEV_Sequencer *seq)
{//free sequencer

    if(IS_NULL(seq))
        return;

    for(int i=0; i<SEQ_TYPE_LAST; i++)
    {
        for(int j=0; j <seq->numOfState; j++)
        {
            free(seq->execute[i][j]);
        }

        free(seq->execute[i]);
    }

    free(seq->execute);//was missing : added 03/2023

    free(seq);
}


void CEV_seqClear(CEV_Sequencer *seq)
{//clear
    seq->stsPrev = seq->stsAct = 0;

    for (int type=0; type< SEQ_TYPE_LAST; type++)
    {
        for(int x=0; x<seq->numOfState; x++)
        {
            for(int y=0; y<seq->numOfState; y++)
            {
                seq->execute[type][x][y] = NULL;
            }
        }
    }
}


int CEV_seqStsGet(CEV_Sequencer *seq)
{//actual state

    return seq->stsAct;
}


int CEV_seqStsSet(CEV_Sequencer *seq, int num)
{//forcing state

    if(num < seq->numOfState)
    {
        seq->stsAct = num;
        return seq->stsAct;
    }
    //else
    return -1;
}


//stops loop if FE function returns true
void CEV_seqUpdate(CEV_Sequencer *seq, void *in)
{//updating sequencer

    int stsNext, loop = 0;
    bool stopLoop = false;

    do
    {
        loop++; //safety count to escape infinite loop

        //status RE execution
        if (seq->stsAct != seq->stsPrev)
        {//status has changed since last scan

            //executing arriving function if any
            if(seq->execute[SEQ_RE][seq->stsAct][seq->stsPrev] != NULL)
            {
                seq->execute[SEQ_RE][seq->stsAct][seq->stsPrev](in);

                #if SEQ_DEBUG
                    printf("seq re executed : %d from %d : %d\n", seq->stsAct, seq->stsPrev, loop);

                #endif // DEBUG
            }

            seq->stsPrev = seq->stsAct;
        }

        //active state execution
        if(seq->execute[SEQ_ACTIVE][seq->stsAct][seq->stsAct] != NULL)
            seq->execute[SEQ_ACTIVE][seq->stsAct][seq->stsAct](in);

        #if SEQ_DEBUG
            printf("seq active executed : %s : %d \n", status[seq->stsAct], loop);

        #endif // DEBUG

        //stsNext = seq->stsAct;

        //seeking any true condition to another status
        for(int i=0; i<seq->numOfState; i++)
        {
            if((seq->execute[SEQ_TEST][seq->stsAct][i] != NULL) && (seq->execute[SEQ_TEST][seq->stsAct][i](in)))
            {
                stsNext = i;

                #if SEQ_DEBUG
                    printf("seq find trans from %d to %d : %d \n", seq->stsAct, stsNext, loop);
                #endif // DEBUG

                //executing leaving function
                if(seq->execute[SEQ_FE][seq->stsAct][stsNext] != NULL)
                {
                    stopLoop = seq->execute[SEQ_FE][seq->stsAct][stsNext](in);//true stops the do loop

                    #if SEQ_DEBUG
                        printf("seq fe executed : %d to %d : %d\n", seq->stsAct, stsNext, loop);
                    #endif // DEBUG
                }
                else
                    stopLoop = true;//nothing to do when leaving state, quitting function.

                //record for next loop / next call
                seq->stsPrev = seq->stsAct;
                seq->stsAct  = stsNext;

                break;
            }
            else
                stopLoop = true;
        }

        #if SEQ_DEBUG
            printf("%s with Status : %d : %d\n",stopLoop? "leaving":"loops again", seq->stsAct, loop);
        #endif // DEBUG

        if(loop>100)
        {
            CEV_dispWarning("erreur programme", "Boucle sequenceur, le programme va fermer");
            exit(EXIT_FAILURE);
        }

    }while((seq->stsAct != seq->stsPrev) && !stopLoop);


}

