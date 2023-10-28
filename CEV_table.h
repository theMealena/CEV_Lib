//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   0.0    |    creation    **/
//**   CEV    |    05-2016    |   0.1    |    SDL2 rev    **/
//**   CEV    |  15-02-2017   |   1.0    |   rev & test   **/
//**********************************************************/
#ifndef TABLE_MANAGER_H_INCLUDED
#define TABLE_MANAGER_H_INCLUDED

#include <stdlib.h>

#define DYNA_IS_EMPTY(x) (x.inUse == 0)

#ifdef __cplusplus
extern "C" {
#endif

/** \brief dynamic array structure.
 */
typedef struct CEV_DynamicArray
{
    unsigned int inUse,     /**< number of value in use*/
                 capacity;  /**< number of value it can store*/

    size_t elemSize;    /**< byte length of one value*/
    void *data;         /**< the data array itself*/
}
CEV_DynamicArray;


/** \brief Extend table capacity by 1 element.
 *
 * \param table : the table to extend.
 *
 * \return one of the functions status.
 *
 * \sa CEV_tabSizeDouble()
 */
int CEV_tabSizeExtend(CEV_DynamicArray *table);



/** \brief doubles table capacity.
 *
 * \param table : the table to extend.
 *
 * \return one of the functions status.
 *
 * \sa CEV_tabSizeExtend()
 */
int CEV_tabSizeDouble(CEV_DynamicArray *table);


/** \brief append value in table.
 *
 * \param table : table in which to add value.
 * \param value : memory field to be added in table.
 *
 * \return one of the functions status.
 *
 * \note value content is copied and can be freed.
    Will extend table if necessary.
 */
int CEV_tabAddValue(CEV_DynamicArray *table, void *value);



/** \brief append value in table.
 *
 * \param table : table in which to add value.
 * \param value : memory field to be added in table.
 * \param index : array index in wich to copy value.
 *
 * \return index at which value was written.
 *
 * \note value content is copied and can be freed.
    Value is append if index is too high.
 */
int CEV_tabIndexSet(CEV_DynamicArray *table, void *value, unsigned int index);


/** \brief Deactivate an element of the table.
 *
 * \param table : the table from which to remove index.
 * \param index : index to be removed.
 *
 * \return N/A.
 *
 * \note Nothing done if index is out of range.
 */
void CEV_tabRemoveIndex(CEV_DynamicArray *table, unsigned int index);



/** \brief Initialize table content.
 *
 * \param table : CEV_DynamicArray* to initialize.
 * \param num : number of element to be allocated.
 * \param size : size of an element of the table.
 *
 * \return one of the functions status.
 */
int CEV_tabInit(CEV_DynamicArray *table, unsigned int num, size_t size);



/** \brief Free table content and itself / NULL.
 *
 * \param table : CEV_DynamicArray* to free.
 *
 * \return N/A.
 */
void CEV_tabFree(CEV_DynamicArray *table);


/** \brief Free table content an reset 0 / NULL.
 *
 * \param table : CEV_DynamicArray* to clear.
 *
 * \return N/A.
 */
void CEV_tabClear(CEV_DynamicArray *table);



/** \brief fetches table content.
 *
 * \param table : CEV_DynamicArray* to fetch from.
 * \param index : index of the element to fetch.
 *
 * \return void* pointer on element requested beeing table[index].
 */
void *CEV_tabIndexGet(CEV_DynamicArray *table, unsigned int index);


/** \brief fetches free data slot.
 *
 * \param table : CEV_DynamicArray* to fetch free slot from.
 *
 * \return pointer on free slot or NULL on error.
 */
void* CEV_tabFreeSlotGet(CEV_DynamicArray *table);


/** \brief tableSize
 *
 * \param table : CEV_DynamicArray* to query size from.
 *
 * \return act in use of table.
 */
unsigned int CEV_tabSizeGet(CEV_DynamicArray *table);


#ifdef __cplusplus
}
#endif

#endif /* TABLE_MANAGER_H_INCLUDED */
