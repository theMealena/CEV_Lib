#ifndef CEV_BMPFONT_H_INCLUDED
#define CEV_BMPFONT_H_INCLUDED

#include <SDL.h>

#define CEV_BMPFONT_MAX_DIGIT   16  /**< Maximum digit displayable for numbers*/
#define CEV_BMPFONT_NUM         7   /**< num of font in the main font pic */
#define CEV_BMPFONT_CHAR_NUM    60  /**< num of char in default main font pic */


/*png of font is embedded anyway*/


typedef enum S_CEV_BmpFontSize
{
    CEV_BMPFONT_SMALL   = 8,
    CEV_BMPFONT_MED     = 16,
    CEV_BMPFONT_LARGE   = 32
}
CEV_BmpFontSize;


/** \brief BMP font structure definition.
 */
typedef struct S_CEV_bmpFont
{
    uint32_t id,        /**< own unique id */
             //picId,     /**< src picture id */
             numOfChar; /**< num of available char in bmp font */

    //NOTE (drx#1#05/02/23)keep surface here, stringman intro is based on it
    SDL_Surface* pic;  /**< single line font */

    SDL_Rect    picDim,     /**< font dimension (surface size) */
                charDim;    /**< single font's char size in surface */
}
CEV_BmpFont;


void CEV_BMPFont_Test(void);

/** \brief Dumps structure content to stdout.
 *
 * \param this : CEV_BmpFont* to dump.
 *
 * \return void.
 */
void CEV_bmpFontDump(CEV_BmpFont* this);


/** \brief Creates bmp font from multi font picture.
 *
 * \param fonts : SDL_Surface* as surface holding all fonts.
 * \param which : int as wich font to select.
 * \param size : CEV_BmpFontSize among available values.
 *
 * \return CEV_BmpFont* on success, NULL on error.
 */
CEV_BmpFont* CEV_bmpFontCreateFrom(SDL_Surface* fonts, int which, CEV_BmpFontSize size);


/** \brief Creates Bmp font from single line font surface.
 *
 * \param pic : SDL_Surface* which contains the bmp font.
 * \param numOfChar : uint32_t as num of char held by picture.
 *
 * \return CEV_BmpFont* on success, NULL on failure.
 */
CEV_BmpFont* CEV_bmpFontCreate(SDL_Surface* pic, uint32_t numOfChar);


/** \brief Destroys Bmp font and its content.
 *
 * \param this : CEV_BmpFont* to destroy.
 *
 * \return void
 */
void CEV_bmpFontDestroy(CEV_BmpFont* this);


/** \brief Clears Bmp font content.
 *
 * \param this : CEV_BmpFont* to be cleared.
 *
 * \return void
 *  any SDL_Surface will be destroyed.
 */
void CEV_bmpFontClear(CEV_BmpFont* this);


/** \brief Loads Bmp font from file.
 *
 * \param fileName : const char* as name of file to load from.
 *
 * \return CEV_BmpFont* on success, NULL on failure.
 */
CEV_BmpFont* CEV_bmpFontLoad(const char* fileName);


/** \brief Loads Bmp font from SDL_RWops.
 *
 * \param src : SDL_RWops* to load from.
 * \param freeSrc : bool closes src if true.
 *
 * \return CEV_BmpFont* on success, NULL on failure.
 *
 * \note If requested src is closed weither the function succeeds or not.
 */
CEV_BmpFont* CEV_bmpFontLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Save Bmp into file.
 *
 * \param src : CEV_BmpFont* to be saved.
 * \param fileName : const char* as path and name of resulting file.
 *
 * \return int of std return values.
 *
 * \note File extension should be ".bmf" to help with recognition on auto-loading.
 */
int CEV_bmpFontSave(CEV_BmpFont* src, const char* fileName);


/** \brief Reads Bmp font from file.
 *
 * \param src : FILE* as file to read from.
 * \param dst : CEV_BmpFont* to be filled with file content.
 *
 * \return int of std return values.
 *
 * \note Structure should be cleared as content is not freed before beeing filled.
 */
int CEV_bmpFontTypeRead(FILE* src, CEV_BmpFont* dst);


/** \brief Write Bmp font into file.
 *
 * \param src : CEV_BmpFont* to be written.
 * \param dst : FILE* to write into.
 *
 * \return int of std return values.
 */
int CEV_bmpFontTypeWrite(CEV_BmpFont* src, FILE* dst);


/** \brief Reads Bmp font from RWops.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst CEV_BmpFont* to be filled with file content.
 * \param freeSrc : bool that closed RWops if true.
 *
 * \return int of std return values.
 *
 * \note Structure should be cleared as content is not freed before beeing filled.
 * \note If requested src is closed weither the function succeeds or not.
 */
int CEV_bmpFontTypeRead_RW(SDL_RWops* src, CEV_BmpFont* dst, bool freeSrc);


/** \brief Writes Bmp font into RWops.
 *
 * \param src : CEV_BmpFont* to be written.
 * \param dst : SDL_RWops* to write into.
 *
 * \return int of std return values.
 */
int CEV_bmpFontTypeWrite_RW(CEV_BmpFont* src, SDL_RWops* dst);


/** \brief Attaches surface as new font.
 *
 * \param src : SDL_Surface* to be attached.
 * \param numOfChar : uint32_t as num of character held by the surface.
 * \param dst : CEV_BmpFont* to attach surface to.
 *
 * \return int of std return values.
 *
 * \note Any previously existing surface will be freed upon new attachment.
 */
int CEV_bmpFontSurfaceAttach(SDL_Surface *src, uint32_t numOfChar, CEV_BmpFont *dst);


/** \brief Displays value in font style onto texture.
 *
 * \param src : CEV_BmpFont* as source to be displayed.
 * \param dst : SDL_Texture* as destination to blit onto.
 * \param value : int as value to be displayed.
 * \param pos : SDL_Rect with x,y as topmost leftmost position.
 *
 * \return int of std return values.
 *
 * \note Num of digit is limited by CEV_BMPFONT_MAX_DIGIT
 * \note Texture should has SDL_TEXTUREACCESS_STREAMING property
 * \sa CEV_BMPFONT_MAX_DIGIT
 */
int CEV_bmpFontDispValue(CEV_BmpFont* src, SDL_Texture* dst, int value, SDL_Rect pos);


/** \brief Displays text in font style onto texture.
 *
 * \param src : CEV_BmpFont* as source to be displayed.
 * \param dst : SDL_Texture* as destination to blit onto.
 * \param text : const char* as text to be displayed.
 * \param pos : SDL_Rect with x,y as topmost leftmost position.
 *
 * \return int of std return values.
 *
 * \note Texture should has SDL_TEXTUREACCESS_STREAMING property
 * \note Text length is not limited though not guaranted to fit into texture.
 */
int CEV_bmpFontDispText(CEV_BmpFont* src, SDL_Texture* dst, const char* text, SDL_Rect pos);


#endif // CEV_BMPFONT_H_INCLUDED
