#ifndef CEV_FILE_H_INCLUDED
#define CEV_FILE_H_INCLUDED

#include <stdio.h>


/** \brief extract number of lines in file.
 *
 * \param file : ptr on FILE.
 *
 * \return num of lines 0 otherwise.
 */
int CEV_fileNumOfLine(FILE *file);


/** \brief sets file's cursor at begin of line line.
 *
 * \param line : which line to reach.
 *
 * \return true on success, false otherwise.
 */
bool CEV_fileGotoLine(int line, FILE* file);


/** \brief sets file's cursor after nums separator.
 *
 * \param num : num of separator to pass.
 * \param seprator : separator identifier.
 *
 * \return true on success, false otherwise.
 */
bool CEV_fileGotoColumn(int num, FILE *file, int separator);


/** \brief sets file's cirsor at begin of next available line.
 *
 * \param file : ptr on FILE.
 *
 * \return true on success, false otherwise.
 */
bool CEV_fileGotoNextLine(FILE *file);


/** \brief size of a file.
 *
 * \param file : ptr on FILE.
 *
 * \return size in bytes.
 */
size_t CEV_fileSize(FILE* file);


/** \brief extract file name from full path name
 *
 * \param src : full name path+file name
 * \param dst : stores result as file name only
 *
 * \return 1 on success, 0 otherwise.
 */
bool CEV_fileFileNameGet(const char* src, char* dst);


/** \brief extract folder from full path name
 *
 * \param src : full name path+file name
 * \param dst : stores result as path only
 *
 * \return 1 on success, 0 otherwise.
 */
bool CEV_fileFolderNameGet(const char *src, char *dst);


/** \brief cleans end of string from LR/CR.
 *
 * \param in : string to clean.
 *
 * \return N/A.
 */
void CEV_stringEndFormat(char* in);


/** \brief groups chars in string, filled with space.
 *
 * \param src : string to modify.
 * \param group : num of char to group together.
 *
 * \return N/A.
 *
 * \note src must be long enough to be extended.
 */
void CEV_stringGroup(char *src, unsigned int group);

#endif // CEV_FILE_H_INCLUDED
