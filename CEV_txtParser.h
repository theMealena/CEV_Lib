//**********************************************************/
//** Done by  |      Date     |  version |    comment     **/
//**------------------------------------------------------**/
//**   CEV    |  03-03-2023   |   1.0    |    creation    **/
//**********************************************************/



#ifndef CEV_TXTPARSER_H_INCLUDED
#define CEV_TXTPARSER_H_INCLUDED

#include <CEV_texts.h>


/** \brief Fetches parameter numeric value.
 *
 * \param src : CEV_Text* to extract value from.
 * \param name : char* as parameter's name.
 *
 * \return double as value extracted, 0.0 is default value on error.
 *
 * \note Value must be placed after '=' char.
 */
double CEV_txtParseValueFrom(CEV_Text* src, char* name);


/** \brief Fetches parameter string value.
 *
 * \param src : CEV_Text* to extratc string from.
 * \param name : char* as parameter's name.
 *
 * \return char* to start of string, NULL on error / inexistant.
 */
char* CEV_txtParseTxtFrom(CEV_Text* src, char* name);


/** \brief Fetches multiple values from string.
 *
 * \param src : CEV_Text* from which values must be extracted.
 * \param name : char* as parameter's name.
 * \param dst : double* array that will be filled with read values.
 * \param maxNum : int as maximum values to read (dst[] size).
 *
 * \return int as num of values that could be read.
 *
 * \note Values shall be separated by the ';' char.
 */
int CEV_txtParseValueArrayFrom(CEV_Text *src, char* name, double* dst, int maxNum);


/** \brief Provide name's index in src.
 *
 * \param src : CEV_Text* to seek into.
 * \param name : char* as the parameter to find.
 *
 * \return int : index of the paramater name, 0 if not found.
 */
int CEV_txtParseIndexGetFrom(CEV_Text *src, char* name);


/** \brief stress test / tries.
 *
 * \param void
 *
 * \return void
 */
void TEST_txtParse(void);

#endif // CEV_TXTPARSER_H_INCLUDED
