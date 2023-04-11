//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |    02-2017    |   1.0    |    creation    **/
//**   CEV    |    05-2021    |   1.0    | Documentation  **/
//**   CEV    |    02-2022    |   1.0.3  | sort added     **/
//**********************************************************/
/*
- CEV 2022_07_24 : R/W fonctions revised or added / struct to capsule added
*/

#ifndef CEV_TEXT_H_INCLUDED
#define CEV_TEXT_H_INCLUDED

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "CEV_types.h"




#ifdef __cplusplus
extern "C" {
#endif


/** \brief Easy text management.
 *  note : For each line, the alloc size is stored in size_t just after the nul char
 */
typedef struct CEV_Text
{
    size_t  linesNum,       /**< number of lines*/
            lineSize;       /**< longest line length terminal char included*/

    char    fileTerminator, /**< file terminator char*/
            **line;         /**< texts storage*/
}
CEV_Text;


/** \brief allocates a new text structure.
 *
 * \param number of lines to create.
 * \param longest line length.
 *
 * \return CEV_Text ptr on success, NULL on error.
 *
 * note : stderr is filed with error if any.
 */
CEV_Text* CEV_textCreate(unsigned int lines, unsigned int maxLength);



/** \brief writes text line.
 *
 * \param dst : text structure to write into.
 * \param index : "string" index to write into.
 * \param src : "string" to be written.
 *
 * \return return any of standard status.
 * note : internal strings need maintenance, do not use std func on CEV_Text strings
 */
int CEV_textWrite(CEV_Text* dst, unsigned int index, const char* src);


/** \brief fetches text line.
 *
 * \param src : text structure to read from.
 * \param index : "string" index to read from.
 *
 * \return  ptr* to start of string address.
 * note : internal strings need maintenance, do not use std func on CEV_Text strings
 */
char* CEV_textRead(CEV_Text *src, unsigned int index);


/** \brief Sorts content A to Z
 *
 * \param src : CEV_Text* to sort
 *
 * \return void
 */
void CEV_textSortAZ(CEV_Text *src);


/** \brief Sorts content Z to A
 *
 * \param src : CEV_Text* to sort
 *
 * \return void
 */
void CEV_textSortZA(CEV_Text *src);

 /** \brief appends text line.
 *
 * \param dst : text structure to add src into.
 * \param src : source "string".
 *
 * \return  return any of standard status.
 */
 int CEV_textAppend(CEV_Text* dst, const char* src);


/**files related*/
/** \brief Converts user parameter file into pgrm friendly file.
 *
 * \param srcName : file to read from.
 * \param dstName : file to write into.
 *
 * \return any of function status.
 *
 * note : dstName should have .tdat extension type.
 * any line starting with '/' char are ignored.
 */
int CEV_textConvertTxtToData(const char *srcName, const char *dstName);


/** \brief Loads CEV_Text from file.
 *
 * \param fileName : const char* as file name to open.
 *
 * \return CEV_Text* on success, NULL on failure.
 */
CEV_Text* CEV_textLoad(const char *fileName);


/** \brief Loads CEV_Text from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param freeSrc : SDL_RWops is closed if true.
 *
 * \return : CEV_Text* on success, NULL on failure.
 * note : : if freeScrc is true, src is freed weither the fonction succeeds or not.
 */
CEV_Text* CEV_textLoad_RW(SDL_RWops* src, bool freeSrc);


/** \brief Loads user parameter file.
 *
 * \param fileName : file to load.
 * \return CEV_Text* on succes, NULL on error.
 *
 * note : opens "natural" txt file.
 * any line starting with '/' char are ignored as beeing comment.
 * \sa CEV_textTxtLoadf()
 */
CEV_Text* CEV_textTxtLoad(const char *fileName);


/** \brief Loads opened user parameter file.
 *
 * \param src : opend file to read from.
 * \return CEV_Text* on succes, NULL on error.
 *
 * note : reads "natural" txt file.
 * \sa CEV_textTxtLoad()
 */
CEV_Text* CEV_textTxtLoadf(FILE* src);


/** \brief Saves CEV_Text into file.
 *
 * \param text : CEV_Text* to save.
 * \param fileName : const char* as file name.
 *
 * \return int as std function status.
 */
int CEV_textSave(CEV_Text *text, const char* fileName);


/** \brief Writes CEV_Text into file at data format.
 *
 * \param src : CEV_Text* to read from.
 * \param dst : FILE* to write into.
 * \return readWriteError is incremented on error.
 *
 * \sa CEV_textTypeRead()
 */
void CEV_textTypeWrite(CEV_Text* src, FILE* dst);


/** \brief Writes CEV_Text into virtual file.
 *
 * \param text : CEV_Text* to read from.
 * \param dst  : SDL_RWops* to write into
 * \return int
 *
 */
int CEV_textTypeWrite_RW(CEV_Text* src, SDL_RWops* dst);


/** \brief Reads CEV_Text from opened data format file.
 *
 * \param src  : FILE* to read from.
 * \param dst : CEV_Text* to fill.
 *
 * \return readWriteError is incremented on error.
 *
 * \sa CEV_textTypeWrite().
 */
void CEV_textTypeRead(FILE *src, CEV_Text* dst);


/** \brief Reads CEV_Text from virtual file.
 *
 * \param src : SDL_RWops* to read from.
 * \param dst : CEV_Text* to fill.
 *
 * \return void
 * note : readWriteErr is used.
 */
void CEV_textTypeRead_RW(SDL_RWops* src, CEV_Text* dst);

/** \brief Fills capsule with CEV_Text.
 *
 * \param src : CEV_Text* to read from.
 * \param dst : CEV_Capsule* to store result.
 *
 * \return int : of std function status.
 *
 */
int CEV_textToCapsule(CEV_Text* src, CEV_Capsule* dst);


/** \brief Creates a CEV_Text from SDL_RWops (data format).
 *
 * \param src  : SDL_RWops* to read from.
 * \param freeData  : internally closes ops if not 0.
 *
 * \return CEV_Text* on succes, NULL on error.
 *
 */
CEV_Text* CEV_textTxtLoad_RW(SDL_RWops* src, char freeData);


/** \brief Creates a data format file from CEV_Text.
 *
 * \param src : CEV_Text* to create file from.
 * \param dstFileName : name of file which will be created.
 *
 * \return any of function status.
 */
int CEV_textSave(CEV_Text *src, const char* dstFileName);


/** \brief Free allocated CEV_Text.
 *
 * \param in : CEV_Text* to be freed.
 *
 * \return N/A.
 */
void CEV_textDestroy(CEV_Text * in);


/** \brief Free and raz content of CEV_Text.
 *
 * \param in : CEV_Text* to clear.
 *
 * \return N/A.
 */
void CEV_textClear(CEV_Text* in);


/** \brief Dumps structure content into stdout.
 *
 * \param in : CEV_Text* to dump.
 *
 * \return N/A.
 */
void CEV_textDump(CEV_Text* in);


/** \brief Seeks string in file.
 *
 * \param file : FILE* to seek into.
 * \param src : char* to find.
 *
 * \return line index where src was found, -1 if not found.
 */
int CEV_fileStrSearch(FILE* file, char* src);



/** \brief cleans file string into memory friendly string :
 * note : removes '/\r' and '/\n' replaced by '/\0'.
 * \param in : char* to clean.
 *
 * \return N/A.
 */
void CEV_stringEndFormat(char* in);


/** \brief digit grouping.
 *
 * \param src : char* to format.
 * \param group : number of digit to group together.
 *
 * \return N/A.
 */
void CEV_stringGroup(char *src, unsigned int group);


/** \brief stress / test
 * \param void
 * \return void
 */
void TEST_text(void);

#ifdef __cplusplus
}
#endif


#endif // CEV_TEXT_H_INCLUDED
