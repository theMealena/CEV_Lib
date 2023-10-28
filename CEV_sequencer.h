//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  04-2020      |   1.0    |    creation    **/
//**   CEV    |  03-2023      |   1.1    | CEV_lib style  **/
//**********************************************************/

//log
//  2023-03 - CEV :
//      seqFree() renamed seqDestroy().
//      Doxy comments added.


/** \file   CEV_sequencer.h
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

#ifndef CEV_SEQ_H_INCLUDED
#define CEV_SEQ_H_INCLUDED


#include <stdbool.h>



/** \brief
 *  \enum SeqAttribute.
 */
enum SeqAttribute
{
    SEQ_RE      = 0,    /**< State change Rising Edge functions table index */
    SEQ_ACTIVE  = 1,    /**< State active functions table index */
    SEQ_TEST    = 2,    /**< State test functions table index */
    SEQ_FE      = 3,    /**< State change Falling Edge functions table index */
    SEQ_TYPE_LAST       /**< Max type value, as internal, used for loops count */
};


/** \brief Sequencer main structure
 * \struct CEV_Sequencer
 */
typedef struct CEV_Sequencer
{
    int stsPrev,    /**< previous state / used to compute new state */
        stsAct,     /**< actual state */
        numOfState; /**< num of state in this sequencer*/

    bool (****execute)(void*); /**< 3D matrix of function ptr */

    //bool(*execute[SEQ_TYPE_LAST][NUM_OF_STATUS][NUM_OF_STATUS])(void*);//functions pointers matrix
}
CEV_Sequencer;


/** \brief Creates new sequencer.
 *
 * \param numOfState : number of status in sequencer.
 *
 * \return New sequencer on success or NULL.
 */
CEV_Sequencer *CEV_seqCreate(unsigned int numOfState);


/** \brief Frees sequencer content and itself.
 *
 * \param seq : sequencer to free.
 *
 * \return N/A.
 */
void CEV_seqDestroy(CEV_Sequencer *seq);


/** \brief Clears sequencer content.
 *
 * \param seq : CEV_Sequencer* to clear.
 *
 * \return N/A.
 * \note Sets functions ptr to NULL but DOES NOT free allocated matrix.
 */
void CEV_seqClear(CEV_Sequencer *seq);


/** \brief Gets sequencer actual State.
 *
 * \param seq : CEV_Sequencer* to fetch state from.
 *
 * \return int : sequencer's actual state value.
 */
int CEV_seqStsGet(CEV_Sequencer *seq);


/** \brief Sets sequencer actual State.
 *
 * \param seg : CEV_Sequencer* which state is to be set.
 * \param num : state value to be set.
 *
 * \return sequencer's state, -1 if num is out of range.
 *
 * \note Next call to CEV_seqUpdate() will behave as if it was a computed state change.
 */
int CEV_seqStsSet (CEV_Sequencer *seq, int num);


/** \brief Updates sequencer.
 *
 * \param seq : CEV_Sequencer* to update.
 * \param in : void* any management structure used as parameter in matrix functions call.
 *
 * \return N/A.
 */
void CEV_seqUpdate(CEV_Sequencer *seq, void *in);


#endif // CEV_SEQ_H_INCLUDED
