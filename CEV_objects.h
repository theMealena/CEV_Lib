#ifndef CEV_OBJECTS_H_INCLUDED
#define CEV_OBJECTS_H_INCLUDED

#include "CEV_platform.h"
#include "CEV_button.h"

typedef union CEV_Object
{
    uint32_t ID;
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
 *  If requested src is closed weither this function succeed or not.
 */
CEV_Object* CEV_objectLoad_RW(SDL_RWops* src, bool freeSrc);

#endif // CEV_OBJECTS_H_INCLUDED
