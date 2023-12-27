#ifndef CEV_OBJECTS_H_INCLUDED
#define CEV_OBJECTS_H_INCLUDED

#include "CEV_platform.h"
#include "CEV_button.h"

#define OBJECT_TYPE_ID 0x12000000
#define IS_OBJECT (x) ((x & 0xFF000000) == OBJECT_TYPE_ID)

typedef union CEV_Object
{
    uint32_t id;
    CEV_Platform platform;
    CEV_SwitchButton button;
}
CEV_Object;



/** \brief Loads object from RWops.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool to close RWops if true.
 *
 * \return CEV_Object* on success, NULL on failure.
 * \note If requested src is closed weither this function succeed or not.
 */
CEV_Object* CEV_objectLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Destroys object and content.
 *
 * \param this : CEV_Object* to destroy.
 *
 * \return void.
 */
void CEV_objectDestroy(CEV_Object* this);

#endif // CEV_OBJECTS_H_INCLUDED
