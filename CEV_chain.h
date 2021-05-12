//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  15-04-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**********************************************************/

#ifndef CEV_CHAIN_H_INCLUDED
#define CEV_CHAIN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif



typedef struct CEV_ChainLink CEV_ChainLink;


/** \brief data link
 */
struct CEV_ChainLink
{/**chain element */

    void* data; /*instance pointer*/

    CEV_ChainLink *prev,  /*previous link*/
                  *next;  /*next link*/
};


/** \brief chain handle
 */
typedef struct CEV_ChainHandle
{/**chain handler */

    unsigned int count;

    CEV_ChainLink *first, /*first link*/
                  *last;  /*last link*/
}
CEV_ChainHandle;



/** \brief creates chain handle.
 *
 * \return CEV_ChainHandle* on success, NULL on error.
 *
 */
CEV_ChainHandle* CEV_chainCreate(void);


/** \brief gets the Nth data
 *
 * \param handle : main handle.
 * \param index : index to be reached.
 *
 * \return void* on data held by the indexth link, NULL on error.
 */
void *CEV_chainGetDataIndex(CEV_ChainHandle* handle, unsigned int index);


/** \brief gets the Nth link
 *
 * \param handle : main handle.
 * \param index : index to be reached.
 *
 * \return CEV_ChainLink* on success, NULL on error.
 */
CEV_ChainLink *CEV_chainGetIndex(CEV_ChainHandle* handle, unsigned int index);


/** \brief creates and insert a new link
 *
 * \param handle : main handle.
 * \param data : data to be held by this link.
 * \param index : position to insert it at.
 *
 * \return CEV_ChainLink* on success, NULL on error.
 */
CEV_ChainLink* CEV_chainInsert(CEV_ChainHandle* handle, void* data, unsigned int index);


/** \brief creates and appends new link
 *
 * \param handle : main handle.
 * \param data : data to be held by this link.
 *
 * \return CEV_ChainLink* on success, NULL on error
 */
CEV_ChainLink* CEV_chainAppend(CEV_ChainHandle* handle, void* data);


/** \brief creates and preppends new link
 *
 * \param handle : main handle.
 * \param data : data to be held by this link.
 *
 * \return CEV_ChainLink* on success, NULL on error
 */
CEV_ChainLink* CEV_chainPrepend(CEV_ChainHandle* handle, void* data);


/** \brief destroys indexth link
 *
 * \param handle : main handle.
 * \param index : index of link to be destroyed.
 * \param freeData : let the function free data field if not 0.
 *
 * \return N/A.
 */
void CEV_chainRemoveIndex(CEV_ChainHandle* handle, unsigned int index, char freeData);


/** \brief destroys known link
 *
 * \param handle : main handle.
 * \param link : thelink to be destroyed.
 * \param freeData : let the function free data field if not 0.
 *
 * \return N/A.
 */
void CEV_chainRemove(CEV_ChainHandle* handle, CEV_ChainLink* link, char freeData);


/** \brief destroys full chain
 *
 * \param handle : main handle.
 * \param freeData : let the function free data field if not 0.
 *
 * \return N/A.
 */
void CEV_chainDestroy(CEV_ChainHandle* handle, char freeData);


#ifdef __cplusplus
}
#endif

#endif /* CEV_CHAIN_H_INCLUDED */
