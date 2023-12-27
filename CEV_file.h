//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  14-02-2015   |   1.0    |    creation    **/
//**********************************************************/

#ifndef CEV_FILE_H_INCLUDED
#define CEV_FILE_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>


/** \brief extract number of lines in file.
 *
 * \param file : ptr on FILE.
 *
 * \return num of lines 0 otherwise.
 *
 * \note File ought to be in text mode / csv or compliant.
 */
int CEV_fileNumOfLine(FILE *file);


/** \brief sets file's cursor at begin of line line.
 *
 * \param line : which line to reach.
 *
 * \return true on success, false otherwise.
 *
 * \note File ought to be in text mode / csv or compliant.
 */
bool CEV_fileGotoLine(int line, FILE* file);


/** \brief sets file's cursor after nums separator.
 *
 * \param num : num of separator to pass.
 * \param seprator : separator identifier.
 *
 * \return true on success, false otherwise.
 *
 * \note File ought to be in text mode / csv or compliant.
 */
bool CEV_fileGotoColumn(int num, FILE *file, int separator);


/** \brief sets file's cirsor at begin of next available line.
 *
 * \param file : ptr on FILE.
 *
 * \return true on success, false otherwise.
 *
 * \note File ought to be in text mode / csv or compliant.
 */
bool CEV_fileGotoNextLine(FILE *file);


/** \brief size of a file.
 *
 * \param file : ptr on FILE.
 *
 * \return size in bytes.
 */
size_t CEV_fileSize(FILE* file);


/** \brief extract file name from full path name.
 *
 * \param src : full name path+file name.
 * \param dst : stores result as file name only.
 *
 * \return 1 on success, 0 otherwise.
 */
bool CEV_fileFileNameGet(const char* src, char* dst);


/** \brief extract folder from full path and name.
 *
 * \param src : full name path+file name.
 * \param dst : stores result as path only.
 *
 * \return 1 on success, 0 otherwise.
 *
 * \note folder name provided with strin end '/' or '\\'
 */
bool CEV_fileFolderNameGet(const char *src, char *dst);



/** \brief Modify str to parent folder.
 * note : subfolder char '/' or '\' is removed.
 * \param str : char* to path.
 *
 * \return true if str was modified.
 */
bool CEV_fileFolderUp(char* str);


/** \brief cleans end of string from LR/CR with nul.
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
 * \note Src must be long enough to be extended.
 */
void CEV_stringGroup(char *src, unsigned int group);


/** \brief seeks string in file (complete line).
 *
 * \param file : file to seek string into.
 * \param src : string to seek.
 *
 * \return line index of where string was found, -1 otherwise.
 *
 * \note Src must be long enough to be extended.
 */
int CEV_fileStrSearch(FILE* file, char* src);


/** \brief copies file.
 *
 * \param srcName : char* path and name of file to copy.
 * \param dstName char* path and name of file as result.
 *
 * \return int : any of std return value.
 *
 */
int CEV_fileCopy(char *srcName, char *dstName);


/** \brief insert file into another one
 *
 * \param srcName : char* as path and name of file to read.
 * \param dst : FILE* as file to write into at current cursor.
 *
 * \return int : any of std return value.
 */
int CEV_fileInsert(char* srcName, FILE* dst);

#endif // CEV_FILE_H_INCLUDED
