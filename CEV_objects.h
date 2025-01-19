#ifndef CEV_OBJECTS_H_INCLUDED
#define CEV_OBJECTS_H_INCLUDED

#include "CEV_platform.h"
#include "CEV_button.h"

#define OBJECT_TYPE_ID 0x12000000
#define IS_OBJECT(x) ((x & 0xFF000000) == OBJECT_TYPE_ID)

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


/** \brief Saves object accordingly to sub type
 *
 * \param src : CEV_Object* to save.
 * \param fileName : char as path and file name to save as.
 *
 * \return int of std function status.
 *
 */
int CEV_objectSave(CEV_Object* src, char* fileName);


/** \brief Destroys object and content.
 *
 * \param this : CEV_Object* to destroy.
 *
 * \return void.
 */
void CEV_objectDestroy(CEV_Object* this);


/** \brief Updates any object (sts + display)
 *
 * \param this CEV_Object*
 * \return void
 *
 */
void CEV_objectUpdate(CEV_Object* this, CEV_Camera* cam, uint32_t now);


/** \brief Converts editable text file into data file.
 *
 * \param srcName : const char* as path and file name of source file.
 * \param dstName : const char* as path and file name of resulting file.
 *
 * \return int of standard function status.
 *
 * \note file extension should be ".xxx" to ensure recognition on auto-load.
 */
int CEV_objectConvertToData(const char* srcName, const char* dstName);


/** \brief Writes to data file from CEV_Text.
 *
 * \param src : CEV_Text* build from txt file.
 * \param dst : FILE* as destination file.
 * \param srcName : char* as name of file of src.
 *
 * \return int of standard function status.
 *
 * \note src & dst are not freed in this function.
 */
int CEV_objectConvertTxtToDataFile(CEV_Text *src, FILE *dst, const char* srcName);

#endif // CEV_OBJECTS_H_INCLUDED
